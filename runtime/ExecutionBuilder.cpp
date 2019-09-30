/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "ExecutionBuilder"

#include "ExecutionBuilder.h"

#include <algorithm>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "CompilationBuilder.h"
#include "CpuExecutor.h"
#include "ExecutionBurstController.h"
#include "HalInterfaces.h"
#include "Manager.h"
#include "ModelArgumentInfo.h"
#include "ModelBuilder.h"
#include "Tracing.h"
#include "TypeManager.h"
#include "Utils.h"

namespace android {
namespace nn {

using namespace hal;

const Timing kNoTiming = {.timeOnDevice = UINT64_MAX, .timeInDriver = UINT64_MAX};

static MeasureTiming measureTiming(const ExecutionBuilder* execution) {
    return execution->measureTiming() ? MeasureTiming::YES : MeasureTiming::NO;
}

static bool checkDimensionInfo(const Operand& operand, const ANeuralNetworksOperandType* newType,
                               const char* tag, bool allowUnspecified) {
    if (newType != nullptr) {
        const Extension::OperandTypeInformation* info = nullptr;
        if (isExtensionOperandType(operand.type)) {
            NN_RET_CHECK(TypeManager::get()->getExtensionOperandTypeInfo(operand.type, &info));
        }
        if (validateOperandType(*newType, info, tag, allowUnspecified) !=
            ANEURALNETWORKS_NO_ERROR) {
            LOG(ERROR) << tag << ": Invalid newType";
            return false;
        }
        if (operand.dimensions.size() == 0) {
            return true;
        }
        if (operand.dimensions.size() != newType->dimensionCount) {
            LOG(ERROR) << tag << ": Setting with incompatible dimension count";
            return false;
        }
        for (uint32_t i = 0; i < newType->dimensionCount; i++) {
            if (operand.dimensions[i] != newType->dimensions[i] && operand.dimensions[i] != 0) {
                LOG(ERROR) << tag << ": Overriding a fully specified dimension is disallowed";
                return false;
            }
        }
    } else {
        if (!allowUnspecified && TypeManager::get()->isTensorType(operand.type) &&
            tensorHasUnspecifiedDimensions(operand)) {
            LOG(ERROR) << tag << ": Setting with operand type that is not fully specified";
            return false;
        }
    }
    return true;
}

ExecutionBuilder::ExecutionBuilder(const CompilationBuilder* compilation)
    : mCompilation(compilation),
      mModel(compilation->mModel),
      mPlan(&compilation->mPlan),
      mPartitioning(compilation->mPartitioning),
      mInputs(mModel->inputCount()),
      mOutputs(mModel->outputCount()) {
    VLOG(EXECUTION) << "ExecutionBuilder::ExecutionBuilder";
}

int ExecutionBuilder::setInput(uint32_t index, const ANeuralNetworksOperandType* type,
                               const void* buffer, size_t length) {
    if (mStarted) {
        LOG(ERROR) << "ANeuralNetworksExecution_setInput called after the "
                      "execution has started.";
        return ANEURALNETWORKS_BAD_STATE;
    }
    uint32_t count = static_cast<uint32_t>(mInputs.size());
    if (index >= count) {
        LOG(ERROR) << "ANeuralNetworksExecution_setInput bad index " << index << " " << count;
        return ANEURALNETWORKS_BAD_DATA;
    }
    if (!checkDimensionInfo(mModel->getInputOperand(index), type,
                            "ANeuralNetworksExecution_setInput", buffer == nullptr)) {
        return ANEURALNETWORKS_BAD_DATA;
    }
    if (length > 0xFFFFFFFF) {
        LOG(ERROR) << "ANeuralNetworksExecution_setInput input exceeds max length " << length;
        return ANEURALNETWORKS_BAD_DATA;
    }
    uint32_t l = static_cast<uint32_t>(length);
    return mInputs[index].setFromPointer(mModel->getInputOperand(index), type,
                                         const_cast<void*>(buffer), l);
}

int ExecutionBuilder::setInputFromMemory(uint32_t index, const ANeuralNetworksOperandType* type,
                                         const Memory* memory, size_t offset, size_t length) {
    // Should be similar to StepExecutor::setInputOrOutputFromTemporaryMemory()

    if (mStarted) {
        LOG(ERROR) << "ANeuralNetworksExecution_setInputFromMemory called after the "
                      "execution has started.";
        return ANEURALNETWORKS_BAD_STATE;
    }
    uint32_t count = static_cast<uint32_t>(mInputs.size());
    if (index >= count) {
        LOG(ERROR) << "ANeuralNetworksExecution_setInputFromMemory bad index " << index << " "
                   << count;
        return ANEURALNETWORKS_BAD_DATA;
    }
    if (!checkDimensionInfo(mModel->getInputOperand(index), type,
                            "ANeuralNetworksExecution_setInputFromMemory", false)) {
        return ANEURALNETWORKS_BAD_DATA;
    }
    // Both offset & length must be zero for Non-BLOB format AHardwareBuffer.
    if (memory->getHidlMemory().name() == "hardware_buffer" && (offset != 0 || length != 0)) {
        LOG(ERROR) << "ANeuralNetworksExecution_setInputFromMemory has non-zero offset and length"
                   << " for Non-BLOB format AHardwareBuffer.";
        return ANEURALNETWORKS_BAD_DATA;
    } else if (!memory->validateSize(offset, length)) {
        return ANEURALNETWORKS_BAD_DATA;
    }
    // TODO validate the rest
    uint32_t poolIndex = mMemories.add(memory);
    return mInputs[index].setFromMemory(mModel->getInputOperand(index), type, poolIndex, offset,
                                        length);
}

int ExecutionBuilder::setOutput(uint32_t index, const ANeuralNetworksOperandType* type,
                                void* buffer, size_t length) {
    if (mStarted) {
        LOG(ERROR) << "ANeuralNetworksExecution_setOutput called after the "
                      "execution has started.";
        return ANEURALNETWORKS_BAD_STATE;
    }
    uint32_t count = static_cast<uint32_t>(mOutputs.size());
    if (index >= count) {
        LOG(ERROR) << "ANeuralNetworksExecution_setOutput bad index " << index << " " << count;
        return ANEURALNETWORKS_BAD_DATA;
    }
    if (!checkDimensionInfo(mModel->getOutputOperand(index), type,
                            "ANeuralNetworksExecution_setOutput", true)) {
        return ANEURALNETWORKS_BAD_DATA;
    }
    if (length > 0xFFFFFFFF) {
        LOG(ERROR) << "ANeuralNetworksExecution_setOutput input exceeds max length " << length;
        return ANEURALNETWORKS_BAD_DATA;
    }
    uint32_t l = static_cast<uint32_t>(length);
    return mOutputs[index].setFromPointer(mModel->getOutputOperand(index), type, buffer, l);
}

int ExecutionBuilder::setOutputFromMemory(uint32_t index, const ANeuralNetworksOperandType* type,
                                          const Memory* memory, size_t offset, size_t length) {
    // Should be similar to StepExecutor::setInputOrOutputFromTemporaryMemory()

    if (mStarted) {
        LOG(ERROR) << "ANeuralNetworksExecution_setOutputFromMemory called after the "
                      "execution has started.";
        return ANEURALNETWORKS_BAD_STATE;
    }
    uint32_t count = static_cast<uint32_t>(mOutputs.size());
    if (index >= count) {
        LOG(ERROR) << "ANeuralNetworksExecution_setOutputFromMemory bad index " << index << " "
                   << count;
        return ANEURALNETWORKS_BAD_DATA;
    }
    if (!checkDimensionInfo(mModel->getOutputOperand(index), type,
                            "ANeuralNetworksExecution_setOutputFromMemory", true)) {
        return ANEURALNETWORKS_BAD_DATA;
    }
    // Both offset & length must be zero for Non-BLOB format AHardwareBuffer.
    if (memory->getHidlMemory().name() == "hardware_buffer" && (offset != 0 || length != 0)) {
        LOG(ERROR) << "ANeuralNetworksExecution_setOutputFromMemory has non-zero offset and length"
                   << " for Non-BLOB format AHardwareBuffer.";
        return ANEURALNETWORKS_BAD_DATA;
    } else if (!memory->validateSize(offset, length)) {
        return ANEURALNETWORKS_BAD_DATA;
    }
    // TODO validate the rest
    uint32_t poolIndex = mMemories.add(memory);
    return mOutputs[index].setFromMemory(mModel->getOutputOperand(index), type, poolIndex, offset,
                                         length);
}

int ExecutionBuilder::setMeasureTiming(bool measure) {
    if (!mCompilation->mExplicitDeviceList || (mCompilation->mDevices.size() != 1)) {
        LOG(ERROR) << "ANeuralNetworksExecution_setMeasureTiming called on "
                   << "an ANeuralNetworksExecution created from an ANeuralNetworksCompilation "
                   << "that was not created by ANeuralNetworksCompilation_createForDevices "
                   << "with numDevices = 1";
        return ANEURALNETWORKS_BAD_DATA;
    }
    if (mStarted) {
        LOG(ERROR) << "ANeuralNetworksExecution_setMeasureTiming called after the "
                      "execution has started.";
        return ANEURALNETWORKS_BAD_STATE;
    }
    mMeasureTiming = measure;
    return ANEURALNETWORKS_NO_ERROR;
}

int ExecutionBuilder::getDuration(int32_t durationCode, uint64_t* duration) const {
    if (!mFinished) {
        LOG(ERROR) << "ANeuralNetworksExecution_getDuration called before the "
                      "execution has finished.";
        return ANEURALNETWORKS_BAD_STATE;
    }

    // NOTE: At the HAL level, timing is in microseconds. At the NDK level, nanoseconds.
    const uint64_t kNanoPerMicro = 1000;

    if (!mMeasureTiming) {
        *duration = UINT64_MAX;
        return ANEURALNETWORKS_BAD_STATE;
    }

    uint64_t microDuration = UINT64_MAX;
    switch (durationCode) {
        case ANEURALNETWORKS_DURATION_ON_HARDWARE:
            microDuration = mTiming.timeOnDevice;
            break;
        case ANEURALNETWORKS_DURATION_IN_DRIVER:
            microDuration = mTiming.timeInDriver;
            break;
        default:
            CHECK(!"unexpected");
    }
    *duration = (microDuration == UINT64_MAX) ? UINT64_MAX : kNanoPerMicro * microDuration;

    VLOG(EXECUTION) << "getDuration(" << durationCode << "): " << *duration;
    return ANEURALNETWORKS_NO_ERROR;
}

int ExecutionBuilder::getOutputOperandDimensions(uint32_t index, uint32_t* dimensions) {
    if (!mFinished) {
        LOG(ERROR) << "ANeuralNetworksExecution_getOutputOperandDimensions called before the "
                      "execution has finished.";
        return ANEURALNETWORKS_BAD_STATE;
    }
    uint32_t count = static_cast<uint32_t>(mOutputs.size());
    if (index >= count) {
        LOG(ERROR) << "ANeuralNetworksExecution_getOutputOperandDimensions bad index " << index
                   << " " << count;
        return ANEURALNETWORKS_BAD_DATA;
    }
    const auto& dims = mOutputs[index].dimensions;
    if (dims.empty()) {
        LOG(ERROR) << "ANeuralNetworksExecution_getOutputOperandDimensions can not query "
                      "dimensions of a scalar";
        return ANEURALNETWORKS_BAD_DATA;
    }
    std::copy(dims.begin(), dims.end(), dimensions);
    return mOutputs[index].isSufficient ? ANEURALNETWORKS_NO_ERROR
                                        : ANEURALNETWORKS_OUTPUT_INSUFFICIENT_SIZE;
}

int ExecutionBuilder::getOutputOperandRank(uint32_t index, uint32_t* rank) {
    if (!mFinished) {
        LOG(ERROR) << "ANeuralNetworksExecution_getOutputOperandRank called before the "
                      "execution has finished.";
        return ANEURALNETWORKS_BAD_STATE;
    }
    uint32_t count = static_cast<uint32_t>(mOutputs.size());
    if (index >= count) {
        LOG(ERROR) << "ANeuralNetworksExecution_getOutputOperandRank bad index " << index << " "
                   << count;
        return ANEURALNETWORKS_BAD_DATA;
    }
    *rank = static_cast<uint32_t>(mOutputs[index].dimensions.size());
    return mOutputs[index].isSufficient ? ANEURALNETWORKS_NO_ERROR
                                        : ANEURALNETWORKS_OUTPUT_INSUFFICIENT_SIZE;
}

// Attempt synchronous execution of full model on CPU.
// Ensure that executionCallback->notify() is called.
// TODO: How should we handle timing in this case?
//       For Q this is irrelevant: We only support timing in conjunction
//         with an explicit device list; and we do not support CPU fallback
//         with an explicit device list.  See CompilationBuilder::mExplicitDeviceList.
static void cpuFallbackFull(ExecutionBuilder* executionBuilder,
                            const sp<ExecutionCallback>& executionCallback) {
    CHECK(executionBuilder != nullptr);
    CHECK(executionCallback != nullptr);
    NNTRACE_RT(NNTRACE_PHASE_EXECUTION, "cpuFallbackFull");
    VLOG(EXECUTION) << "cpuFallbackFull";
    StepExecutor executor(executionBuilder, executionBuilder->getModel(),
                          DeviceManager::getCpuDevice(), /*preparedModel=*/nullptr);
    executor.mapInputsAndOutputsTrivially();
    sp<ExecutionCallback> fallbackCallback;
    int n = executor.startComputeOnCpuFallback(&fallbackCallback);
    if (n != ANEURALNETWORKS_NO_ERROR) {
        executionCallback->notify(convertResultCodeToErrorStatus(n), {}, kNoTiming);
        return;
    }
    fallbackCallback->wait();
    executionCallback->notify(fallbackCallback->getStatus(), fallbackCallback->getOutputShapes(),
                              fallbackCallback->getTiming());
}

// Attempt synchronous execution on CPU.
// (1) First, attempt to execute this step on CPU.  If successful,
//     return true.  (Do not call executionCallback->notify().)
// (2) If unsuccessful, and the ExecutionPlan is compound, attempt to execute the
//     full model on CPU, ensure that executionCallback->notify() is called, and return
//     false.
// TODO: How should we handle timing in this case?
//       For Q this is irrelevant: We only support timing in conjunction
//         with an explicit device list; and we do not support CPU fallback
//         with an explicit device list.  See CompilationBuilder::mExplicitDeviceList.
static bool cpuFallbackPartial(ExecutionBuilder* executionBuilder, const ExecutionPlan* plan,
                               std::shared_ptr<ExecutionPlan::Controller> controller,
                               const sp<ExecutionCallback>& executionCallback,
                               std::vector<OutputShape>* outputShapes) {
    CHECK(executionBuilder != nullptr);
    CHECK(plan != nullptr);
    CHECK(executionCallback != nullptr);
    CHECK(outputShapes != nullptr);
    NNTRACE_RT(NNTRACE_PHASE_EXECUTION, "cpuFallbackPartial");
    VLOG(EXECUTION) << "cpuFallbackPartial";
    std::shared_ptr<StepExecutor> executor;
    int n = plan->fallback(controller, &executor);
    if (n != ANEURALNETWORKS_NO_ERROR || executor->isCpu()) {
        cpuFallbackFull(executionBuilder, executionCallback);
        return false;
    }
    sp<ExecutionCallback> fallbackCallback;
    if (executor->startComputeOnCpuFallback(&fallbackCallback) != ANEURALNETWORKS_NO_ERROR) {
        cpuFallbackFull(executionBuilder, executionCallback);
        return false;
    }
    fallbackCallback->wait();
    ErrorStatus status = fallbackCallback->getStatus();
    const auto& stepOutputShapes = fallbackCallback->getOutputShapes();
    if (!executor->updateOutputShapes(stepOutputShapes, outputShapes)) {
        status = ErrorStatus::GENERAL_FAILURE;
    }
    if (status != ErrorStatus::NONE) {
        // Do not fallback twice if the ExecutionPlan is simple.
        // OUTPUT_INSUFFICIENT_SIZE is not recoverable
        if (plan->isSimple() || status == ErrorStatus::OUTPUT_INSUFFICIENT_SIZE) {
            executionCallback->notify(status, *outputShapes, kNoTiming);
        } else {
            cpuFallbackFull(executionBuilder, executionCallback);
        }
        return false;
    }
    return true;
}

static void asyncStartComputePartitioned(ExecutionBuilder* executionBuilder,
                                         const ExecutionPlan* plan,
                                         std::shared_ptr<ExecutionPlan::Controller> controller,
                                         bool allowFallback,
                                         const sp<ExecutionCallback>& executionCallback) {
    CHECK(executionBuilder != nullptr);
    CHECK(plan != nullptr);
    VLOG(EXECUTION) << "ExecutionBuilder::compute (from plan, iteratively)";
    std::vector<OutputShape> outputShapes;
    Timing timing = kNoTiming;
    // Disallow fallback when the ExecutionPlan is simple on CPU.
    allowFallback &= !plan->isSimpleCpu();
    executionBuilder->initializeOutputShapes(&outputShapes);
    while (true) {
        std::shared_ptr<StepExecutor> executor;
        VLOG(EXECUTION) << "looking for next StepExecutor";
        std::shared_ptr<ExecutionBurstController> burstController = nullptr;
        int n = plan->next(controller, &executor, &burstController);
        if (n != ANEURALNETWORKS_NO_ERROR) {
            if (allowFallback) {
                cpuFallbackFull(executionBuilder, executionCallback);
            } else {
                executionCallback->notify(convertResultCodeToErrorStatus(n), {}, kNoTiming);
            }
            return;
        }
        if (executor == nullptr) {
            executionCallback->notify(ErrorStatus::NONE, outputShapes, timing);
            return;
        }

        sp<ExecutionCallback> stepCallback;
        n = executor->startCompute(&stepCallback, burstController);
        if (n != ANEURALNETWORKS_NO_ERROR) {
            if (allowFallback) {
                if (cpuFallbackPartial(executionBuilder, plan, controller, executionCallback,
                                       &outputShapes)) {
                    // Successfully executed one step on CPU.
                    continue;
                } else {
                    // Either successfully executed entire plan on
                    // CPU, or tried and failed to do so.
                    return;
                }
            } else {
                executionCallback->notify(convertResultCodeToErrorStatus(n), {}, kNoTiming);
                return;
            }
        }
        stepCallback->wait();
        ErrorStatus status = stepCallback->getStatus();
        const auto& stepOutputShapes = stepCallback->getOutputShapes();
        if (!executor->updateOutputShapes(stepOutputShapes, &outputShapes)) {
            status = ErrorStatus::GENERAL_FAILURE;
        }
        if (status == ErrorStatus::NONE) {
            // We only support collection of timing information in the case of a
            // single step, so it's safe to just keep track of the last step's
            // timing information.
            timing = stepCallback->getTiming();
        } else {
            // OUTPUT_INSUFFICIENT_SIZE is not recoverable
            if (allowFallback && status != ErrorStatus::OUTPUT_INSUFFICIENT_SIZE) {
                if (cpuFallbackPartial(executionBuilder, plan, controller, executionCallback,
                                       &outputShapes)) {
                    // Successfully executed one step on CPU.
                    continue;
                } else {
                    // Either successfully executed entire plan on
                    // CPU, or tried and failed to do so.
                    return;
                }
            } else if (status == ErrorStatus::OUTPUT_INSUFFICIENT_SIZE) {
                executionCallback->notify(status, outputShapes, kNoTiming);
                return;
            } else {
                executionCallback->notify(status, {}, kNoTiming);
                return;
            }
        }
    }
}

int ExecutionBuilder::compute(sp<ExecutionCallback>* synchronizationCallback,
                              BurstBuilder* burstBuilder) {
    CHECK(synchronizationCallback == nullptr || burstBuilder == nullptr)
            << "synchronizationCallback and burstBuilder cannot simultaneously be used";

    const bool synchronous = (synchronizationCallback == nullptr);

    if (!synchronous) {
        *synchronizationCallback = nullptr;
    }

    // TODO validate that we have full types for all inputs and outputs,
    // that the graph is not cyclic,

    auto name = [synchronous, burstBuilder] {
        return burstBuilder ? "burstCompute" : synchronous ? "compute" : "startCompute";
    };
    if (mStarted) {
        LOG(ERROR) << "ANeuralNetworksExecution_" << name()
                   << " called on an execution that has already started";
        return ANEURALNETWORKS_BAD_STATE;
    }
    for (auto& p : mInputs) {
        if (p.state == ModelArgumentInfo::UNSPECIFIED) {
            LOG(ERROR) << "ANeuralNetworksExecution_" << name() << " not all inputs specified";
            return ANEURALNETWORKS_BAD_DATA;
        }
    }
    for (auto& p : mOutputs) {
        if (p.state == ModelArgumentInfo::UNSPECIFIED) {
            LOG(ERROR) << "ANeuralNetworksExecution_" << name() << " not all outputs specified";
            return ANEURALNETWORKS_BAD_DATA;
        }
    }

    auto wrappedFinish = [this](ErrorStatus error, const std::vector<OutputShape>& outputShapes) {
        return finish(error, outputShapes);
    };

    // TODO: For asynchronous execution, entire plan-based-path should run in an
    // asynchronous thread -- take the asynchronous thread logic out of
    // CpuPreparedModel::execute() and use it to wrap the plan-based-path.
    mStarted = true;
    const bool allowFallback = DeviceManager::partitioningAllowsFallback(mPartitioning);
    std::shared_ptr<ExecutionPlan::Controller> controller =
            mPlan->makeController(this, burstBuilder);
    if (synchronous) {
        VLOG(EXECUTION) << "ExecutionBuilder::compute (synchronous API)";
        sp<ExecutionCallback> localSynchronizationCallback = new ExecutionCallback();
        localSynchronizationCallback->setOnFinish(wrappedFinish);
        asyncStartComputePartitioned(this, mPlan, controller, allowFallback,
                                     localSynchronizationCallback);
        localSynchronizationCallback->wait();
        if (mMeasureTiming) {
            mTiming = localSynchronizationCallback->getTiming();
        }
        return convertErrorStatusToResultCode(localSynchronizationCallback->getStatus());
    } else /* asynchronous */ {
        // TODO: use a thread pool
        // TODO(mikie): this could have NNTRACE so we could measure the overhead
        //              of spinning up a new thread.

        // Prepare the callback for asynchronous execution.
        // sp<ExecutionCallback> object is returned when the
        // execution has been successfully launched, otherwise a
        // nullptr is returned.  The executionCallback is
        // abstracted in the NN API as an "event".
        sp<ExecutionCallback> executionCallback = new ExecutionCallback();
        executionCallback->setOnFinish(wrappedFinish);
        if (DeviceManager::get()->syncExecRuntime()) {
            VLOG(EXECUTION) << "ExecutionBuilder::compute (asynchronous API, non-threaded)";
            asyncStartComputePartitioned(this, mPlan, controller, allowFallback, executionCallback);
        } else {
            VLOG(EXECUTION) << "ExecutionBuilder::compute (asynchronous API)";
            std::thread thread(asyncStartComputePartitioned, this, mPlan, controller, allowFallback,
                               executionCallback);
            executionCallback->bindThread(std::move(thread));
        }
        *synchronizationCallback = executionCallback;
        return ANEURALNETWORKS_NO_ERROR;
    }
}

void ExecutionBuilder::initializeOutputShapes(std::vector<OutputShape>* outputShapes) const {
    outputShapes->resize(mOutputs.size());
    for (uint32_t i = 0; i < mOutputs.size(); i++) {
        (*outputShapes)[i].dimensions = mOutputs[i].dimensions;
        (*outputShapes)[i].isSufficient = true;
    }
}

// Check if the dimensions "to" is updatable by dimensions "from", where "from" must
// have a higher specification level.
static bool isUpdatable(const std::vector<uint32_t>& to, const std::vector<uint32_t>& from) {
    if (to.size() == 0) return true;
    NN_RET_CHECK_EQ(to.size(), from.size());
    for (uint32_t i = 0; i < to.size(); i++) {
        NN_RET_CHECK(to[i] == from[i] || to[i] == 0);
    }
    return true;
}

bool ExecutionBuilder::updateOutputShapes(const std::vector<OutputShape>& outputShapes) {
    if (outputShapes.size() == 0) {
        return true;
    }
    NN_RET_CHECK_EQ(outputShapes.size(), mOutputs.size());
    for (uint32_t i = 0; i < outputShapes.size(); i++) {
        // Check if only unspecified dimensions or rank are overwritten.
        NN_RET_CHECK(isUpdatable(mOutputs[i].dimensions, outputShapes[i].dimensions));
    }
    for (uint32_t i = 0; i < outputShapes.size(); i++) {
        mOutputs[i].dimensions = outputShapes[i].dimensions;
        mOutputs[i].isSufficient = outputShapes[i].isSufficient;
    }
    return true;
}

ErrorStatus ExecutionBuilder::finish(ErrorStatus, const std::vector<OutputShape>& outputShapes) {
    CHECK(!mFinished) << "ExecutionBuilder::finish is called twice";
    mFinished = true;
    if (!updateOutputShapes(outputShapes)) {
        return ErrorStatus::GENERAL_FAILURE;
    }
    return ErrorStatus::NONE;
}

bool StepExecutor::updateOutputShapes(const std::vector<OutputShape>& from,
                                      std::vector<OutputShape>* to) {
    if (from.size() == 0) {
        return true;
    }
    if (mExecutionStep != nullptr) {
        const auto& indexMapping = mExecutionStep->getOutputIndexSubModelToFromModel();
        NN_RET_CHECK_LE(indexMapping.size(), from.size());
        for (uint32_t i = 0, e = indexMapping.size(); i < e; i++) {
            uint32_t toIndex = indexMapping[i];
            NN_RET_CHECK_GT(to->size(), toIndex);
            NN_RET_CHECK(isUpdatable(to->at(toIndex).dimensions, from[i].dimensions));
            (*to)[toIndex] = from[i];
        }
    } else {
        NN_RET_CHECK_EQ(from.size(), to->size());
        for (uint32_t i = 0, e = from.size(); i < e; i++) {
            NN_RET_CHECK(isUpdatable(to->at(i).dimensions, from[i].dimensions));
            (*to)[i] = from[i];
        }
    }
    return true;
}

StepExecutor::StepExecutor(ExecutionBuilder* executionBuilder, const ModelBuilder* model,
                           std::shared_ptr<Device> device,
                           std::shared_ptr<PreparedModel> preparedModel)
    : mExecutionBuilder(executionBuilder),
      mModel(model),
      mDevice(device),
      mPreparedModel(preparedModel),
      mInputs(model->inputCount()),
      mOutputs(model->outputCount()) {
    CHECK(mDevice != nullptr);
}

void StepExecutor::mapInputsAndOutputsTrivially() {
    mInputs = mExecutionBuilder->mInputs;
    mOutputs = mExecutionBuilder->mOutputs;
    mMemories = mExecutionBuilder->mMemories;
}

void StepExecutor::mapInputOrOutput(const ModelArgumentInfo& builderInputOrOutput,
                                    ModelArgumentInfo* executorInputOrOutput) {
    *executorInputOrOutput = builderInputOrOutput;
    switch (executorInputOrOutput->state) {
        default:
            nnAssert(!"unexpected ModelArgumentInfo::state");
            break;
        case ModelArgumentInfo::HAS_NO_VALUE:
        case ModelArgumentInfo::POINTER:
        case ModelArgumentInfo::UNSPECIFIED:
            break;
        case ModelArgumentInfo::MEMORY: {
            const uint32_t builderPoolIndex = builderInputOrOutput.locationAndLength.poolIndex;
            const Memory* memory = mExecutionBuilder->mMemories[builderPoolIndex];
            const uint32_t executorPoolIndex = mMemories.add(memory);
            executorInputOrOutput->locationAndLength.poolIndex = executorPoolIndex;
            break;
        }
    }
}

int StepExecutor::setInputOrOutputFromTemporaryMemory(const Operand& inputOrOutputOperand,
                                                      const Memory* memory, uint32_t offset,
                                                      ModelArgumentInfo* inputOrOutputInfo) {
    // Should be similar to
    //     ExecutionBuilder::setInputFromMemory()
    //     ExecutionBuilder::setOutputFromMemory()

    uint32_t poolIndex = mMemories.add(memory);
    uint32_t length = TypeManager::get()->getSizeOfData(inputOrOutputOperand);
    return inputOrOutputInfo->setFromTemporaryMemory(inputOrOutputOperand, poolIndex, offset,
                                                     length);
}

static void logArguments(const char* kind, const std::vector<ModelArgumentInfo>& args) {
    for (unsigned i = 0; i < args.size(); i++) {
        const auto& arg = args[i];
        std::string prefix = kind + std::string("[") + std::to_string(i) + "] = ";
        switch (arg.state) {
            case ModelArgumentInfo::POINTER:
                VLOG(EXECUTION) << prefix << "POINTER(" << SHOW_IF_DEBUG(arg.buffer) << ")";
                break;
            case ModelArgumentInfo::MEMORY:
                VLOG(EXECUTION) << prefix << "MEMORY("
                                << "pool=" << arg.locationAndLength.poolIndex << ", "
                                << "off=" << arg.locationAndLength.offset << ")";
                break;
            case ModelArgumentInfo::HAS_NO_VALUE:
                VLOG(EXECUTION) << prefix << "HAS_NO_VALUE";
                break;
            case ModelArgumentInfo::UNSPECIFIED:
                VLOG(EXECUTION) << prefix << "UNSPECIFIED";
                break;
            default:
                VLOG(EXECUTION) << prefix << "state(" << arg.state << ")";
                break;
        }
    }
}

bool StepExecutor::isCpu() const {
    return mDevice == DeviceManager::getCpuDevice();
}

int StepExecutor::startCompute(sp<ExecutionCallback>* synchronizationCallback,
                               const std::shared_ptr<ExecutionBurstController>& burstController) {
    CHECK(mPreparedModel != nullptr);
    CHECK(synchronizationCallback != nullptr);
    *synchronizationCallback = nullptr;

    if (VLOG_IS_ON(EXECUTION)) {
        logArguments("input", mInputs);
        logArguments("output", mOutputs);
    }

    const MeasureTiming measure = measureTiming(mExecutionBuilder);
    const auto [n, outputShapes, timing] =
            mPreparedModel->execute(mInputs, mOutputs, mMemories, burstController, measure);
    mExecutionBuilder->reportTiming(timing);

    if (n != ANEURALNETWORKS_NO_ERROR && n != ANEURALNETWORKS_OUTPUT_INSUFFICIENT_SIZE) {
        return n;
    }

    const ErrorStatus status = convertResultCodeToErrorStatus(n);
    *synchronizationCallback = new ExecutionCallback();
    (*synchronizationCallback)->notify_1_2(status, outputShapes, timing);
    return ANEURALNETWORKS_NO_ERROR;
}

// For cpuFallback{Partial,Full}, recompile the model on CPU and then start compute.
int StepExecutor::startComputeOnCpuFallback(sp<ExecutionCallback>* synchronizationCallback) {
    NNTRACE_RT(NNTRACE_PHASE_EXECUTION, "StepExecutor::startComputeOnCpuFallback");
    VLOG(EXECUTION) << "Re-compile the model on CPU";
    const Model model = mModel->makeHidlModel();
    mDevice = DeviceManager::getCpuDevice();
    mPreparedModel = nullptr;
    // TODO: Propagate user preference to this point instead of using default value of
    // ANEURALNETWORKS_PREFER_FAST_SINGLE_ANSWER.
    const ExecutionPreference preference =
            static_cast<ExecutionPreference>(ANEURALNETWORKS_PREFER_FAST_SINGLE_ANSWER);
    const auto [n, preparedModel] = mDevice->prepareModel(model, preference, {}, {});
    mPreparedModel = preparedModel;
    NN_RETURN_IF_ERROR(n);
    return startCompute(synchronizationCallback, /*burstController=*/nullptr);
}

}  // namespace nn
}  // namespace android
