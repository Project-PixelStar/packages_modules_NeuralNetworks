/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef ANDROID_PACKAGES_MODULES_NEURALNETWORKS_COMMON_NNAPI_OPERATION_TYPES_H
#define ANDROID_PACKAGES_MODULES_NEURALNETWORKS_COMMON_NNAPI_OPERATION_TYPES_H

namespace android::nn {

/**
 * Operation types.
 *
 * The type of an operation in a model.
 */
enum class OperationType {
    /**
     * Adds two tensors, element-wise.
     *
     * Takes two input tensors of identical {@link OperandType} and compatible
     * dimensions. The output is the sum of both input tensors, optionally
     * modified by an activation function.
     *
     * Two dimensions are compatible when:
     *     1. they are equal, or
     *     2. one of them is 1
     *
     * The size of the output is the maximum size along each dimension of the
     * input operands. It starts with the trailing dimensions, and works its
     * way forward.
     *
     * Example:
     *
     *     input1.dimension = {4, 1, 2}
     *     input2.dimension = {5, 4, 3, 1}
     *     output.dimension = {5, 4, 3, 2}
     *
     * Since HAL version 1.2, generic zero-sized input tensor is supported. Zero
     * dimension is only compatible with 0 or 1. The size of the output
     * dimension is zero if either of corresponding input dimension is zero.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     * * {@link OperandType::TENSOR_INT32} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4
     *
     * Inputs:
     * * 0: A tensor.
     * * 1: A tensor of the same {@link OperandType}, and compatible dimensions
     *      as input0.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scales and zeroPoint can be different from input0 scale and zeroPoint.
     * * 2: An {@link OperandType::INT32} scalar, and has to be one of the
     *      {@link FusedActivationFunc} values. Specifies the activation to
     *      invoke on the result.
     *      For a {@link OperandType::TENSOR_INT32} tensor,
     *      the {@link FusedActivationFunc} must be "NONE".
     *
     * Outputs:
     * * 0: The sum, a tensor of the same {@link OperandType} as input0.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint can be different from inputs' scale and zeroPoint.
     */
    ADD = 0,

    /**
     * Performs a 2-D average pooling operation.
     *
     * The output dimensions are functions of the filter dimensions, stride, and
     * padding.
     *
     * The values in the output tensor are computed as:
     *
     *     output[b, i, j, channel] =
     *         sum_{di, dj}(
     *             input[b, strides[1] * i + di, strides[2] * j + dj, channel]
     *         ) / sum(1)
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: 4, with "NHWC" or "NCHW" data layout.
     * With the default data layout NHWC, the data is stored in the order of:
     * [batch, height, width, channels]. Alternatively, the data layout could
     * be NCHW, the data storage order of: [batch, channels, height, width].
     * NCHW is supported since HAL version 1.2.
     *
     * Both explicit padding and implicit padding are supported.
     *
     * Inputs (explicit padding):
     * * 0: A 4-D tensor, of shape [batches, height, width, depth], specifying
     *      the input.
     *      Since HAL version 1.2, zero batches is supported for this tensor.
     * * 1: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the left, in the ‘width’ dimension.
     * * 2: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the right, in the ‘width’ dimension.
     * * 3: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the top, in the ‘height’ dimension.
     * * 4: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the bottom, in the ‘height’ dimension.
     * * 5: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘width’ dimension.
     * * 6: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘height’ dimension.
     * * 7: An {@link OperandType::INT32} scalar, specifying the filter
     *      width.
     * * 8: An {@link OperandType::INT32} scalar, specifying the filter
     *      height.
     * * 9: An {@link OperandType::INT32} scalar, and has to be one of the
     *      {@link FusedActivationFunc} values. Specifies the activation to
     *      invoke on the result.
     * * 10: An optional {@link OperandType::BOOL} scalar, default to false.
     *       Set to true to specify NCHW data layout for input0 and output0.
     *       Available since HAL version 1.2.
     *
     * Inputs (implicit padding):
     * * 0: A 4-D tensor, of shape [batches, height, width, depth], specifying
     *      the input.
     *      Since HAL version 1.2, zero batches is supported for this tensor.
     * * 1: An {@link OperandType::INT32} scalar, specifying the implicit
     *      padding scheme, has to be one of the
     *      following values: {0 (NONE), 1 (SAME), 2 (VALID)}.
     * * 2: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘width’ dimension.
     * * 3: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘height’ dimension.
     * * 4: An {@link OperandType::INT32} scalar, specifying the filter
     *      width.
     * * 5: An {@link OperandType::INT32} scalar, specifying the filter
     *      height.
     * * 6: An {@link OperandType::INT32} scalar, and has to be one of the
     *      {@link FusedActivationFunc} values. Specifies the activation to
     *      invoke on the result.
     * * 7: An optional {@link OperandType::BOOL} scalar, default to false.
     *      Set to true to specify NCHW data layout for input0 and output0.
     *      Available since HAL version 1.2.
     *
     * Outputs:
     * * 0: The output 4-D tensor, of shape
     *      [batches, out_height, out_width, depth].
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    AVERAGE_POOL_2D = 1,

    /**
     * Concatenates the input tensors along the given dimension.
     *
     * The input tensors must have identical {@link OperandType} and the same
     * dimensions except the dimension along the concatenation axis.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     *   (full support since HAL version 1.2, see the input section)
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4
     *
     * Inputs:
     * * 0 ~ n-1: The list of n input tensors, of shape
     *            [D0, D1, ..., Daxis(i), ..., Dm].
     *            Before HAL version 1.2, all input tensors of
     *            {@link OperandType::TENSOR_QUANT8_ASYMM}
     *            must have the same scale and zeroPoint as the output tensor.
     *            Input tensors of
     *            {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}
     *            are allowed to have different scale and zeroPoint.
     *            Since HAL version 1.2, zero-sized tensors are supported.
     * * n: An {@link OperandType::INT32} scalar, specifying the
     *      concatenation axis.
     *
     * Outputs:
     * * 0: The output, a tensor of the same {@link OperandType} as the input
     *      tensors. The output shape is [D0, D1, ..., sum(Daxis(i)), ..., Dm].
     *      Since HAL version 1.2, for a {@link OperandType::TENSOR_QUANT8_ASYMM} tensor,
     *      the scale and zeroPoint values can be different from
     *      input tensors. Before HAL version 1.2 they have to be the same as for the
     *      input tensors.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint values can be different from input tensors.
     */
    CONCATENATION = 2,

    /**
     * Performs a 2-D convolution operation.
     *
     * The CONV_2D op sweeps a 2-D filter that can mix channels together over a
     * batch of images, applying the filter to each window of each image of the
     * appropriate size.
     *
     * The output dimensions are functions of the filter dimensions, stride, and
     * padding.
     *
     * The values in the output tensor are computed as:
     *
     *     output[b, i, j, channel] =
     *         sum_{di, dj, k} (
     *             input[b, strides[1] * i + di, strides[2] * j + dj, k] *
     *             filter[channel, di, dj, k]
     *         ) + bias[channel]
     *
     * Supported tensor {@link OperandType} configurations:
     * * 32 bit floating point:
     * * * {@link OperandType::TENSOR_FLOAT32} for input, filter, output, and bias.
     *
     * * Quantized:
     * * * {@link OperandType::TENSOR_QUANT8_ASYMM} for input, filter, and output.
     * * * {@link OperandType::TENSOR_INT32} for bias (with scale set to
     * * * input.scale * filter.scale).
     *
     * Available since HAL version 1.2:
     * * 16 bit floating point:
     * * * {@link OperandType::TENSOR_FLOAT16} for input, filter, output, and bias.
     *
     * * Quantized with symmetric per channel quantization for the filter:
     * * * {@link OperandType::TENSOR_QUANT8_ASYMM} for input, and output.
     * * * {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL} for filter.
     * * * {@link OperandType::TENSOR_INT32} for bias (scale set to 0.0,
     * * * each value scaling is separate and equal to input.scale * filter.scales[channel]).
     *
     * Supported tensor rank: 4, with "NHWC" or "NCHW" data layout.
     * With the default data layout NHWC, the data is stored in the order of:
     * [batch, height, width, channels]. Alternatively, the data layout could
     * be NCHW, the data storage order of: [batch, channels, height, width].
     * NCHW is supported since HAL version 1.2.
     *
     * Both explicit padding and implicit padding are supported.
     *
     * Inputs (explicit padding):
     * * 0: A 4-D tensor, of shape [batches, height, width, depth_in],
     *      specifying the input.
     *      Since HAL version 1.2, zero batches is supported for this tensor.
     * * 1: A 4-D tensor, of shape
     *      [depth_out, filter_height, filter_width, depth_in], specifying the
     *      filter.
     *      For tensor of type {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL}
     *      the channel dimension (SymmPerChannelQuantParams::channelDim)
     *      must be set to 0.
     * * 2: A 1-D tensor, of shape [depth_out], specifying the bias. For input
     *      tensor of type {@link OperandType::TENSOR_FLOAT32}
     *      or {@link OperandType::TENSOR_FLOAT16} the bias must be of the same type.
     *      For filter tensor of {@link OperandType::TENSOR_QUANT8_ASYMM}
     *      and {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED},
     *      the bias should be of {@link OperandType::TENSOR_INT32}, with zeroPoint
     *      of 0 and bias_scale == input_scale * filter_scale.
     *      For filter tensor of {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL},
     *      the bias should be of {@link OperandType::TENSOR_INT32}, with zeroPoint of 0
     *      and bias_scale of 0. The actual scale of each value 'i' is equal to
     *      bias_scale[i] = input_scale * filter_scale[i].
     * * 3: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the left, in the ‘width’ dimension.
     * * 4: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the right, in the ‘width’ dimension.
     * * 5: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the top, in the ‘height’ dimension.
     * * 6: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the bottom, in the ‘height’ dimension.
     * * 7: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘width’ dimension.
     * * 8: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘height’ dimension.
     * * 9: An {@link OperandType::INT32} scalar, and has to be one of the
     *      {@link FusedActivationFunc} values. Specifies the activation to
     *      invoke on the result.
     * * 10: An optional {@link OperandType::BOOL} scalar, default to false.
     *      Set to true to specify NCHW data layout for input0 and output0.
     *      Available since HAL version 1.2.
     * * 11: An optional {@link OperandType::INT32} scalar, specifying the dilation
     *      factor for width. Defaults to 1. If set to k > 1, there will be k-1 skipped
     *      cells between each filter element on width dimension. If this input is set,
     *      input 12 (dilation factor for height) must be specified as well.
     *      Available since HAL version 1.2.
     * * 12: An optional {@link OperandType::INT32} scalar, specifying the dilation
     *      factor for height. Defaults to 1. If set to k > 1, there will be k-1 skipped
     *      cells between each filter element on height dimension. If this input is set,
     *      input 11 (dilation factor for width) must be specified as well.
     *      Available since HAL version 1.2.
     *
     * Inputs (implicit padding):
     * * 0: A 4-D tensor, of shape [batches, height, width, depth_in],
     *      specifying the input.
     *      Since HAL version 1.2, zero batches is supported for this tensor.
     * * 1: A 4-D tensor, of shape
     *      [depth_out, filter_height, filter_width, depth_in], specifying the
     *      filter.
     *      For tensor of type {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL}
     *      the channel dimension (SymmPerChannelQuantParams::channelDim)
     *      must be set to 0.
     * * 2: A 1-D tensor, of shape [depth_out], specifying the bias. For input
     *      tensor of type {@link OperandType::TENSOR_FLOAT32}
     *      or {@link OperandType::TENSOR_FLOAT16} the bias must be of the same
     *      type.
     *      For filter tensor of {@link OperandType::TENSOR_QUANT8_ASYMM}
     *      and {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED},
     *      the bias should be of {@link OperandType::TENSOR_INT32}, with zeroPoint
     *      of 0 and bias_scale == input_scale * filter_scale.
     *      For filter tensor of {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL},
     *      the bias should be of {@link OperandType::TENSOR_INT32}, with zeroPoint of 0
     *      and bias_scale of 0. The actual scale of each value 'i' is equal to
     *      bias_scale[i] = input_scale * filter_scale[i].
     * * 3: An {@link OperandType::INT32} scalar, specifying the implicit
     *      padding scheme, has to be one of the
     *      following values: {0 (NONE), 1 (SAME), 2 (VALID)}.
     * * 4: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘width’ dimension.
     * * 5: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘height’ dimension.
     * * 6: An {@link OperandType::INT32} scalar, and has to be one of the
     *      {@link FusedActivationFunc} values. Specifies the activation to
     *      invoke on the result.
     * * 7: An optional {@link OperandType::BOOL} scalar, default to false.
     *      Set to true to specify NCHW data layout for input0 and output0.
     *      Available since HAL version 1.2.
     * * 8: An optional {@link OperandType::INT32} scalar, specifying the dilation
     *      factor for width. Defaults to 1. If set to k > 1, there will be k-1 skipped
     *      cells between each filter element on width dimension. If this input is set,
     *      input 9 (dilation factor for height) must be specified as well.
     *      Available since HAL version 1.2.
     * * 9: An optional {@link OperandType::INT32} scalar, specifying the dilation
     *      factor for height. Defaults to 1. If set to k > 1, there will be k-1 skipped
     *      cells between each filter element on height dimension. If this input is set,
     *      input 8 (dilation factor for width) must be specified as well.
     *      Available since HAL version 1.2.
     *
     * Outputs:
     * * 0: The output 4-D tensor, of shape
     *      [batches, out_height, out_width, depth_out].
     *      Before HAL version 1.2, for output tensor of
     *      {@link OperandType::TENSOR_QUANT8_ASYMM}, the following condition must
     *      be satisfied: output_scale > input_scale * filter_scale
     */
    CONV_2D = 3,

    /**
     * Performs a depthwise 2-D convolution operation.
     *
     * Given an input tensor of shape [batches, height, width, depth_in] and a
     * filter tensor of shape [1, filter_height, filter_width, depth_out]
     * containing depth_out convolutional filters of depth 1, DEPTHWISE_CONV
     * applies a different filter to each input channel (expanding from 1
     * channel to channel_multiplier channels for each), then concatenates the
     * results together.
     *
     * The output has depth_out = depth_in * depth_multiplier channels.
     * The output dimensions are functions of the filter dimensions, stride, and
     * padding.
     *
     * The values in the output tensor are computed as:
     *
     *     output[b, i, j, k * channel_multiplier + q] =
     *         sum_{di, dj} (
     *             input[b, strides[1] * i + di, strides[2] * j + dj, k] *
     *             filter[1, di, dj, k * channel_multiplier + q]
     *         ) + bias[k * channel_multiplier + q]
     *
     * Supported tensor {@link OperandType} configurations:
     * * 32 bit floating point:
     * * * {@link OperandType::TENSOR_FLOAT32} for input, filter, output, and bias.
     *
     * * Quantized:
     * * * {@link OperandType::TENSOR_QUANT8_ASYMM} for input, filter, and output.
     * * * {@link OperandType::TENSOR_INT32} for bias (with scale set to
     * * * input.scale * filter.scale).
     *
     * Available since HAL version 1.2:
     * * 16 bit floating point:
     * * * {@link OperandType::TENSOR_FLOAT16} for input, filter, output, and bias.
     *
     * * Quantized with symmetric per channel quantization for the filter:
     * * * {@link OperandType::TENSOR_QUANT8_ASYMM} for input, and output.
     * * * {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL} for filter.
     * * * {@link OperandType::TENSOR_INT32} for bias (scale set to 0.0,
     * * * each value scaling is separate and equal to input.scale * filter.scales[channel]).
     *
     * Available since HAL version 1.3:
     * * Quantized signed (since HAL version 1.3):
     * * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} for input, filter, and output.
     * * * {@link OperandType::TENSOR_INT32} for bias (with scale set to
     * * * input.scale * filter.scale).
     *
     * * Quantized signed with filter symmetric per channel quantization
     *   (since HAL version 1.3):
     * * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} for input, and output.
     * * * {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL} for filter.
     * * * {@link OperandType::TENSOR_INT32} for bias (scale set to 0.0,
     * * * each value scaling is separate and equal to input.scale * filter.scales[channel]).
     *
     * Supported tensor rank: 4, with "NHWC" or "NCHW" data layout.
     * With the default data layout NHWC, the data is stored in the order of:
     * [batch, height, width, channels]. Alternatively, the data layout could
     * be NCHW, the data storage order of: [batch, channels, height, width].
     * NCHW is supported since HAL version 1.2.
     *
     * Both explicit padding and implicit padding are supported.
     *
     * Inputs (explicit padding):
     * * 0: A 4-D tensor, of shape [batches, height, width, depth_in],
     *      specifying the input.
     * * 1: A 4-D tensor, of shape [1, filter_height, filter_width, depth_out],
     *      specifying the filter.
     *      For tensor of type {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL}
     *      the channel dimension (SymmPerChannelQuantParams::channelDim)
     *      must be set to 3.
     * * 2: A 1-D tensor, of shape [depth_out], specifying the bias. For input
     *      tensor of type {@link OperandType::TENSOR_FLOAT32}
     *      or {@link OperandType::TENSOR_FLOAT16} the bias must be of the same type.
     *      For filter tensor of {@link OperandType::TENSOR_QUANT8_ASYMM}
     *      and {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED},
     *      the bias should be of {@link OperandType::TENSOR_INT32}, with zeroPoint
     *      of 0 and bias_scale == input_scale * filter_scale.
     *      For filter tensor of {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL},
     *      the bias should be of {@link OperandType::TENSOR_INT32}, with zeroPoint of 0
     *      and bias_scale of 0. The actual scale of each value 'i' is equal to
     *      bias_scale[i] = input_scale * filter_scale[i].
     * * 3: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the left, in the ‘width’ dimension.
     * * 4: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the right, in the ‘width’ dimension.
     * * 5: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the top, in the ‘height’ dimension.
     * * 6: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the bottom, in the ‘height’ dimension.
     * * 7: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘width’ dimension.
     * * 8: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘height’ dimension.
     * * 9: An {@link OperandType::INT32} scalar, specifying the depthwise
     *      multiplier.
     * * 10: An {@link OperandType::INT32} scalar, and has to be one of the
     *       {@link FusedActivationFunc} values. Specifies the activation to
     *       invoke on the result.
     * * 11: An optional {@link OperandType::BOOL} scalar, default to false.
     *       Set to true to specify NCHW data layout for input0 and output0.
     *       Available since HAL version 1.2.
     * * 12: An optional {@link OperandType::INT32} scalar, specifying the dilation
     *      factor for width. Defaults to 1. If set to k > 1, there will be k-1 skipped
     *      cells between each filter element on width dimension. If this input is set,
     *      input 13 (dilation factor for height) must be specified as well.
     *      Available since HAL version 1.2.
     * * 13: An optional {@link OperandType::INT32} scalar, specifying the dilation
     *      factor for height. Defaults to 1. If set to k > 1, there will be k-1 skipped
     *      cells between each filter element on height dimension. If this input is set,
     *      input 12 (dilation factor for width) must be specified as well.
     *      Available since HAL version 1.2.
     *
     * Inputs (implicit padding):
     * * 0: A 4-D tensor, of shape [batches, height, width, depth_in],
     *      specifying the input.
     * * 1: A 4-D tensor, of shape [1, filter_height, filter_width, depth_out],
     *      specifying the filter.
     * * 2: A 1-D tensor, of shape [depth_out], specifying the bias. For input
     *      tensor of type {@link OperandType::TENSOR_FLOAT32}
     *      or {@link OperandType::TENSOR_FLOAT16} the bias must be of the same type.
     *      For filter tensor of {@link OperandType::TENSOR_QUANT8_ASYMM}
     *      and {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED},
     *      the bias should be of {@link OperandType::TENSOR_INT32}, with zeroPoint
     *      of 0 and bias_scale == input_scale * filter_scale.
     *      For filter tensor of {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL},
     *      the bias should be of {@link OperandType::TENSOR_INT32}, with zeroPoint of 0
     *      and bias_scale of 0. The actual scale of each value 'i' is equal to
     *      bias_scale[i] = input_scale * filter_scale[i].
     * * 3: An {@link OperandType::INT32} scalar, specifying the implicit
     *      padding scheme, has to be one of the
     *      following values: {0 (NONE), 1 (SAME), 2 (VALID)}.
     * * 4: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘width’ dimension.
     * * 5: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘height’ dimension.
     * * 6: An {@link OperandType::INT32} scalar, specifying the depthwise
     *      multiplier.
     * * 7: An {@link OperandType::INT32} scalar, and has to be one of the
     *      {@link FusedActivationFunc} values. Specifies the activation to
     *      invoke on the result.
     * * 8: An optional {@link OperandType::BOOL} scalar, default to false.
     *      Set to true to specify NCHW data layout for input0 and output0.
     *      Available since HAL version 1.2.
     * * 9: An optional {@link OperandType::INT32} scalar, specifying the dilation
     *      factor for width. Defaults to 1. If set to k > 1, there will be k-1 skipped
     *      cells between each filter element on width dimension. If this input is set,
     *      input 10 (dilation factor for height) must be specified as well.
     *      Available since HAL version 1.2.
     * * 10: An optional {@link OperandType::INT32} scalar, specifying the dilation
     *      factor for height. Defaults to 1. If set to k > 1, there will be k-1 skipped
     *      cells between each filter element on height dimension. If this input is set,
     *      input 9 (dilation factor for width) must be specified as well.
     *      Available since HAL version 1.2.
     *
     * Outputs:
     * * 0: The output 4-D tensor, of shape
     *      [batches, out_height, out_width, depth_out]. Before HAL version 1.2, for
     *      output tensor of {@link OperandType::TENSOR_QUANT8_ASYMM},
     *      the following condition must be satisfied:
     *      output_scale > input_scale * filter_scale
     */
    DEPTHWISE_CONV_2D = 4,

    /**
     * Rearranges data from depth into blocks of spatial data.
     *
     * More specifically, this op outputs a copy of the input tensor where
     * values from the depth dimension are moved in spatial blocks to the height
     * and width dimensions. The value block_size indicates the input block size
     * and how the data is moved.
     *
     * Chunks of data of size block_size * block_size from depth are rearranged
     * into non-overlapping blocks of size block_size x block_size.
     *
     * The width of the output tensor is input_depth * block_size, whereas the
     * height is input_height * block_size. The depth of the input tensor must
     * be divisible by block_size * block_size
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: 4, with "NHWC" or "NCHW" data layout.
     * With the default data layout NHWC, the data is stored in the order of:
     * [batch, height, width, channels]. Alternatively, the data layout could
     * be NCHW, the data storage order of: [batch, channels, height, width].
     * NCHW is supported since HAL version 1.2.
     *
     * Inputs:
     * * 0: A 4-D tensor, of shape [batches, height, width, depth_in],
     *      specifying the input.
     * * 1: An {@link OperandType::INT32} scalar, specifying the block_size.
     *      block_size must be >=1 and block_size * block_size must be a divisor
     *      of the input depth.
     * * 2: An optional {@link OperandType::BOOL} scalar, default to false.
     *      Set to true to specify NCHW data layout for input0 and output0.
     *      Available since HAL version 1.2.
     *
     * Outputs:
     * * 0: The output 4-D tensor, of shape [batch, height*block_size,
     *      width*block_size, depth/(block_size*block_size)].
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    DEPTH_TO_SPACE = 5,

    /**
     * Dequantizes the input tensor.
     *
     * The formula is:
     *
     *     output = (input - zeroPoint) * scale.
     *
     * Supported input tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_SYMM} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported output tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}.
     *
     * Supported tensor rank: up to 4
     *
     * Inputs:
     * * 0: A tensor.
     *      Since HAL version 1.2, this tensor may be zero-sized.
     *
     * Outputs:
     * * 0: A tensor with the same shape as input0.
     */
    DEQUANTIZE = 6,

    /**
     * Looks up sub-tensors in the input tensor.
     *
     * This operator takes for input a tensor of values (Values) and
     * a one-dimensional tensor of selection indices (Lookups).
     * The output tensor is the concatenation of sub-tensors of Values as
     * selected by Lookups.
     *
     * Think of Values as being sliced along its first dimension:
     * The entries in Lookups select which slices are concatenated together
     * to create the output tensor.
     *
     * For example, if Values has shape of [40, 200, 300] and
     * Lookups has shape of [3], all three values found in Lookups are
     * expected to be between 0 and 39. The resulting tensor must
     * have shape of [3, 200, 300].
     *
     * If a value in Lookups is out of bounds, the operation must fail
     * and an error must be reported.
     *
     * Supported value tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.3)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_QUANT8_ASYMM} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported value tensor rank: from 2
     *
     * Inputs:
     * * 0: Lookups. A 1-D tensor of {@link OperandType::TENSOR_INT32}.
     *      The values are indices into the first dimension of Values.
     * * 1: Values. An n-D tensor, where n >= 2, from which sub-tensors are
     *      extracted.
     *
     * Output:
     * * 0: A n-D tensor with the same rank and shape as the Values
     *      tensor, except for the first dimension which has the same size
     *      as Lookups' only dimension.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input1.
     */
    EMBEDDING_LOOKUP = 7,

    /**
     * Computes element-wise floor() on the input tensor.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * Supported tensor rank: up to 4
     *
     * Inputs:
     * * 0: A tensor.
     *
     * Outputs:
     * * 0: The output tensor, of the same {@link OperandType} and dimensions as
     *      the input tensor.
     */
    FLOOR = 8,

    /**
     * Denotes a fully (densely) connected layer, which connects all elements
     * in the input tensor with each element in the output tensor.
     *
     * This layer implements the operation:
     *
     *     outputs = activation(inputs * weights’ + bias)
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4.
     *
     * Inputs:
     * * 0: A tensor of at least rank 2, specifying the input. If rank is
     *      greater than 2, then it gets flattened to a 2-D Tensor. The
     *      (flattened) 2-D Tensor is reshaped (if necessary) to
     *      [batch_size, input_size], where "input_size" corresponds to the
     *      number of inputs to the layer, matching the second dimension of
     *      weights, and "batch_size" is calculated by dividing the number of
     *      elements by "input_size".
     *      Since HAL version 1.2, zero batch_size is supported for this tensor.
     * * 1: A 2-D tensor, specifying the weights, of shape
     *      [num_units, input_size], where "num_units" corresponds to the number
     *      of output nodes.
     * * 2: A 1-D tensor, of shape [num_units], specifying the bias. For input
     *      tensor of {@link OperandType::TENSOR_FLOAT32}, the bias should
     *      also be of {@link OperandType::TENSOR_FLOAT32}.
     *      For input tensor of {@link OperandType::TENSOR_QUANT8_ASYMM}
     *      and {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED},
     *      the bias should be of {@link OperandType::TENSOR_INT32},
     *      with zeroPoint of 0 and bias_scale == input_scale * filter_scale.
     * * 3: An {@link OperandType::INT32} scalar, and has to be one of the
     *      {@link FusedActivationFunc} values. Specifies the activation to
     *      invoke on the result.
     *
     * Outputs:
     * * 0: The output tensor, of shape [batch_size, num_units]. Before HAL version 1.2, for
     *      output tensor of {@link OperandType::TENSOR_QUANT8_ASYMM}, the following
     *      condition must be satisfied: output_scale > input_scale * filter_scale.
     */
    FULLY_CONNECTED = 9,

    /**
     * Looks up sub-tensors in the input tensor using a key-value map.
     *
     * This operator takes for input a tensor of values (Values),
     * a one-dimensional tensor of selection values (Lookups) and
     * a one-dimensional tensor that maps these values to Values
     * indexes. The output tensor is the concatenation of sub-tensors of
     * Values as selected by Lookups via Keys.
     *
     * Think of Values as being sliced along its outer-most dimension.
     * The output is a concatenation of selected slices, with one slice
     * for each entry of Lookups. The slice selected is the one at the
     * same index as the Maps entry that matches the value in Lookups.
     *
     * For a hit, the corresponding sub-tensor of Values is included
     * in the Output tensor. For a miss, the corresponding sub-tensor in
     * Output must have zero values.
     *
     * For example, if Values has shape of [40, 200, 300],
     * Keys should have a shape of [40]. If Lookups tensor has shape
     * of [3], three slices are being concatenated, so the resulting tensor
     * must have the shape of [3, 200, 300]. If the first entry in Lookups
     * has the value 123456, that value must be located in Keys tensor.
     * If the sixth entry of Keys contains 123456, the sixth slice of Values
     * must be selected. If no entry in Keys has 123456, a slice of zeroes
     * must be concatenated.
     *
     * Supported value tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     *
     * Supported value tensor rank: from 2
     *
     * Inputs:
     * * 0: Lookups. A 1-D {@link OperandType::TENSOR_INT32} tensor with
     *      shape [ k ].
     * * 1: Keys. A 1-D {@link OperandType::TENSOR_INT32} tensor with shape
     *      [ n ]; Keys and Values pair represent a map, i.e., the ith element
     *      in Keys (Keys[i]) is the key to select the ith sub-tensor in Values
     *      (Values[i]), where 0 <= i <= n-1. Keys tensor *MUST* be sorted in
     *      ascending order.
     * * 2: Values. A tensor with shape of [ n, … ]; i.e., the first dimension
     *      must be n.
     *
     * Outputs:
     * * 0: Output. A tensor with shape [ k …].
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} tensor,
     *      the scale and zeroPoint must be the same as input2.
     * * 1: Hits. A boolean tensor with shape [ k ] indicates whether the lookup
     *      hits (True) or not (False).
     *      Stored as {@link OperandType::TENSOR_QUANT8_ASYMM} with offset 0
     *      and scale 1.0f.
     *      A non-zero byte represents True, a hit. A zero indicates otherwise.
     */
    HASHTABLE_LOOKUP = 10,

    /**
     * Applies L2 normalization along the axis dimension.
     *
     * The values in the output tensor are computed as:
     *
     *     output[batch, row, col, channel] =
     *         input[batch, row, col, channel] /
     *         sqrt(sum_{c} pow(input[batch, row, col, c], 2))
     *
     * By default the axis dimension is the last dimension of the input tensor.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4
     * Tensors with rank less than 4 are only supported since HAL version 1.2.
     *
     * Inputs:
     * * 0: An n-D tensor, specifying the tensor to be normalized.
     * * 1: An optional {@link OperandType::INT32} scalar, default to -1,
     *      specifying the dimension normalization would be performed on.
     *      Negative index is used to specify axis from the end (e.g. -1 for
     *      the last axis). Must be in the range [-n, n).
     *      Available since HAL version 1.2.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} and same shape as input0.
     *      For {@link OperandType::TENSOR_QUANT8_ASYMM},
     *      the scale must be 1.f / 128 and the zeroPoint must be 128.
     *      For {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED},
     *      the scale must be 1.f / 128 and the zeroPoint must be 0.
     *
     *      NOTE: Before HAL version 1.3, if the elements along an axis are all zeros,
     *      the result is undefined. Since HAL version 1.3, if the elements along an axis
     *      are all zeros, the result is logical zero.
     */
    L2_NORMALIZATION = 11,

    /**
     * Performs an 2-D L2 pooling operation.
     *
     * The output dimensions are functions of the filter dimensions, stride, and
     * padding.
     *
     * The values in the output tensor are computed as:
     *
     *     output[b, i, j, c] =
     *         sqrt(sum_{di, dj} pow(input[b, strides[1] * i + di, strides[2] * j + dj, c], 2) /
     *              sum(1))
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * Supported tensor rank: 4, with "NHWC" or "NCHW" data layout.
     * With the default data layout NHWC, the data is stored in the order of:
     * [batch, height, width, channels]. Alternatively, the data layout could
     * be NCHW, the data storage order of: [batch, channels, height, width].
     * NCHW is supported since HAL version 1.2.
     *
     * Both explicit padding and implicit padding are supported.
     *
     * Inputs (explicit padding):
     * * 0: A 4-D tensor, of shape [batches, height, width, depth], specifying
     *      the input.
     *      Since HAL version 1.2, zero batches is supported for this tensor.
     * * 1: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the left, in the ‘width’ dimension.
     * * 2: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the right, in the ‘width’ dimension.
     * * 3: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the top, in the ‘height’ dimension.
     * * 4: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the bottom, in the ‘height’ dimension.
     * * 5: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘width’ dimension.
     * * 6: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘height’ dimension.
     * * 7: An {@link OperandType::INT32} scalar, specifying the filter
     *      width.
     * * 8: An {@link OperandType::INT32} scalar, specifying the filter
     *      height.
     * * 9: An {@link OperandType::INT32} scalar, and has to be one of the
     *      {@link FusedActivationFunc} values. Specifies the activation to
     *      invoke on the result.
     * * 10: An optional {@link OperandType::BOOL} scalar, default to false.
     *       Set to true to specify NCHW data layout for input0 and output0.
     *       Available since HAL version 1.2.
     *
     * Inputs (implicit padding):
     * * 0: A 4-D tensor, of shape [batches, height, width, depth], specifying
     *      the input.
     *      Since HAL version 1.2, zero batches is supported for this tensor.
     * * 1: An {@link OperandType::INT32} scalar, specifying the implicit
     *      padding scheme, has to be one of the
     *      following values: {0 (NONE), 1 (SAME), 2 (VALID)}.
     * * 2: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘width’ dimension.
     * * 3: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘height’ dimension.
     * * 4: An {@link OperandType::INT32} scalar, specifying the filter
     *      width.
     * * 5: An {@link OperandType::INT32} scalar, specifying the filter
     *      height.
     * * 6: An {@link OperandType::INT32} scalar, and has to be one of the
     *      {@link FusedActivationFunc} values. Specifies the activation to
     *      invoke on the result.
     * * 7: An optional {@link OperandType::BOOL} scalar, default to false.
     *      Set to true to specify NCHW data layout for input0 and output0.
     *      Available since HAL version 1.2.
     *
     * Outputs:
     * * 0: The output 4-D tensor, of shape
     *      [batches, out_height, out_width, depth].
     */
    L2_POOL_2D = 12,

    /**
     * Applies Local Response Normalization along the depth dimension.
     *
     * The 4-D input tensor is treated as a 3-D array of 1-D vectors (along the
     * last dimension), and each vector is normalized independently. Within a
     * given vector, each component is divided by the weighted, squared sum of
     * inputs within depth_radius.
     *
     * The output is calculated using this formula:
     *
     *     sqr_sum[a, b, c, d] = sum(
     *         pow(input[a, b, c, d - depth_radius : d + depth_radius + 1], 2))
     *     output = input / pow((bias + alpha * sqr_sum), beta)
     *
     * For input tensor with rank less than 4, independently normalizes each
     * 1-D slice along specified dimension.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * Supported tensor rank: up to 4
     * Tensors with rank less than 4 are only supported since HAL version 1.2.
     *
     * Inputs:
     * * 0: A 4-D tensor, of shape [batches, height, width, depth], specifying
     *      the input.
     * * 1: An {@link OperandType::INT32} scalar, specifying the radius of
     *      the normalization window.
     * * 2: A scalar, specifying the bias, must not be zero.
     *      For input tensor of {@link OperandType::TENSOR_FLOAT16}, the bias
     *      value must be of {@link OperandType::FLOAT16}.
     *      For input tensor of {@link OperandType::TENSOR_FLOAT32}, the bias
     *      value must be of {@link OperandType::FLOAT32}.
     * * 3: A scalar, specifying the scale factor, alpha.
     *      For input tensor of {@link OperandType::TENSOR_FLOAT16}, the
     *      alpha value must be of {@link OperandType::FLOAT16}.
     *      For input tensor of {@link OperandType::TENSOR_FLOAT32}, the
     *      alpha value must be of {@link OperandType::FLOAT32}.
     * * 4: A scalar, specifying the exponent, beta.
     *      For input tensor of {@link OperandType::TENSOR_FLOAT16}, the beta
     *      value must be of {@link OperandType::FLOAT16}.
     *      For input tensor of {@link OperandType::TENSOR_FLOAT32}, the beta
     *      value must be of {@link OperandType::FLOAT32}.
     * * 5: An optional {@link OperandType::INT32} scalar, default to -1,
     *      specifying the dimension normalization would be performed on.
     *      Negative index is used to specify axis from the end (e.g. -1 for
     *      the last axis). Must be in the range [-n, n).
     *      Available since HAL version 1.2.
     *
     * Outputs:
     * * 0: The output tensor of same shape as input0.
     */
    LOCAL_RESPONSE_NORMALIZATION = 13,

    /**
     * Computes sigmoid activation on the input tensor element-wise.
     *
     * The output is calculated using this formula:
     *
     *     output = 1 / (1 + exp(-input))
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4.
     *
     * Inputs:
     * * 0: A tensor, specifying the input.
     *      Since HAL version 1.2, this tensor may be zero-sized.
     *
     * Outputs:
     * * 0: The output tensor of same shape as input0.
     *      For {@link OperandType::TENSOR_QUANT8_ASYMM},
     *      the scale must be 1.f / 256 and the zeroPoint must be 0.
     *      For {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED},
     *      the scale must be 1.f / 256 and the zeroPoint must be -128.
     */
    LOGISTIC = 14,

    /**
     * Projects an input to a bit vector via locality senstive hashing.
     *
     * Supported input tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     *
     * Supported input tensor rank: from 1
     *
     * Inputs:
     * * 0: Hash functions. Dim.size == 2, DataType: Float.
     *      Tensor[0].Dim[0]: Number of hash functions.
     *      Tensor[0].Dim[1]: Number of projected output bits generated by each
     *      hash function.
     *      If the projection type is Sparse:
     *      Tensor[0].Dim[1] + ceil(log2(Tensor[0].Dim[0])) <= 32
     *
     * * 1: Input. Dim.size >= 1, no restriction on DataType.
     * * 2: Weight. Optional. Dim.size == 1, DataType: Float.
     *      If not set, each input element is considered to have the same weight
     *      of 1.0.
     *      Tensor[1].Dim[0] == Tensor[2].Dim[0]
     * * 3: Type:
     *        Sparse:
     *          Value LSHProjectionType_SPARSE(=3) (since HAL version 1.2).
     *          Computed bit vector is considered to be sparse.
     *          Each output element is an int32 made up of multiple bits
     *          computed from hash functions.
     *
     *          NOTE: To avoid collisions across hash functions, an offset value
     *          of k * (1 << Tensor[0].Dim[1]) will be added to each signature,
     *          where k is the index of the hash function.
     *
     *          Value LSHProjectionType_SPARSE_DEPRECATED(=1).
     *          Legacy behavior that does not include the offset value.
     *
     *        Dense:
     *          Value LSHProjectionType_DENSE(=2).
     *          Computed bit vector is considered to be dense. Each output
     *          element represents a bit and can take the value of either
     *          0 or 1.
     *
     * Outputs:
     * * 0: If the projection type is Sparse:
     *      Output.Dim == { Tensor[0].Dim[0] }
     *      A tensor of int32 that represents hash signatures.
     *
     *      If the projection type is Dense:
     *      Output.Dim == { Tensor[0].Dim[0] * Tensor[0].Dim[1] }
     *      A flattened tensor that represents projected bit vectors.
     * The offset value for sparse projections was added in HAL version 1.2.
     */
    LSH_PROJECTION = 15,

    /**
     * Performs a single time step in a Long Short-Term Memory (LSTM) layer
     *
     * The LSTM operation is described by the following equations.
     *
     * \f{eqnarray*}{
     * i_t =& \sigma(W_{xi}x_t+W_{hi}h_{t-1}+W_{ci}C_{t-1}+b_i) & \\
     * f_t =& \sigma(W_{xf}x_t+W_{hf}h_{t-1}+W_{cf}C_{t-1}+b_f) & \\
     * C_t =& clip(f_t \odot C_{t-1} + i_t \odot
     *        g(W_{xc}x_t+W_{hc}h_{t-1}+b_c),\ t_{cell}) & \\
     * o_t =& \sigma(W_{xo}x_t+W_{ho}h_{t-1}+W_{co}C_t+b_o) & \\
     *      & & \\
     *      & clip(W_{proj}(o_t \odot g(C_t))+b_{proj},\ t_{proj})
     *      & if\ there\ is\ a\ projection; \\
     * h_t =& & \\
     *      & o_t \odot g(C_t) & otherwise. \\
     * \f}
     * Where:
     * * \f$x_t\f$ is the input,
     * * \f$i_t\f$ is the input gate,
     * * \f$f_t\f$ is the forget gate,
     * * \f$C_t\f$ is the cell state,
     * * \f$o_t\f$ is the output,
     * * \f$h_t\f$ is the output state,
     * * \f$\sigma\f$ is the logistic sigmoid function,
     * * \f$g\f$ is the cell input and cell output activation function, usually
     *   \f$tahn\f$,
     * * \f$W_{xi}\f$ is the input-to-input weight matrix,
     * * \f$W_{hi}\f$ is the recurrent to input weight matrix,
     * * \f$W_{ci}\f$ is the cell-to-input weight matrix,
     * * \f$b_i\f$ is the input gate bias,
     * * \f$W_{xf}\f$ is the input-to-forget weight matrix,
     * * \f$W_{hf}\f$ is the recurrent-to-forget weight matrix,
     * * \f$W_{cf}\f$ is the cell-to-forget weight matrix,
     * * \f$b_f\f$ is the forget gate bias,
     * * \f$W_{xc}\f$ is the input-to-cell weight matrix,
     * * \f$W_{hc}\f$ is the recurrent-to-cell weight matrix,
     * * \f$b_c\f$ is the cell bias,
     * * \f$W_{xo}\f$ is the input-to-output weight matrix,
     * * \f$W_{ho}\f$ is the recurrent-to-output weight matrix,
     * * \f$W_{co}\f$ is the cell-to-output weight matrix,
     * * \f$b_o\f$ is the output gate bias,
     * * \f$W_{proj}\f$ is the projection weight matrix,
     * * \f$b_{proj}\f$ is the projection bias,
     * * \f$t_{cell}\f$ is the threshold for clipping the cell state, and
     * * \f$t_{proj}\f$ is the threshold for clipping the projected output.
     * * \f$\odot\f$ is the
     *   <a href="https://en.wikipedia.org/wiki/Hadamard_product_(matrices)">
     *   Hadamard product</a> that takes two matrices and produces another
     *   matrix, each element of which is the product of the corresponding
     *   elements of the input matrices.
     *
     * Since HAL version 1.2 LSTM supports layer normalization.
     * In case layer normalization is used, the inputs to internal activation
     * functions (sigmoid and \f$g\f$) are normalized, rescaled and recentered
     * following an approach from section 3.1 from
     * https://arxiv.org/pdf/1607.06450.pdf
     *
     * The operation has the following independently optional inputs:
     * * The cell-to-input weights (\f$W_{ci}\f$), cell-to-forget weights
     *   (\f$W_{cf}\f$) and cell-to-output weights (\f$W_{co}\f$) either all
     *   have values or neither of them have values (i.e., all set to null). If
     *   they have values, the peephole optimization is used.
     * * The input-to-input weights (\f$W_{xi}\f$), recurrent-to-input weights
     *   (\f$W_{hi}\f$) and input gate bias (\f$b_i\f$) either all have values,
     *   or none of them have values. If they have no values, coupling of input
     *   and forget gates (CIFG) is used, in which case the input gate
     *   (\f$i_t\f$) is calculated using the following equation instead.
     *   \f{eqnarray*}{
     *   i_t = 1 - f_t
     *   \f}
     *   In case peephole optimization is used and CIFG is not used
     *   cell-to-input (\f$W_{ci}\f$) weights must be present. Otherwise, the
     *   cell-to-input weights must have no value.
     * * The projection weights (\f$W_{proj}\f$) is required only for the
     *   recurrent projection layer, and should otherwise have no value.
     * * The projection bias (\f$b_{proj}\f$) may (but not required to) have a
     *   value if the recurrent projection layer exists, and should otherwise
     *   have no value.
     * * (HAL version 1.2 or later) The four layer normalization weights either all have
     *   values or none of them have values. Additionally, if CIFG is used,
     *   input layer normalization weights tensor is omitted and the other layer
     *   normalization weights either all have values or none of them have
     *   values. Layer normalization is used when the values of all the layer
     *   normalization weights are present.
     *
     * References:
     *
     * The default non-peephole non-CIFG implementation is based on:
     * http://www.bioinf.jku.at/publications/older/2604.pdf
     * S. Hochreiter and J. Schmidhuber. "Long Short-Term Memory". Neural
     * Computation, 9(8):1735-1780, 1997.
     *
     * The peephole implementation and projection layer is based on:
     * https://research.google.com/pubs/archive/43905.pdf
     * Hasim Sak, Andrew Senior, and Francoise Beaufays. "Long short-term memory
     * recurrent neural network architectures for large scale acoustic
     * modeling." INTERSPEECH, 2014.
     * (However, the concept of peephole optimization was introduced in work
     * prior to this paper.)
     *
     * The coupling of input and forget gate (CIFG) is based on:
     * http://arxiv.org/pdf/1503.04069.pdf
     * Greff et al. "LSTM: A Search Space Odyssey"
     *
     * The layer normalization is based on:
     * https://arxiv.org/pdf/1607.06450.pdf
     * Jimmy Ba et al. "Layer Normalization"
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * All input and output tensors must be of the same type.
     *
     * Inputs:
     * * 0: The input (\f$x_t\f$).
     *      A 2-D tensor of shape [batch_size, input_size], where “batch_size”
     *      corresponds to the batching dimension, and “input_size” is the size
     *      of the input.
     * * 1: The input-to-input weights (\f$W_{xi}\f$). Optional.
     *      A 2-D tensor of shape [num_units, input_size], where “num_units”
     *      corresponds to the number of cell units.
     * * 2: The input-to-forget weights (\f$W_{xf}\f$).
     *      A 2-D tensor of shape [num_units, input_size].
     * * 3: The input-to-cell weights (\f$W_{xc}\f$).
     *      A 2-D tensor of shape [num_units, input_size].
     * * 4: The input-to-output weights (\f$W_{xo}\f$).
     *      A 2-D tensor of shape [num_units, input_size].
     * * 5: The recurrent-to-input weights (\f$W_{hi}\f$). Optional.
     *      A 2-D tensor of shape [num_units, output_size], where “output_size”
     *      corresponds to either the number of cell units (i.e., “num_units”),
     *      or the second dimension of the “projection_weights”, if defined.
     * * 6: The recurrent-to-forget weights (\f$W_{hf}\f$).
     *      A 2-D tensor of shape [num_units, output_size].
     * * 7: The recurrent-to-cell weights (\f$W_{hc}\f$).
     *      A 2-D tensor of shape [num_units, output_size].
     * * 8: The recurrent-to-output weights (\f$W_{ho}\f$).
     *      A 2-D tensor of shape [num_units, output_size].
     * * 9: The cell-to-input weights (\f$W_{ci}\f$). Optional.
     *      A 1-D tensor of shape [num_units].
     * * 10:The cell-to-forget weights (\f$W_{cf}\f$). Optional.
     *      A 1-D tensor of shape [num_units].
     * * 11:The cell-to-output weights (\f$W_{co}\f$). Optional.
     *      A 1-D tensor of shape [num_units].
     * * 12:The input gate bias (\f$b_i\f$). Optional.
     *      A 1-D tensor of shape [num_units].
     * * 13:The forget gate bias (\f$b_f\f$).
     *      A 1-D tensor of shape [num_units].
     * * 14:The cell bias (\f$b_c\f$).
     *      A 1-D tensor of shape [num_units].
     * * 15:The output gate bias (\f$b_o\f$).
     *      A 1-D tensor of shape [num_units].
     * * 16:The projection weights (\f$W_{proj}\f$). Optional.
     *      A 2-D tensor of shape [output_size, num_units].
     * * 17:The projection bias (\f$b_{proj}\f$). Optional.
     *      A 1-D tensor of shape [output_size].
     * * 18:The output state (in) (\f$h_{t-1}\f$).
     *      A 2-D tensor of shape [batch_size, output_size].
     * * 19:The cell state (in) (\f$C_{t-1}\f$).
     *      A 2-D tensor of shape [batch_size, num_units].
     * * 20:The activation function (\f$g\f$).
     *      A value indicating the activation function:
     *      <ul>
     *      <li>0: None;
     *      <li>1: Relu;
     *      <li>3: Relu6;
     *      <li>4: Tanh;
     *      <li>6: Sigmoid.
     *      </ul>
     * * 21:The clipping threshold (\f$t_{cell}\f$) for the cell state, such
     *      that values are bound within [-cell_clip, cell_clip]. If set to 0.0
     *      then clipping is disabled.
     *      Until HAL version 1.2 this scalar must be of type {@link
     *      OperandType::FLOAT32}. Since HAL version 1.2, if all the input
     *      tensors have type {@link OperandType::TENSOR_FLOAT32}, this
     *      scalar must be of the type {@link OperandType::FLOAT32},
     *      otherwise if all the input tensors have the type {@link
     *      OperandType::TENSOR_FLOAT16}, this scalar must be of type {@link
     *      OperandType::FLOAT16}.
     * * 22:The clipping threshold (\f$t_{proj}\f$) for the output from the
     *      projection layer, such that values are bound within
     *      [-proj_clip, proj_clip]. If set to 0.0 then clipping is disabled.
     *      Until HAL version 1.2 this scalar must be of type {@link
     *      OperandType::FLOAT32}. Since HAL version 1.2, if all the input
     *      tensors have type {@link OperandType::TENSOR_FLOAT32}, this
     *      scalar must be of the type {@link OperandType::FLOAT32},
     *      otherwise if all the input tensors have the type {@link
     *      OperandType::TENSOR_FLOAT16}, this scalar must be of type {@link
     *      OperandType::FLOAT16}.
     * Since HAL version 1.2 there are additional inputs to this op:
     * * 23:The input layer normalization weights.
     *      A 1-D tensor of shape [num_units]. Used to rescale normalized inputs
     *      to activation at input gate.
     * * 24:The forget layer normalization weights.
     *      A 1-D tensor of shape [num_units]. Used to rescale normalized inputs
     *      to activation at forget gate.
     * * 25:The cell layer normalization weights.
     *      A 1-D tensor of shape [num_units]. Used to rescale normalized inputs
     *      to activation at cell gate.
     * * 26:The output layer normalization weights.
     *      A 1-D tensor of shape [num_units]. Used to rescale normalized inputs
     *      to activation at output gate.
     *
     * Outputs:
     * * 0: The scratch buffer.
     *      A 2-D tensor of shape [batch_size, num_units * 3] with CIFG, or
     *      [batch_size, num_units * 4] without CIFG.
     * * 1: The output state (out) (\f$h_t\f$).
     *      A 2-D tensor of shape [batch_size, output_size].
     * * 2: The cell state (out) (\f$C_t\f$).
     *      A 2-D tensor of shape [batch_size, num_units].
     * * 3: The output (\f$o_t\f$).
     *      A 2-D tensor of shape [batch_size, output_size]. This is effectively
     *      the same as the current “output state (out)” value.
     */
    LSTM = 16,

    /**
     * Performs an 2-D max pooling operation.
     *
     * The output dimensions are functions of the filter dimensions, stride, and
     * padding.
     *
     * The values in the output tensor are computed as:
     *
     *     output[b, i, j, channel] =
     *         max_{di, dj} (
     *             input[b, strides[1] * i + di, strides[2] * j + dj, channel]
     *         )
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: 4, with "NHWC" or "NCHW" data layout.
     * With the default data layout NHWC, the data is stored in the order of:
     * [batch, height, width, channels]. Alternatively, the data layout could
     * be NCHW, the data storage order of: [batch, channels, height, width].
     * NCHW is supported since HAL version 1.2.
     *
     * Both explicit padding and implicit padding are supported.
     *
     * Inputs (explicit padding):
     * * 0: A 4-D tensor, of shape [batches, height, width, depth], specifying
     *      the input.
     *      Since HAL version 1.2, zero batches is supported for this tensor.
     * * 1: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the left, in the ‘width’ dimension.
     * * 2: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the right, in the ‘width’ dimension.
     * * 3: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the top, in the ‘height’ dimension.
     * * 4: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the bottom, in the ‘height’ dimension.
     * * 5: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘width’ dimension.
     * * 6: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘height’ dimension.
     * * 7: An {@link OperandType::INT32} scalar, specifying the filter
     *      width.
     * * 8: An {@link OperandType::INT32} scalar, specifying the filter
     *      height.
     * * 9: An {@link OperandType::INT32} scalar, and has to be one of the
     *      {@link FusedActivationFunc} values. Specifies the activation to
     *      invoke on the result.
     * * 10: An optional {@link OperandType::BOOL} scalar, default to false.
     *       Set to true to specify NCHW data layout for input0 and output0.
     *       Available since HAL version 1.2.
     *
     * Inputs (implicit padding):
     * * 0: A 4-D tensor, of shape [batches, height, width, depth], specifying
     *      the input.
     *      Since HAL version 1.2, zero batches is supported for this tensor.
     * * 1: An {@link OperandType::INT32} scalar, specifying the implicit
     *      padding scheme, has to be one of the
     *      following values: {0 (NONE), 1 (SAME), 2 (VALID)}.
     * * 2: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘width’ dimension.
     * * 3: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘height’ dimension.
     * * 4: An {@link OperandType::INT32} scalar, specifying the filter
     *      width.
     * * 5: An {@link OperandType::INT32} scalar, specifying the filter
     *      height.
     * * 6: An {@link OperandType::INT32} scalar, and has to be one of the
     *      {@link FusedActivationFunc} values. Specifies the activation to
     *      invoke on the result.
     * * 7: An optional {@link OperandType::BOOL} scalar, default to false.
     *      Set to true to specify NCHW data layout for input0 and output0.
     *      Available since HAL version 1.2.
     *
     * Outputs:
     * * 0: The output 4-D tensor, of shape
     *      [batches, out_height, out_width, depth].
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    MAX_POOL_2D = 17,

    /**
     * Multiplies two tensors, element-wise.
     *
     * Takes two input tensors of identical {@link OperandType} and compatible
     * dimensions. The output is the product of both input tensors, optionally
     * modified by an activation function.
     *
     * Two dimensions are compatible when:
     *     1. they are equal, or
     *     2. one of them is 1
     *
     * The size of the resulting output is the maximum size along each dimension
     * of the input operands. It starts with the trailing dimensions, and works
     * its way forward.
     *
     * Since HAL version 1.2, generic zero-sized input tensor is supported. Zero
     * dimension is only compatible with 0 or 1. The size of the output
     * dimension is zero if either of corresponding input dimension is zero.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     * * {@link OperandType::TENSOR_INT32} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4
     *
     * Inputs:
     * * 0: A tensor.
     * * 1: A tensor of the same {@link OperandType}, and compatible dimensions
     *      as input0.
     * * 2: An {@link OperandType::INT32} scalar, and has to be one of the
     *      {@link FusedActivationFunc} values. Specifies the activation to
     *      invoke on the result.
     *      For a {@link OperandType::TENSOR_INT32} tensor,
     *      the {@link FusedActivationFunc} must be "NONE".
     *
     * Outputs:
     * * 0: The product, a tensor of the same {@link OperandType} as input0.
     *      For output tensor of {@link OperandType::TENSOR_QUANT8_ASYMM}
     *      and {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED},
     *      the following condition must be satisfied:
     *      output_scale > input1_scale * input2_scale.
     */
    MUL = 18,

    /**
     * Computes rectified linear activation on the input tensor element-wise.
     *
     * The output is calculated using this formula:
     *
     *     output = max(0, input)
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4.
     *
     * Inputs:
     * * 0: A tensor, specifying the input.
     *      Since HAL version 1.2, this tensor may be zero-sized.
     *
     * Outputs:
     * * 0: The output tensor of same shape as input0.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    RELU = 19,

    /**
     * Computes rectified linear 1 activation on the input tensor element-wise.
     *
     * The output is calculated using this formula:
     *
     *     output = min(1.f, max(-1.f, input))
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4.
     *
     * Inputs:
     * * 0: A tensor, specifying the input.
     *      Since HAL version 1.2, this tensor may be zero-sized.
     *
     * Outputs:
     * * 0: The output tensor of the same shape as input0.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    RELU1 = 20,

    /**
     * Computes rectified linear 6 activation on the input tensor element-wise.
     *
     * The output is calculated using this formula:
     *
     *     output = min(6, max(0, input))
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4.
     *
     * Inputs:
     * * 0: A tensor, specifying the input.
     *      Since HAL version 1.2, this tensor may be zero-sized.
     *
     * Outputs:
     * * 0: The output tensor of same shape as input0.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    RELU6 = 21,

    /**
     * Reshapes a tensor.
     *
     * Given tensor, this operation returns a tensor that has the same values as
     * tensor, but with a newly specified shape.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     * * {@link OperandType::TENSOR_INT32} (since NNAPI feature level 6)
     *
     * Supported tensor rank: up to 4.
     *
     * Inputs:
     * * 0: A tensor, specifying the tensor to be reshaped.
     * * 1: A 1-D tensor of {@link OperandType::TENSOR_INT32}, defining the
     *      shape of the output tensor. The number of elements implied by shape
     *      must be the same as the number of elements in the input tensor.
     *
     *      If one component of shape is the special value -1, the size of that
     *      dimension is computed so that the total size remains constant. In
     *      particular, a shape of [-1] flattens into 1-D. At most one component
     *      of shape can be -1.
     *
     * Outputs:
     * * 0: The output tensor, of shape specified by the input shape.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    RESHAPE = 22,

    /**
     * Resizes images to given size using the bilinear interpretation.
     *
     * Resized images must be distorted if their output aspect ratio is not the
     * same as input aspect ratio. The corner pixels of output may not be the
     * same as corner pixels of input.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: 4, with "NHWC" or "NCHW" data layout.
     * With the default data layout NHWC, the data is stored in the order of:
     * [batch, height, width, channels]. Alternatively, the data layout could
     * be NCHW, the data storage order of: [batch, channels, height, width].
     * NCHW is supported since HAL version 1.2.
     *
     * Both resizing by shape and resizing by scale are supported.
     *
     * Inputs (resizing by shape):
     * * 0: A 4-D tensor, of shape [batches, height, width, depth], specifying
     *      the input.
     *      Since HAL version 1.2, zero batches is supported for this tensor.
     * * 1: An {@link OperandType::INT32} scalar, specifying the output
     *      width of the output tensor.
     * * 2: An {@link OperandType::INT32} scalar, specifying the output
     *      height of the output tensor.
     * * 3: An optional {@link OperandType::BOOL} scalar, default to false.
     *      Set to true to specify NCHW data layout for input0 and output0.
     *      Available since HAL version 1.2.
     * * 4: Align corners. An optional {@link OperandType::BOOL}
     *      scalar, default to false.  If True, the centers of the 4 corner
     *      pixels of the input and output tensors are aligned, preserving the
     *      values at the corner pixels.
     *      Available since HAL version 1.3.
     * * 5: Half pixel centers. An optional {@link OperandType::BOOL}
     *      scalar, default to false. If True, the pixel centers are assumed to
     *      be at (0.5, 0.5). This is the default behavior of image.resize in
     *      TF 2.0. If this parameter is True, then align_corners parameter
     *      must be False.
     *      Available since HAL version 1.3.
     *
     * Inputs (resizing by scale, since HAL version 1.2):
     * * 0: A 4-D tensor, of shape [batches, height, width, depth], specifying
     *      the input. Zero batches is supported for this tensor.
     * * 1: A scalar, specifying width_scale, the scaling factor of the width
     *      dimension from the input tensor to the output tensor. The output
     *      width is calculated as new_width = floor(width * width_scale).
     *      The scalar must be of {@link OperandType::FLOAT16} if input0 is
     *      of {@link OperandType::TENSOR_FLOAT16} and of
     *      {@link OperandType::FLOAT32} otherwise.
     * * 2: A scalar, specifying height_scale, the scaling factor of the height
     *      dimension from the input tensor to the output tensor. The output
     *      height is calculated as new_height = floor(height * height_scale).
     *      The scalar must be of {@link OperandType::FLOAT16} if input0 is
     *      of {@link OperandType::TENSOR_FLOAT16} and of
     *      {@link OperandType::FLOAT32} otherwise.
     * * 3: An optional {@link OperandType::BOOL} scalar, default to false.
     *      Set to true to specify NCHW data layout for input0 and output0.
     * * 4: Align corners. An optional {@link OperandType::BOOL}
     *      scalar, default to false.  If True, the centers of the 4 corner
     *      pixels of the input and output tensors are aligned, preserving the
     *      values at the corner pixels.
     *      Available since HAL version 1.3.
     * * 5: Half pixel centers. An optional {@link OperandType::BOOL}
     *      scalar, default to false. If True, the pixel centers are assumed to
     *      be at (0.5, 0.5). This is the default behavior of image.resize in
     *      TF 2.0. If this parameter is True, then align_corners parameter
     *      must be False.
     *      Available since HAL version 1.3.
     *
     * Outputs:
     * * 0: The output 4-D tensor, of shape
     *      [batches, new_height, new_width, depth].
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    RESIZE_BILINEAR = 23,

    /**
     * A basic recurrent neural network layer.
     *
     * This layer implements the operation:
     * outputs = state = activation(inputs * input_weights +
     *                              state * recurrent_weights + bias)
     *
     * Where:
     * * “input_weights” is a weight matrix that multiplies the inputs;
     * * “recurrent_weights” is a weight matrix that multiplies the current
     *    “state” which itself is the output from the previous time step
     *    computation;
     * * “bias” is a bias vector (added to each output vector in the batch);
     * * “activation” is the function passed as the “fused_activation_function”
     *   argument (if not “NONE”).
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * The input tensors must all be the same type.
     *
     * Inputs:
     * * 0: input.
     *      A 2-D tensor of shape [batch_size, input_size], where “batch_size”
     *      corresponds to the batching dimension, and “input_size” is the size
     *      of the input.
     * * 1: weights.
     *      A 2-D tensor of shape [num_units, input_size], where “num_units”
     *      corresponds to the number of units.
     * * 2: recurrent_weights.
     *      A 2-D tensor of shape [num_units, num_units], with columns
     *      corresponding to the weights from each unit.
     * * 3: bias.
     *      A 1-D tensor of shape [num_units].
     * * 4: hidden state (in).
     *      A 2-D tensor of shape [batch_size, num_units].
     * * 5: fused_activation_function.
     *      An optional {@link FusedActivationFunc} value indicating the
     *      activation function. If “NONE” is specified then it results in a
     *      linear activation.
     *
     * Outputs:
     * * 0: hidden state (out).
     *      A 2-D tensor of shape [batch_size, num_units].
     *
     * * 1: output.
     *      A 2-D tensor of shape [batch_size, num_units]. This is effectively
     *      the same as the current state value.
     */
    RNN = 24,

    /**
     * Computes the softmax activation on the input tensor element-wise, per
     * batch, by normalizing the input vector so the maximum coefficient is
     * zero.
     *
     * The output is calculated using this formula:
     *
     *     output[batch, i] =
     *         exp((input[batch, i] - max(input[batch, :])) * beta) /
     *         sum_{k}{exp((input[batch, k] - max(input[batch, :])) * beta)}
     *
     * For input tensor with rank other than 2, the activation will be applied
     * independently on each 1-D slice along specified dimension.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4.
     * Tensors with rank other than 2 or 4 are only supported since HAL version 1.2.
     *
     * Inputs:
     * * 0: A 2-D or 4-D tensor, specifying the tensor to be reshaped.
     *      Since HAL version 1.2, this tensor may be zero-sized.
     * * 1: A scalar, specifying the positive scaling factor for the exponent,
     *      beta. If input0 is of {@link OperandType::TENSOR_FLOAT32},
     *      {@link OperandType::TENSOR_QUANT8_ASYMM} or
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}, the scalar
     *      must be of {@link OperandType::FLOAT32}.
     *      If input0 is of {@link OperandType::TENSOR_FLOAT16}, then the
     *      scalar must be of {@link OperandType::FLOAT16}.
     * * 2: An optional {@link OperandType::INT32} scalar, default to -1,
     *      specifying the dimension the activation would be performed on.
     *      Negative index is used to specify axis from the end (e.g. -1 for
     *      the last axis). Must be in the range [-n, n).
     *      Available since HAL version 1.2.
     *
     * Outputs:
     * * 0: The output tensor of same shape as input0.
     *      For {@link OperandType::TENSOR_QUANT8_ASYMM},
     *      the scale must be 1.f / 256 and the zeroPoint must be 0.
     *      For {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED},
     *      the scale must be 1.f / 256 and the zeroPoint must be -128.
     */
    SOFTMAX = 25,

    /**
     * Rearranges blocks of spatial data, into depth.
     *
     * More specifically, this op outputs a copy of the input tensor where
     * values from the height and width dimensions are moved to the depth
     * dimension. The value block_size indicates the input block size and how
     * the data is moved.
     *
     * Chunks of data of size block_size * block_size from depth are rearranged
     * into non-overlapping blocks of size block_size x block_size.
     *
     * The depth of the output tensor is input_depth * block_size * block_size.
     * The input tensor's height and width must be divisible by block_size.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: 4, with "NHWC" or "NCHW" data layout.
     * With the default data layout NHWC, the data is stored in the order of:
     * [batch, height, width, channels]. Alternatively, the data layout could
     * be NCHW, the data storage order of: [batch, channels, height, width].
     * NCHW is supported since HAL version 1.2.
     *
     * Inputs:
     * * 0: A 4-D tensor, of shape [batches, height, width, depth_in],
     *      specifying the input.
     * * 1: An {@link OperandType::INT32} scalar, specifying the block_size.
     *      block_size must be >=1 and block_size must be a divisor of both the
     *      input height and width.
     * * 2: An optional {@link OperandType::BOOL} scalar, default to false.
     *      Set to true to specify NCHW data layout for input0 and output0.
     *      Available since HAL version 1.2.
     *
     * Outputs:
     * * 0: The output 4-D tensor, of shape [batches, height/block_size,
     *      width/block_size, depth_in*block_size*block_size].
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    SPACE_TO_DEPTH = 26,

    /**
     * SVDF op is a kind of stateful layer derived from the notion that a
     * densely connected layer that's processing a sequence of input frames can
     * be approximated by using a singular value decomposition of each of its
     * nodes. The implementation is based on:
     *
     * https://research.google.com/pubs/archive/43813.pdf
     *
     * P. Nakkiran, R. Alvarez, R. Prabhavalkar, C. Parada.
     * “Compressing Deep Neural Networks using a Rank-Constrained Topology”.
     * INTERSPEECH, 2015.
     *
     * It processes the incoming input using a 2-stage filtering mechanism:
     * * stage 1 performs filtering on the "features" dimension, whose outputs
     *   get pushed into a memory of fixed-size memory_size.
     * * stage 2 performs filtering on the "time" dimension of the memory_size
     *   memoized outputs of stage 1.
     *
     * Specifically, for rank 1, this layer implements the operation:
     *
     *     memory = push(conv1d(inputs, weights_feature, feature_dim,
     *                          "PADDING_VALID"));
     *     outputs = activation(memory * weights_time + bias);
     *
     * Where:
     * * “weights_feature” is a weights matrix that processes the inputs (by
     *   convolving the input with every “feature filter”), and whose outputs
     *   get pushed, stacked in order, into the fixed-size “memory” (the oldest
     *   entry gets dropped);
     * * “weights_time” is a weights matrix that processes the “memory” (by a
     *   batched matrix multiplication on the num_units);
     * * “bias” is an optional bias vector (added to each output vector in the
     *   batch); and
     * * “activation” is the function passed as the “fused_activation_function”
     *   argument (if not “NONE”).
     *
     * Each rank adds a dimension to the weights matrices by means of stacking
     * the filters.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * All input tensors must be the same type.
     *
     * Inputs:
     * * 0: input.
     *      A 2-D tensor of shape [batch_size, input_size], where “batch_size”
     *      corresponds to the batching dimension, and “input_size” is the size
     *      of the input.
     * * 1: weights_feature.
     *      A 2-D tensor of shape [num_units, input_size], where “num_units”
     *      corresponds to the number of units.
     * * 2: weights_time.
     *      A 2-D tensor of shape [num_units, memory_size], where “memory_size”
     *      corresponds to the fixed-size of the memory.
     * * 3: bias.
     *      An optional 1-D tensor of shape [num_units].
     * * 4: state (in).
     *      A 2-D tensor of shape [batch_size, (memory_size - 1) * num_units * rank].
     * * 5: rank.
     *      The rank of the SVD approximation.
     * * 6: fused_activation_function.
     *      An optional {@link FusedActivationFunc} value indicating the
     *      activation function. If “NONE” is specified then it results in a
     *      linear activation.
     *
     * Outputs:
     * * 0: state (out).
     *      A 2-D tensor of the same {@link OperandType} as the inputs, with shape
     *      [batch_size, (memory_size - 1) * num_units * rank].
     * * 1: output.
     *      A 2-D tensor of the same {@link OperandType} as the inputs, with shape
     *      [batch_size, num_units].
     */
    SVDF = 27,

    /**
     * Computes hyperbolic tangent of input tensor element-wise.
     *
     * The output is calculated using this formula:
     *
     *     output = tanh(input)
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4.
     *
     * Inputs:
     * * 0: A tensor, specifying the input.
     *      Since HAL version 1.2, this tensor may be zero-sized.
     *
     * Outputs:
     * * 0: The output tensor of same shape as input0.
     *      For {@link OperandType::TENSOR_QUANT8_ASYMM},
     *      the scale must be 1.f / 128 and the zeroPoint must be 128.
     *      For {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED},
     *      the scale must be 1.f / 128 and the zeroPoint must be 0.
     */
    TANH = 28,

    /**
     * BatchToSpace for N-dimensional tensors.
     *
     * This operation reshapes the batch dimension (dimension 0) into M + 1
     * dimensions of shape block_shape + [batch], interleaves these blocks back
     * into the grid defined by the spatial dimensions [1, ..., M], to obtain a
     * result with the same rank as the input.
     *
     * This is the reverse of SpaceToBatch.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: 4, with "NHWC" or "NCHW" data layout.
     * With the default data layout NHWC, the data is stored in the order of:
     * [batch, height, width, channels]. Alternatively, the data layout could
     * be NCHW, the data storage order of: [batch, channels, height, width].
     * NCHW is supported since HAL version 1.2.
     *
     * Inputs:
     * * 0: An n-D tensor, specifying the tensor to be reshaped
     * * 1: A 1-D Tensor of {@link OperandType::TENSOR_INT32}, the block
     *      sizes for each spatial dimension of the input tensor. All values
     *      must be >= 1.
     * * 2: An optional {@link OperandType::BOOL} scalar, default to false.
     *      Set to true to specify NCHW data layout for input0 and output0.
     *      Available since API level 29.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    BATCH_TO_SPACE_ND = 29,

    /**
     * Element-wise division of two tensors.
     *
     * Takes two input tensors of identical {@link OperandType} and compatible
     * dimensions. The output is the result of dividing the first input tensor
     * by the second, optionally modified by an activation function.
     *
     * For inputs of {@link OperandType::TENSOR_INT32}, performs
     * "floor division" ("//" in Python). For example,
     *     5 // 2 = 2
     *    -5 // 2 = -3
     *
     * Two dimensions are compatible when:
     *     1. they are equal, or
     *     2. one of them is 1
     *
     * The size of the output is the maximum size along each dimension of the
     * input operands. It starts with the trailing dimensions, and works its way
     * forward.
     *
     * Example:
     *     input1.dimension =    {4, 1, 2}
     *     input2.dimension = {5, 4, 3, 1}
     *     output.dimension = {5, 4, 3, 2}
     *
     * Since HAL version 1.2, generic zero-sized input tensor is supported. Zero
     * dimension is only compatible with 0 or 1. The size of the output
     * dimension is zero if either of corresponding input dimension is zero.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4
     *
     * Inputs:
     * * 0: An n-D tensor, specifying the first input.
     * * 1: A tensor of the same {@link OperandType}, and compatible dimensions
     *      as input0.
     * * 2: An {@link OperandType::INT32} scalar, and has to be one of the
     *      {@link FusedActivationFunc} values. Specifies the activation to
     *      invoke on the result.
     *      For a {@link OperandType::TENSOR_INT32} tensor,
     *      the {@link FusedActivationFunc} must be "NONE".
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0.
     */
    DIV = 30,

    /**
     * Computes the mean of elements across dimensions of a tensor.
     *
     * Reduces the input tensor along the given dimensions to reduce. Unless
     * keep_dims is true, the rank of the tensor is reduced by 1 for each entry
     * in axis. If keep_dims is true, the reduced dimensions are retained with
     * length 1.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4
     *
     * Inputs:
     * * 0: A tensor, specifying the input.
     * * 1: A 1-D Tensor of {@link OperandType::TENSOR_INT32}. The dimensions
     *      to reduce. Must be in the range
     *      [-rank(input_tensor), rank(input_tensor)).
     *
     *      NOTE: When the operation was introduced, the documentation
     *      incorrectly stated that if dimensions were empty, the operation
     *      would reduce across all dimensions. This behavior was never
     *      implemented.
     *
     * * 2: An {@link OperandType::INT32} scalar, keep_dims. If positive,
     *      retains reduced dimensions with length 1.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     *      If all dimensions are reduced and keep_dims is false, the output
     *      shape is [1].
     */
    MEAN = 31,

    /**
     * Pads a tensor.
     *
     * This operation pads a tensor according to the specified paddings.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *   (full support since HAL version 1.2, see the output section)
     *
     * Supported tensor rank: up to 4
     *
     * Inputs:
     * * 0: An n-D tensor, specifying the tensor to be padded.
     * * 1: A 2-D Tensor of {@link OperandType::TENSOR_INT32}, the paddings
     *      for each spatial dimension of the input tensor. The shape of the
     *      tensor must be {rank(input0), 2}.
     *      padding[i, 0] specifies the number of elements to be padded in the
     *      front of dimension i.
     *      padding[i, 1] specifies the number of elements to be padded after the
     *      end of dimension i.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0. The
     *      output tensor has the same rank as input0, and each
     *      dimension of the output tensor has the same size as the
     *      corresponding dimension of the input tensor plus the size
     *      of the padding:
     *          output0.dimension[i] =
     *              padding[i, 0] + input0.dimension[i] + padding[i, 1]
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     *
     *      NOTE: Before HAL version 1.2, the pad value for
     *      {@link OperandType::TENSOR_QUANT8_ASYMM} is undefined.
     *      Since HAL version 1.2, the pad value is always the logical zero.
     */
    PAD = 32,

    /**
     * SpaceToBatch for N-Dimensional tensors.
     *
     * This operation divides "spatial" dimensions [1, ..., M] of the input into
     * a grid of blocks of shape block_shape, and interleaves these blocks with
     * the "batch" dimension (0) such that in the output, the spatial dimensions
     * [1, ..., M] correspond to the position within the grid, and the batch
     * dimension combines both the position within a spatial block and the
     * original batch position. Prior to division into blocks, the spatial
     * dimensions of the input are optionally zero padded according to paddings.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *   (full support since HAL version 1.2, see the output section)
     *
     * Supported tensor rank: 4, with "NHWC" or "NCHW" data layout.
     * With the default data layout NHWC, the data is stored in the order of:
     * [batch, height, width, channels]. Alternatively, the data layout could
     * be NCHW, the data storage order of: [batch, channels, height, width].
     * NCHW is supported since HAL version 1.2.
     *
     * Inputs:
     * * 0: An n-D tensor, specifying the input.
     * * 1: A 1-D Tensor of {@link OperandType::TENSOR_INT32}, the block
     *      sizes for each spatial dimension of the input tensor. All values
     *      must be >= 1.
     * * 2: A 2-D Tensor of {@link OperandType::TENSOR_INT32}, the paddings
     *      for each spatial dimension of the input tensor. All values must be
     *      >= 0. The shape of the tensor must be {M, 2}, where M is the number
     *      of spatial dimensions.
     *      padding[i, 0] specifies the number of element to be padded in the
     *      front of dimension i.
     *      padding[i, 1] specifies the number of element to be padded after the
     *      end of dimension i.
     * * 3: An optional {@link OperandType::BOOL} scalar, default to false.
     *      Set to true to specify NCHW data layout for input0 and output0.
     *      Available since HAL version 1.2.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     *
     *      NOTE: Before HAL version 1.2, the pad value for
     *      {@link OperandType::TENSOR_QUANT8_ASYMM} is undefined.
     *      Since HAL version 1.2, the pad value is always the logical zero.
     */
    SPACE_TO_BATCH_ND = 33,

    /**
     * Removes dimensions of size 1 from the shape of a tensor.
     *
     * Given a tensor input, this operation returns a tensor of the same
     * {@link OperandType} with all dimensions of size 1 removed. If you don't
     * want to remove all size 1 dimensions, you can remove specific size 1
     * dimensions by specifying the axes (input1).
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4
     *
     * Inputs:
     * * 0: An n-D tensor, the tensor to be squeezed.
     * * 1: An optional 1-D tensor of {@link OperandType::TENSOR_INT32}. The
     *      dimensions to squeeze. If specified only squeezes the dimensions
     *      listed. Otherwise, squeezes all dimensions. The dimension index
     *      starts at 0. An error must be reported if squeezing a dimension that
     *      is not 1.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0. Contains the
     *      same data as input, but has one or more dimensions of size 1
     *      removed.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     *      If all input dimensions are equal to 1 and are to be squeezed, the
     *      output shape is [1].
     */
    SQUEEZE = 34,

    /**
     * Extracts a strided slice of a tensor.
     *
     * Roughly speaking, this op extracts a slice of size (end - begin) / stride
     * from the given input tensor. Starting at the location specified by begin
     * the slice continues by adding stride to the index until all dimensions
     * are not less than end. Note that a stride can be negative, which causes a
     * reverse slice.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4
     *
     * Inputs:
     * * 0: An n-D tensor, specifying the tensor to be sliced.
     * * 1: begin, a 1-D tensor of {@link OperandType::TENSOR_INT32}. The
     *      starts of the dimensions of the input tensor to be sliced. The
     *      length must be of rank(input0).
     * * 2: end, a 1-D tensor of {@link OperandType::TENSOR_INT32}. The
     *      ends of the dimensions of the input tensor to be sliced. The length
     *      must be of rank(input0).
     * * 3: strides, a 1-D tensor of {@link OperandType::TENSOR_INT32}. The
     *      strides of the dimensions of the input tensor to be sliced. The
     *      length must be of rank(input0). The entries must be non-zero.
     * * 4: begin_mask, an {@link OperandType::INT32} scalar. If the ith bit
     *      of begin_mask is set, begin[i] is ignored and the fullest possible
     *      range in that dimension is used instead.
     * * 5: end_mask, an {@link OperandType::INT32} scalar. If the ith bit of
     *      end_mask is set, end[i] is ignored and the fullest possible range in
     *      that dimension is used instead.
     * * 6: shrink_axis_mask, an {@link OperandType::INT32} scalar. If the
     *      ith bit of shrink_axis_mask is set, the ith dimension specification
     *      shrinks the dimensionality by 1, taking on the value at index
     *      begin[i]. In this case, the ith specification must define a
     *      slice of size 1, e.g. begin[i] = x, end[i] = x + 1.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0 and rank (n - k),
     *      where k is the number of bits set in shrink_axis_mask.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     *      If shrink_axis_mask is true for all input dimensions, the output
     *      shape is [1].
     */
    STRIDED_SLICE = 35,

    /**
     * Element-wise subtraction of two tensors.
     *
     * Takes two input tensors of identical {@link OperandType} and compatible
     * dimensions. The output is the result of subtracting the second input
     * tensor from the first one, optionally modified by an activation function.
     *
     * Two dimensions are compatible when:
     *     1. they are equal, or
     *     2. one of them is 1
     *
     * The size of the output is the maximum size along each dimension of the
     * input operands. It starts with the trailing dimensions, and works its way
     * forward.
     *
     * Example:
     *     input1.dimension =    {4, 1, 2}
     *     input2.dimension = {5, 4, 3, 1}
     *     output.dimension = {5, 4, 3, 2}
     *
     * Since HAL version 1.2, generic zero-sized input tensor is supported. Zero
     * dimension is only compatible with 0 or 1. The size of the output
     * dimension is zero if either of corresponding input dimension is zero.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     * * {@link OperandType::TENSOR_INT32} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4
     *
     * Inputs:
     * * 0: An n-D tensor, specifying the first input.
     * * 1: A tensor of the same {@link OperandType}, and compatible dimensions
     *      as input0.
     * * 2: An {@link OperandType::INT32} scalar, and has to be one of the
     *      {@link FusedActivationFunc} values. Specifies the activation to
     *      invoke on the result.
     *      For a {@link OperandType::TENSOR_INT32} tensor,
     *      the {@link FusedActivationFunc} must be "NONE".
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint can be different from inputs' scale and zeroPoint.
     */
    SUB = 36,

    /**
     * Transposes the input tensor, permuting the dimensions according to the
     * perm tensor.
     *
     * The returned tensor's dimension i corresponds to the input dimension
     * perm[i]. If perm is not given, it is set to (n-1...0), where n is the
     * rank of the input tensor. Hence by default, this operation performs a
     * regular matrix transpose on 2-D input Tensors.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16} (since HAL version 1.2)
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4
     *
     * Inputs:
     * * 0: An n-D tensor, specifying the tensor to be transposed.
     *      Since HAL version 1.2, this tensor may be zero-sized.
     * * 1: An optional 1-D Tensor of {@link OperandType::TENSOR_INT32},
     *      the permutation of the dimensions of the input tensor.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    TRANSPOSE = 37,

    /**
     * Computes the absolute value of a tensor, element-wise.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32} (since HAL version 1.3)
     *
     * Supported tensor rank: from 1.
     *
     * Inputs:
     * * 0: A tensor.
     *
     * Outputs:
     * * 0: The output tensor of same shape as input0.
     */
    ABS = 38,

    /**
     * Returns the index of the largest element along an axis.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: from 1
     *
     * Inputs:
     * * 0: An n-D tensor specifying the input. Must be non-empty.
     * * 1: An {@link OperandType::INT32} scalar specifying the axis to
     *      reduce across. Negative index is used to specify axis from the
     *      end (e.g. -1 for the last axis). Must be in the range [-n, n).
     *
     * Outputs:
     * * 0: An (n - 1)-D {@link OperandType::TENSOR_INT32} tensor.
     *      If input is 1-dimensional, the output shape is [1].
     */
    // There is no underscore in ARG_MAX to avoid name conflict with
    // the macro defined in libc/kernel/uapi/linux/limits.h.
    ARGMAX = 39,

    /**
     * Returns the index of the smallest element along an axis.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: from 1
     *
     * Inputs:
     * * 0: An n-D tensor specifying the input. Must be non-empty.
     * * 1: An {@link OperandType::INT32} scalar specifying the axis to
     *      reduce across. Negative index is used to specify axis from the
     *      end (e.g. -1 for the last axis). Must be in the range [-n, n).
     *
     * Outputs:
     * * 0: An (n - 1)-D {@link OperandType::TENSOR_INT32} tensor.
     *      If input is 1-dimensional, the output shape is [1].
     */
    ARGMIN = 40,  // See ARGMAX for naming discussion.

    /**
     * Transform axis-aligned bounding box proposals using bounding box deltas.
     *
     * Given the positions of bounding box proposals and the corresponding
     * bounding box deltas for each class, return the refined bounding box
     * regions. The resulting bounding boxes are cliped against the edges of
     * the image.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT16_ASYMM}
     *
     * Inputs:
     * * 0: A 2-D Tensor of shape [num_rois, 4], specifying the locations of the
     *      bounding box proposals, each line with format [x1, y1, x2, y2].
     *      For tensor of type {@link OperandType::TENSOR_QUANT16_ASYMM},
     *      the zeroPoint must be 0 and the scale must be 0.125. Zero num_rois
     *      is supported for this tensor.
     * * 1: A 2-D Tensor of shape [num_rois, num_classes * 4], specifying the
     *      bounding box delta for each region of interest and each class. The
     *      bounding box deltas are organized in the following order
     *      [dx, dy, dw, dh], where dx and dy is the relative correction factor
     *      for the center position of the bounding box with respect to the width
     *      and height, dw and dh is the log-scale relative correction factor
     *      for the width and height. For input0 of type
     *      {@link OperandType::TENSOR_QUANT16_ASYMM}, this tensor should be
     *      of {@link OperandType::TENSOR_QUANT8_ASYMM} or
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}. Zero num_rois is
     *      supported for this tensor.
     * * 2: An 1-D {@link OperandType::TENSOR_INT32} tensor, of shape
     *      [num_rois], specifying the batch index of each box. Boxes with
     *      the same batch index are grouped together. Zero num_rois is
     *      supported for this tensor.
     * * 3: A 2-D Tensor of shape [batches, 2], specifying the information of
     *      each image in the batch, each line with format
     *      [image_height, image_width].
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0, with shape
     *      [num_rois, num_classes * 4], specifying the coordinates of each
     *      output bounding box for each class, with format [x1, y1, x2, y2].
     *      For type of {@link OperandType::TENSOR_QUANT16_ASYMM}, the
     *      scale must be 0.125 and the zero point must be 0.
     */
    AXIS_ALIGNED_BBOX_TRANSFORM = 41,

    /**
     * A recurrent neural network layer that applies an LSTM cell to a
     * sequence of inputs in forward and backward directions.
     *
     * The op supports cross-linking via an auxiliary input. Regular cell feeds
     * one input into the two RNN cells in the following way:
     *
     *       INPUT  (INPUT_REVERSED)
     *         |         |
     *    ---------------------
     *    | FW_LSTM   BW_LSTM |
     *    ---------------------
     *         |         |
     *      FW_OUT     BW_OUT
     *
     * An op with cross-linking takes two inputs and feeds them into the RNN
     * cells in the following way:
     *
     *       AUX_INPUT   (AUX_INPUT_REVERSED)
     *           |             |
     *     INPUT | (INPUT_R'D.)|
     *       |   |       |     |
     *    -----------------------
     *    |  \  /        \    / |
     *    | FW_LSTM     BW_LSTM |
     *    -----------------------
     *         |           |
     *      FW_OUT      BW_OUT
     *
     * The cross-linking mode is enabled iff auxiliary input and auxiliary
     * weights are present. While stacking this op on top of itself, this
     * allows to connect both forward and backward outputs from previous cell
     * to the next cell's input.
     *
     * Since HAL version 1.3 parallel linking mode is supported. The mode is
     * enabled if auxiliary input is present but auxiliary weights are omitted.
     * In this case, the cell feeds inputs into the RNN in the following way:
     *
     *       INPUT (AUX_INPUT_REVERSED)
     *         |         |
     *    ---------------------
     *    | FW_LSTM   BW_LSTM |
     *    ---------------------
     *         |         |
     *      FW_OUT     BW_OUT
     *
     * While stacking this op on top of itself, this allows to connect both
     * forward and backward outputs from previous cell to the next cell's
     * corresponding inputs.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * Supported tensor rank: 3, either time-major or batch-major.
     *
     * All input and output tensors must be of the same type.
     *
     * Inputs:
     * * 0: The input.
     *      A 3-D tensor of shape:
     *        If time-major: [max_time, batch_size, input_size]
     *        If batch-major: [batch_size, max_time, input_size]
     *      where "max_time" is the number of timesteps (sequence length),
     *      "batch_size" corresponds to the batching dimension, and
     *      "input_size" is the size of the input.
     * * 1: The forward input-to-input weights. Optional.
     *      A 2-D tensor of shape [fw_num_units, input_size], where “fw_num_units”
     *      corresponds to the number of forward cell units.
     * * 2: The forward input-to-forget weights.
     *      A 2-D tensor of shape [fw_num_units, input_size].
     * * 3: The forward input-to-cell weights.
     *      A 2-D tensor of shape [fw_num_units, input_size].
     * * 4: The forward input-to-output weights.
     *      A 2-D tensor of shape [fw_num_units, input_size].
     * * 5: The forward recurrent-to-input weights. Optional.
     *      A 2-D tensor of shape [fw_num_units, fw_output_size], where “fw_output_size”
     *      corresponds to either the number of cell units (i.e., fw_num_units),
     *      or the second dimension of the “fw_projection_weights”, if defined.
     * * 6: The forward recurrent-to-forget weights.
     *      A 2-D tensor of shape [fw_num_units, fw_output_size].
     * * 7: The forward recurrent-to-cell weights.
     *      A 2-D tensor of shape [fw_num_units, fw_output_size].
     * * 8: The forward recurrent-to-output weights.
     *      A 2-D tensor of shape [fw_num_units, fw_output_size].
     * * 9: The forward cell-to-input weights. Optional.
     *      A 1-D tensor of shape [fw_num_units].
     * * 10: The forward cell-to-forget weights. Optional.
     *       A 1-D tensor of shape [fw_num_units].
     * * 11: The forward cell-to-output weights. Optional.
     *       A 1-D tensor of shape [fw_num_units].
     * * 12: The forward input gate bias. Optional.
     *       A 1-D tensor of shape [fw_num_units].
     * * 13: The forward forget gate bias.
     *       A 1-D tensor of shape [fw_num_units].
     * * 14: The forward cell gate bias.
     *       A 1-D tensor of shape [fw_num_units].
     * * 15: The forward output gate bias.
     *       A 1-D tensor of shape [fw_num_units].
     * * 16: The forward projection weights. Optional.
     *       A 2-D tensor of shape [fw_output_size, fw_num_units].
     * * 17: The forward projection bias. Optional.
     *       A 1-D tensor of shape [fw_output_size].
     * * 18: The backward input-to-input weights. Optional.
     *       A 2-D tensor of shape [bw_num_units, input_size], where “bw_num_units”
     *       corresponds to the number of backward cell units.
     * * 19: The backward input-to-forget weights.
     *       A 2-D tensor of shape [bw_num_units, input_size].
     * * 20: The backward input-to-cell weights.
     *       A 2-D tensor of shape [bw_num_units, input_size].
     * * 21: The backward input-to-output weights.
     *       A 2-D tensor of shape [bw_num_units, input_size].
     * * 22: The backward recurrent-to-input weights. Optional.
     *       A 2-D tensor of shape [bw_num_units, bw_output_size], where “bw_output_size”
     *       corresponds to either the number of cell units (i.e., “bw_num_units”),
     *       or the second dimension of the “bw_projection_weights”, if defined.
     * * 23: The backward recurrent-to-forget weights.
     *       A 2-D tensor of shape [bw_num_units, bw_output_size].
     * * 24: The backward recurrent-to-cell weights.
     *       A 2-D tensor of shape [bw_num_units, bw_output_size].
     * * 25: The backward recurrent-to-output weights.
     *       A 2-D tensor of shape [bw_num_units, bw_output_size].
     * * 26: The backward cell-to-input weights. Optional.
     *       A 1-D tensor of shape [bw_num_units].
     * * 27: The backward cell-to-forget weights. Optional.
     *       A 1-D tensor of shape [bw_num_units].
     * * 28: The backward cell-to-output weights. Optional.
     *       A 1-D tensor of shape [bw_num_units].
     * * 29: The backward input gate bias. Optional.
     *       A 1-D tensor of shape [bw_num_units].
     * * 30: The backward forget gate bias.
     *       A 1-D tensor of shape [bw_num_units].
     * * 31: The backward cell gate bias.
     *       A 1-D tensor of shape [bw_num_units].
     * * 32: The backward output gate bias.
     *       A 1-D tensor of shape [bw_num_units].
     * * 33: The backward projection weights. Optional.
     *       A 2-D tensor of shape [bw_output_size, bw_num_units].
     * * 34: The backward projection bias. Optional.
     *       A 1-D tensor of shape [bw_output_size].
     * * 35: The forward input activation state.
     *       A 2-D tensor of shape [batch_size, bw_output_size].
     * * 36: The forward input cell state.
     *       A 2-D tensor of shape [batch_size, bw_num_units].
     * * 37: The backward input activation state.
     *       A 2-D tensor of shape [batch_size, bw_output_size].
     * * 38: The backward input cell state.
     *       A 2-D tensor of shape [batch_size, bw_num_units].
     * * 39: The auxiliary input. Optional.
     *       A 3-D tensor of shape [max_time, batch_size, aux_input_size],
     *       where “batch_size” corresponds to the batching dimension, and
     *       “aux_input_size” is the size of the auxiliary input. Optional. See
     *       the docs above for the usage modes explanation.
     * * 40: The forward auxiliary input-to-input weights.
     *       Optional. See the docs above for the usage modes explanation.
     *       A 2-D tensor of shape [fw_num_units, aux_input_size].
     * * 41: The forward auxiliary input-to-forget weights.
     *       Optional. See the docs above for the usage modes explanation.
     *       A 2-D tensor of shape [fw_num_units, aux_input_size].
     * * 42: The forward auxiliary input-to-cell weights.
     *       Optional. See the docs above for the usage modes explanation.
     *       A 2-D tensor of shape [fw_num_units, aux_input_size].
     * * 43: The forward auxiliary input-to-output weights.
     *       Optional. See the docs above for the usage modes explanation.
     *       A 2-D tensor of shape [fw_num_units, aux_input_size].
     * * 44: The backward auxiliary input-to-input weights.
     *       Optional. See the docs above for the usage modes explanation.
     *       A 2-D tensor of shape [bw_num_units, aux_input_size].
     * * 45: The backward auxiliary input-to-forget weights.
     *       Optional. See the docs above for the usage modes explanation.
     *       A 2-D tensor of shape [bw_num_units, aux_input_size].
     * * 46: The backward auxiliary input-to-cell weights.
     *       Optional. See the docs above for the usage modes explanation.
     *       A 2-D tensor of shape [bw_num_units, aux_input_size].
     * * 47: The backward auxiliary input-to-output weights.
     *       Optional. See the docs above for the usage modes explanation.
     *       A 2-D tensor of shape [bw_num_units, aux_input_size].
     * * 48: The activation function.
     *       A value indicating the activation function:
     *       <ul>
     *       <li>0: None;
     *       <li>1: Relu;
     *       <li>3: Relu6;
     *       <li>4: Tanh;
     *       <li>6: Sigmoid.
     *       </ul>
     * * 49: The clipping threshold for the cell state, such
     *       that values are bound within [-cell_clip, cell_clip]. If set to 0.0
     *       then clipping is disabled.
     *       If all the input tensors have type {@link OperandType::TENSOR_FLOAT32},
     *       this scalar must be of the type {@link OperandType::FLOAT32},
     *       otherwise if all the input tensors have the type
     *       {@link OperandType::TENSOR_FLOAT16}, this scalar must be
     *       of type {@link OperandType::FLOAT16}.
     * * 50: The clipping threshold for the output from the
     *       projection layer, such that values are bound within
     *       [-proj_clip, proj_clip]. If set to 0.0 then clipping is disabled.
     *       If all the input tensors have type {@link OperandType::TENSOR_FLOAT32},
     *       this scalar must be of the type {@link OperandType::FLOAT32},
     *       otherwise if all the input tensors have the type
     *       {@link OperandType::TENSOR_FLOAT16}, this scalar must be
     *       of type {@link OperandType::FLOAT16}.
     * * 51: merge_outputs
     *       An {@link OperandType::BOOL} scalar specifying if the outputs
     *       from forward and backward cells should be merged.
     * * 52: time_major
     *       An {@link OperandType::BOOL} scalar specifying the shape format
     *       of input and output tensors.
     * * 53: The forward input layer normalization weights. Optional.
     *       A 1-D tensor of shape [fw_num_units]. Used to rescale normalized inputs
     *       to activation at input gate.
     * * 54: The forward forget layer normalization weights. Optional.
     *       A 1-D tensor of shape [fw_num_units]. Used to rescale normalized inputs
     *       to activation at forget gate.
     * * 55: The forward cell layer normalization weights. Optional.
     *       A 1-D tensor of shape [fw_num_units]. Used to rescale normalized inputs
     *       to activation at cell gate.
     * * 56: The forward output layer normalization weights. Optional.
     *       A 1-D tensor of shape [fw_num_units]. Used to rescale normalized inputs
     *       to activation at output gate.
     * * 57: The backward input layer normalization weights. Optional.
     *       A 1-D tensor of shape [bw_num_units]. Used to rescale normalized inputs
     *       to activation at input gate.
     * * 58: The backward forget layer normalization weights. Optional.
     *       A 1-D tensor of shape [bw_num_units]. Used to rescale normalized inputs
     *       to activation at forget gate.
     * * 59: The backward cell layer normalization weights. Optional.
     *       A 1-D tensor of shape [bw_num_units]. Used to rescale normalized inputs
     *       to activation at cell gate.
     * * 60: The backward output layer normalization weights. Optional.
     *       A 1-D tensor of shape [bw_num_units]. Used to rescale normalized inputs
     *       to activation at output gate.
     *
     * Outputs:
     * * 0: The forward output.
     *      A 3-D tensor of shape:
     *        If time-major and not merge_outputs:
     *          [max_time, batch_size, fw_output_size]
     *        If time-major and merge_outputs:
     *          [max_time, batch_size, fw_output_size + bw_output_size]
     *        If batch-major and not merge_outputs:
     *          [batch_size, max_time, fw_output_size]
     *        If batch-major and merge_outputs:
     *          [batch_size, max_time, fw_output_size + bw_output_size]
     * * 1: The backward output.  Unused if merge_outputs is true.
     *      A 3-D tensor of shape:
     *        If time-major: [max_time, batch_size, bw_output_size]
     *        If batch-major: [batch_size, max_time, bw_output_size]
     * * 2: The forward activation state output.
     *      A 2-D tensor of shape [batch_size, fw_output_size] containing an
     *      activation state from the last time step in the sequence. This
     *      output is optional and can be omitted. If this output is present
     *      then outputs 3-5 must be present as well.
     *      Available since HAL version 1.3.
     * * 3: The forward cell state output.
     *      A tensor of shape [batch_size, fw_cell_size] containing a cell state
     *      from the last time step in the sequence. This output is optional
     *      and can be omitted. If this output is present
     *      then outputs 2, 4, 5 must be present as well.
     *      Available since HAL version 1.3.
     * * 4: The backward activation state output.
     *      A 2-D tensor of shape [batch_size, bw_output_size] containing an
     *      activation state from the last time step in the sequence. This
     *      output is optional and can be omitted. If this output is present
     *      then outputs 2, 3, 5 must be present as well.
     *      Available since HAL version 1.3.
     * * 5: The backward cell state output.
     *      A tensor of shape [batch_size, bw_cell_size] containing a cell state
     *      from the last time step in the sequence. This output is optional
     *      and can be omitted. If this output is present
     *      then outputs 2-4 must be present as well.
     *      Available since HAL version 1.3.
     */
    BIDIRECTIONAL_SEQUENCE_LSTM = 42,

    /**
     * A recurrent neural network layer that applies a basic RNN cell to a
     * sequence of inputs in forward and backward directions.
     *
     * This Op unrolls the input along the sequence dimension, and implements
     * the following operation for each element in the sequence s =
     * 1...sequence_length:
     *   fw_outputs[s] = fw_state = activation(inputs[s] * fw_input_weights’ +
     *          fw_state * fw_recurrent_weights’ + fw_bias)
     *
     * And for each element in sequence t = sequence_length : 1
     *   bw_outputs[t] = bw_state = activation(inputs[t] * bw_input_weights’ +
     *          bw_state * bw_recurrent_weights’ + bw_bias)
     *
     * Where:
     * * “{fw,bw}_input_weights” is a weight matrix that multiplies the inputs;
     * * “{fw,bw}_recurrent_weights” is a weight matrix that multiplies the
     *    current “state” which itself is the output from the previous time step
     *    computation;
     * * “{fw,bw}_bias” is a bias vector (added to each output vector in the
     *    batch);
     * * “activation” is the function passed as the “fused_activation_function”
     *   argument (if not “NONE”).
     *
     * The op supports cross-linking via an auxiliary input. Regular cell feeds
     * one input into the two RNN cells in the following way:
     *
     *       INPUT  (INPUT_REVERSED)
     *         |         |
     *    ---------------------
     *    | FW_RNN     BW_RNN |
     *    ---------------------
     *         |         |
     *      FW_OUT     BW_OUT
     *
     * An op with cross-linking takes two inputs and feeds them into the RNN
     * cells in the following way:
     *
     *       AUX_INPUT   (AUX_INPUT_REVERSED)
     *           |             |
     *     INPUT | (INPUT_R'D.)|
     *       |   |       |     |
     *    -----------------------
     *    |  \  /        \    / |
     *    | FW_RNN       BW_RNN |
     *    -----------------------
     *         |           |
     *      FW_OUT      BW_OUT
     *
     * The cross-linking mode is enabled iff auxiliary input and auxiliary
     * weights are present. While stacking this op on top of itself, this
     * allows to connect both forward and backward outputs from previous cell
     * to the next cell's input.
     *
     * Since HAL version 1.3 parallel linking mode is supported. The mode is
     * enabled if auxiliary input is present but auxiliary weights are omitted.
     * In this case, the cell feeds inputs into the RNN in the following way:
     *
     *       INPUT (AUX_INPUT_REVERSED)
     *         |         |
     *    ---------------------
     *    | FW_RNN     BW_RNN |
     *    ---------------------
     *         |         |
     *      FW_OUT     BW_OUT
     *
     * While stacking this op on top of itself, this allows to connect both
     * forward and backward outputs from previous cell to the next cell's
     * corresponding inputs.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * The input tensors must all be the same type.
     *
     * Inputs:
     * * 0: input.
     *      A 3-D tensor. The shape is defined by the input 6 (timeMajor). If
     *      it is set to true, then the input has a shape [maxTime, batchSize,
     *      inputSize], otherwise the input has a shape [batchSize, maxTime,
     *      inputSize].
     * * 1: fwWeights.
     *      A 2-D tensor of shape [fwNumUnits, inputSize].
     * * 2: fwRecurrentWeights.
     *      A 2-D tensor of shape [fwNumUnits, fwNumUnits].
     * * 3: fwBias.
     *      A 1-D tensor of shape [fwNumUnits].
     * * 4: fwHiddenState.
     *      A 2-D tensor of shape [batchSize, fwNumUnits]. Specifies a hidden
     *      state input for the first time step of the computation.
     * * 5: bwWeights.
     *      A 2-D tensor of shape [bwNumUnits, inputSize].
     * * 6: bwRecurrentWeights.
     *      A 2-D tensor of shape [bwNumUnits, bwNumUnits].
     * * 7: bwBias.
     *      A 1-D tensor of shape [bwNumUnits].
     * * 8: bwHiddenState
     *      A 2-D tensor of shape [batchSize, bwNumUnits]. Specifies a hidden
     *      state input for the first time step of the computation.
     * * 9: auxInput.
     *      A 3-D tensor. The shape is defined by the input 6 (timeMajor). If
     *      it is set to true, then the input has a shape [maxTime, batchSize,
     *      auxInputSize], otherwise the input has a shape [batchSize, maxTime,
     *      auxInputSize]. Can be omitted. See the docs above for the usage
     *      modes explanation.
     * * 10:fwAuxWeights.
     *      A 2-D tensor of shape [fwNumUnits, auxInputSize]. Can be omitted.
     *      See the docs above for the usage modes explanation.
     * * 11:bwAuxWeights.
     *      A 2-D tensor of shape [bwNumUnits, auxInputSize]. Can be omitted.
     *      See the docs above for the usage modes explanation.
     * * 12:fusedActivationFunction.
     *      A {@link FusedActivationFunc} value indicating the activation function. If
     *      “NONE” is specified then it results in a linear activation.
     * * 13:timeMajor
     *      An {@link OperandType::BOOL} scalar specifying the shape format
     *      of input and output tensors.
     * * 14:mergeOutputs
     *      An {@link OperandType::BOOL} scalar specifying if the outputs
     *      from forward and backward cells are separate (if set to false) or
     *      concatenated (if set to true).
     * Outputs:
     * * 0: fwOutput.
     *      A 3-D tensor. The first two dimensions of the shape are defined by
     *      the input 6 (timeMajor) and the third dimension is defined by the
     *      input 14 (mergeOutputs). If timeMajor is set to true, then the first
     *      two dimensions are [maxTime, batchSize], otherwise they are set to
     *      [batchSize, maxTime]. If mergeOutputs is set to true, then the third
     *      dimension is equal to (fwNumUnits + bwNumUnits), otherwise it is set
     *      to fwNumUnits.
     * * 1: bwOutput.
     *      A 3-D tensor. If the input 14 (mergeOutputs) is set to true, then
     *      this tensor is not produced. The shape is defined by the input 6
     *      (timeMajor). If it is set to true, then the shape is set to
     *      [maxTime, batchSize, bwNumUnits], otherwise the shape is set to
     *      [batchSize, maxTime, bwNumUnits].
     * * 2: The forward hidden state output.
     *      A 2-D tensor of shape [batchSize, fwNumUnits] containing a hidden
     *      state from the last time step in the sequence. This output is
     *      optional and can be omitted. If this output is present then output
     *      3 must be present as well.
     *      Available since HAL version 1.3.
     * * 3: The backward hidden state output.
     *      A 2-D tensor of shape [batchSize, bwNumUnits] containing a hidden
     *      state from the last time step in the sequence. This output is
     *      optional and can be omitted. If this output is present then output
     *      2 must be present as well.
     *      Available since HAL version 1.3.
     */
    BIDIRECTIONAL_SEQUENCE_RNN = 43,

    /**
     * Greedily selects a subset of bounding boxes in descending order of score.
     *
     * This op applies NMS algorithm to each class. In each loop of execution,
     * the box with maximum score gets selected and removed from the pending set.
     * The scores of the rest of boxes are lowered according to the
     * intersection-over-union (IOU) overlapping with the previously selected
     * boxes and a specified NMS kernel method. Any boxes with score less
     * than a threshold are removed from the pending set.
     *
     * Three NMS kernels are supported:
     * * Hard:     score_new = score_old * (1 if IoU < threshold else 0)
     * * Linear:   score_new = score_old * (1 if IoU < threshold else 1 - IoU)
     * * Gaussian: score_new = score_old * exp(- IoU^2 / sigma)
     *
     * Axis-aligned bounding boxes are represented by its upper-left corner
     * coordinate (x1,y1) and lower-right corner coordinate (x2,y2). A valid
     * bounding box should satisfy x1 <= x2 and y1 <= y2.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Inputs:
     * * 0: A 2-D Tensor of shape [num_rois, num_classes], specifying the score
     *      of each bounding box proposal. The boxes are grouped by batches in the
     *      first dimension. Zero num_rois is supported for this tensor.
     * * 1: A 2-D Tensor specifying the bounding boxes of shape
     *      [num_rois, num_classes * 4], organized in the order [x1, y1, x2, y2].
     *      The boxes are grouped by batches in the first dimension. The sequential
     *      order of the boxes corresponds with input0. For input0 of type
     *      {@link OperandType::TENSOR_QUANT8_ASYMM}, this tensor should be of
     *      {@link OperandType::TENSOR_QUANT16_ASYMM}, with zeroPoint of 0 and
     *      scale of 0.125.
     *      For input0 of type {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED},
     *      this tensor should be of {@link OperandType::TENSOR_QUANT16_ASYMM},
     *      with zeroPoint of -128 and scale of 0.125.
     *      Zero num_rois is supported for this tensor.
     * * 2: A 1-D {@link OperandType::TENSOR_INT32} tensor, of shape
     *      [num_rois], specifying the batch index of each box. Boxes with
     *      the same batch index are grouped together.
     * * 3: An {@link OperandType::FLOAT32} scalar, score_threshold. Boxes
     *      with scores lower than the threshold are filtered before sending
     *      to the NMS algorithm.
     * * 4: An {@link OperandType::INT32} scalar, specifying the maximum
     *      number of selected bounding boxes for each image. Set to a negative
     *      value for unlimited number of output bounding boxes.
     * * 5: An {@link OperandType::INT32} scalar, specifying the NMS
     *      kernel method, options are 0:hard, 1:linear, 2:gaussian.
     * * 6: An {@link OperandType::FLOAT32} scalar, specifying the IoU
     *      threshold in hard and linear NMS kernel. This field is ignored if
     *      gaussian kernel is selected.
     * * 7: An {@link OperandType::FLOAT32} scalar, specifying the sigma in
     *      gaussian NMS kernel. This field is ignored if gaussian kernel is
     *      not selected.
     * * 8: An {@link OperandType::FLOAT32} scalar, nms_score_threshold.
     *      Boxes with scores lower than the threshold are dropped during the
     *      score updating phase in soft NMS.
     *
     * Outputs:
     * * 0: A 1-D Tensor of the same {@link OperandType} as input0, with shape
     *      [num_output_rois], specifying the score of each output box. The boxes
     *      are grouped by batches, but the sequential order in each batch is not
     *      guaranteed. For type of {@link OperandType::TENSOR_QUANT8_ASYMM},
     *      guaranteed. For type of {@link OperandType::TENSOR_QUANT8_ASYMM}
     *      or {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED},
     *      the scale and zero point must be the same as input0.
     * * 1: A 2-D Tensor of the same {@link OperandType} as input1, with shape
     *      [num_output_rois, 4], specifying the coordinates of each
     *      output bounding box with the same format as input1. The sequential
     *      order of the boxes corresponds with output0. For type of
     *      {@link OperandType::TENSOR_QUANT16_ASYMM}, the scale must be
     *      0.125 and the zero point must be 0.
     * * 2: A 1-D {@link OperandType::TENSOR_INT32} tensor, of shape
     *      [num_output_rois], specifying the class of each output box. The
     *      sequential order of the boxes corresponds with output0.
     * * 3: A 1-D {@link OperandType::TENSOR_INT32} tensor, of shape
     *      [num_output_rois], specifying the batch index of each box. Boxes
     *      with the same batch index are grouped together.
     */
    BOX_WITH_NMS_LIMIT = 44,

    /**
     * Casts a tensor to a type.
     *
     * This operation ignores the scale and zeroPoint of quanized tensors,
     * e.g. it treats a {@link OperandType::TENSOR_QUANT8_ASYMM} input
     * as a tensor of uint8 values.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * Since HAL version 1.3, casting tensors of the following
     * {@link OperandType} to the same {@link OperandType} is supported:
     * * {@link OperandType::TENSOR_BOOL8}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT16_ASYMM}
     * * {@link OperandType::TENSOR_QUANT16_SYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}
     * * {@link OperandType::TENSOR_QUANT8_SYMM}
     *
     * Supported tensor rank: from 1
     *
     * Inputs:
     * * 0: A tensor.
     *
     * Outputs:
     * * 0: A tensor with the same shape as input0.
     */
    CAST = 45,

    /**
     * Shuffle the channels of the input tensor.
     *
     * Given an input tensor and a integer value of num_groups, CHANNEL_SHUFFLE
     * divide the channel dimension into num_groups groups, and reorganize the
     * channels by grouping channels with the same index in each group.
     *
     * Along the channel dimension, the output is calculated using this formula:
     *
     *     output_channel[k * num_groups + g] = input_channel[g * group_size + k]
     *
     * where group_size = num_channels / num_groups
     *
     * The number of channels must be divisible by num_groups.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4
     *
     * Inputs:
     * * 0: An n-D tensor, specifying the tensor to be shuffled.
     * * 1: An {@link OperandType::INT32} scalar, specifying the number of
     *      groups.
     * * 2: An {@link OperandType::INT32} scalar, specifying the dimension
     *      channel shuffle would be performed on. Negative index is used to
     *      specify axis from the end (e.g. -1 for the last axis). Must be in
     *      the range [-n, n).
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} and same shape as input0.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    CHANNEL_SHUFFLE = 46,

    /**
     * Apply postprocessing steps to bounding box detections.
     *
     * Bounding box detections are generated by applying transformation on a set
     * of predefined anchors with the bounding box deltas from bounding box
     * regression. A final step of hard NMS is applied to limit the number of
     * returned boxes.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * Inputs:
     * * 0: A 3-D Tensor of shape [batches, num_anchors, num_classes], specifying
     *      the score of each anchor with each class. Class 0 for each
     *      [batches, num_anchors, 0] is background and will be ignored.
     * * 1: A 3-D Tensor of shape [batches, num_anchors, length_box_encoding], with
     *      the first four values in length_box_encoding specifying the bounding
     *      box deltas. The box deltas are encoded in the order of [dy, dx, dh, dw],
     *      where dy and dx is the linear-scale relative correction factor for the
     *      center position of the bounding box with respect to the width and height,
     *      dh and dw is the log-scale relative correction factor for the width and
     *      height. All the entries in length_box_encoding beyond the first four
     *      values are ignored in this operation.
     * * 2: A 2-D Tensor of shape [num_anchors, 4], specifying the shape of each
     *      predefined anchor, with format [ctr_y, ctr_x, h, w], where ctr_y and
     *      ctr_x are the center position of the box, and h and w are the height
     *      and the width.
     * * 3: An {@link OperandType::FLOAT32} scalar, specifying the scaling
     *      factor for dy in bounding box deltas.
     * * 4: An {@link OperandType::FLOAT32} scalar, specifying the scaling
     *      factor for dx in bounding box deltas.
     * * 5: An {@link OperandType::FLOAT32} scalar, specifying the scaling
     *      factor for dh in bounding box deltas.
     * * 6: An {@link OperandType::FLOAT32} scalar, specifying the scaling
     *      factor for dw in bounding box deltas.
     * * 7: An {@link OperandType::BOOL} scalar, set to true to use regular
     *      multi-class NMS algorithm that do NMS separately for each class,
     *      set to false for a faster algorithm that only do one single NMS
     *      using the highest class score..
     * * 8: An {@link OperandType::INT32} scalar, max_num_detections, specifying
     *      the maximum number of boxes for the output. Boxes with the lowest
     *      scores are discarded to meet the limit.
     * * 9: An {@link OperandType::INT32} scalar, only used when input7 is
     *      set to false, specifying the maximum number of classes per detection.
     * * 10: An {@link OperandType::INT32} scalar, only used when input7 is
     *       set to true, specifying the maximum number of detections when
     *       applying NMS algorithm for each single class.
     * * 11: A scalar, score_threshold. Boxes with scores lower than the
     *       threshold are filtered before sending to the NMS algorithm. The
     *       scalar must be of {@link OperandType::FLOAT16} if input0 is of
     *       {@link OperandType::TENSOR_FLOAT16} and of
     *       {@link OperandType::FLOAT32} if input0 is of
     *       {@link OperandType::TENSOR_FLOAT32}.
     * * 12: A scalar, specifying the IoU threshold for hard NMS. The scalar
     *       must be of {@link OperandType::FLOAT16} if input0 is of
     *       {@link OperandType::TENSOR_FLOAT16} and of
     *       {@link OperandType::FLOAT32} if input0 is of
     *       {@link OperandType::TENSOR_FLOAT32}.
     * * 13: An {@link OperandType::BOOL} scalar, set to true to include
     *       background class in the list of label map for the output, set
     *       to false to not include the background. When the background
     *       class is included, it has label 0 and the output classes start
     *       at 1 in the label map, otherwise, the output classes start at 0.
     *
     * Outputs:
     * * 0: A 2-D tensor of the same {@link OperandType} as input0, with shape
     *      [batches, max_num_detections], specifying the score of each output
     *      detections.
     * * 1: A 3-D tensor of shape [batches, max_num_detections, 4], specifying the
     *      coordinates of each output bounding box, with format
     *      [y1, x1, y2, x2].
     * * 2: A 2-D {@link OperandType::TENSOR_INT32} tensor, of shape
     *      [batches, max_num_detections], specifying the class label for each
     *      output detection.
     * * 3: An 1-D {@link OperandType::TENSOR_INT32} tensor, of shape [batches],
     *      specifying the number of valid output detections for each batch.
     */
    DETECTION_POSTPROCESSING = 47,

    /**
     * For input tensors x and y, computes x == y elementwise.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_BOOL8}
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: from 1
     *
     * This operation supports broadcasting.
     *
     * Inputs:
     * * 0: A tensor.
     * * 1: A tensor of the same {@link OperandType} and dimensions compatible
     *      with input0.
     *
     * Outputs:
     * * 0: A tensor of {@link OperandType::TENSOR_BOOL8}.
     */
    EQUAL = 48,

    /**
     * Computes exponential of x element-wise.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * Supported tensor rank: from 1.
     *
     * Inputs:
     * * 0: A tensor.
     *
     * Outputs:
     * * 0: The output tensor of same shape as input0.
     */
    EXP = 49,

    /**
     * Inserts a dimension of 1 into a tensor's shape.
     *
     * Given a tensor input, this operation inserts a dimension of 1 at the
     * given dimension index of input's shape. The dimension index starts at
     * zero; if you specify a negative dimension index, it is counted backward
     * from the end.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: from 1
     *
     * Inputs:
     * * 0: An n-D tensor.
     * * 1: An {@link OperandType::INT32} scalar specifying the dimension
     *      index to expand. Must be in the range [-(n + 1), (n + 1)).
     *
     * Outputs:
     * * 0: An (n + 1)-D tensor with the same {@link OperandType} and data as
     *      input0.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    EXPAND_DIMS = 50,

    /**
     * Gathers values along an axis.
     *
     * Produces an output tensor with shape
     *     input0.dimension[:axis] + indices.dimension + input0.dimension[axis + 1:]
     * where:
     *     # Vector indices (output is rank(input0)).
     *     output[a_0, ..., a_n, i, b_0, ..., b_n] =
     *       input0[a_0, ..., a_n, indices[i], b_0, ..., b_n]
     *
     *     # Higher rank indices (output is rank(input0) + rank(indices) - 1).
     *     output[a_0, ..., a_n, i, ..., j, b_0, ... b_n] =
     *       input0[a_0, ..., a_n, indices[i, ..., j], b_0, ..., b_n]
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: from 1
     *
     * Inputs:
     * * 0: An n-D tensor from which to gather values.
     * * 1: An {@link OperandType::INT32} scalar specifying the axis.
     *      Negative index is used to specify axis from the end
     *      (e.g. -1 for the last axis). Must be in the range [-n, n).
     * * 2: A k-D tensor {@link OperandType::TENSOR_INT32} of indices.
     *      The values must be in the bounds of the corresponding dimensions
     *      of input0.
     *
     * Outputs:
     * * 0: An (n + k - 1)-D tensor with the same {@link OperandType} as input0.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    GATHER = 51,

    /**
     * Generate aixs-aligned bounding box proposals.
     *
     * Bounding box proposals are generated by applying transformation on a set
     * of predefined anchors with the bounding box deltas from bounding box
     * regression. A final step of hard NMS is applied to limit the number of
     * returned boxes.
     *
     * Axis-aligned bounding boxes are represented by its upper-left corner
     * coordinate (x1,y1) and lower-right corner coordinate (x2,y2). A valid
     * bounding box should satisfy x1 <= x2 and y1 <= y2.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Inputs:
     * * 0: A 4-D Tensor specifying the score of each anchor at each
     *      location. With "NHWC" data layout, the tensor shape is
     *      [batches, height, width, num_anchors]. With "NCHW" data layout,
     *      the tensor shape is [batches, num_anchors, height, width].
     * * 1: A 4-D Tensor specifying the bounding box deltas. With "NHWC" data
     *      layout, the tensor shape is [batches, height, width, num_anchors * 4].
     *      With "NCHW" data layout, the tensor shape is
     *      [batches, num_anchors * 4, height, width]. The box deltas are encoded
     *      in the order of [dx, dy, dw, dh], where dx and dy is the linear-scale
     *      relative correction factor for the center position of the bounding box
     *      with respect to the width and height, dw and dh is the log-scale
     *      relative correction factor for the width and height. The last
     *      dimensions is the channel dimension.
     * * 2: A 2-D Tensor of shape [num_anchors, 4], specifying the shape of each
     *      predefined anchor, with format [x1, y1, x2, y2]. For input0 of type
     *      {@link OperandType::TENSOR_QUANT8_ASYMM} or
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}, this tensor should be of
     *      {@link OperandType::TENSOR_QUANT16_SYMM}, with scale of 0.125.
     * * 3: A 2-D Tensor of shape [batches, 2], specifying the size of
     *      each image in the batch, with format [image_height, image_width].
     *      For input0 of type {@link OperandType::TENSOR_QUANT8_ASYMM} or
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}, this
     *      tensor should be of {@link OperandType::TENSOR_QUANT16_SYMM}, with
     *      scale of 0.125.
     * * 4: An {@link OperandType::FLOAT32} scalar, specifying the ratio
     *      from the height of original image to the height of feature map.
     * * 5: An {@link OperandType::FLOAT32} scalar, specifying the ratio
     *      from the width of original image to the width of feature map.
     * * 6: An {@link OperandType::INT32} scalar, specifying the maximum
     *      number of boxes before going into the hard NMS algorithm. Boxes
     *      with the lowest scores are discarded to meet the limit. Set to
     *      a non-positive value for unlimited number.
     * * 7: An {@link OperandType::INT32} scalar, specifying the maximum
     *      number of boxes returning from the hard NMS algorithm. Boxes
     *      with the lowest scores are discarded to meet the limit. Set to
     *      a non-positive value for unlimited number.
     * * 8: An {@link OperandType::FLOAT32} scalar, specifying the IoU
     *      threshold for hard NMS.
     * * 9: An {@link OperandType::FLOAT32} scalar, min_size. Boxes with
     *      height or width lower than the absolute threshold are filtered out.
     * * 10: An {@link OperandType::BOOL} scalar, set to true to specify
     *       NCHW data layout for input0 and input1. Set to false for NHWC.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0, of shape
     *      [num_output_rois], specifying the score of each output box.
     *      The boxes are grouped by batches, but the sequential order in
     *      each batch is not guaranteed. For type of
     *      {@link OperandType::TENSOR_QUANT8_ASYMM} or
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}, the scale and zero
     *      point must be the same as input0.
     * * 1: A tensor of the same {@link OperandType} as input3, of shape
     *      [num_output_rois, 4], specifying the coordinates of each output
     *      bounding box for each class, with format [x1, y1, x2, y2].
     *      The sequential order of the boxes corresponds with output0.
     *      For type of {@link OperandType::TENSOR_QUANT16_ASYMM}, the
     *      scale must be 0.125 and the zero point must be 0.
     * * 2: A 1-D {@link OperandType::TENSOR_INT32} tensor, of shape
     *      [num_output_rois], specifying the batch index of each box. Boxes
     *      with the same batch index are grouped together.
     */
    GENERATE_PROPOSALS = 52,

    /**
     * For input tensors x and y, computes x > y elementwise.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_BOOL8}
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: from 1
     *
     * This operation supports broadcasting.
     *
     * Inputs:
     * * 0: A tensor.
     * * 1: A tensor of the same {@link OperandType} and dimensions compatible
     *      with input0.
     *
     * Outputs:
     * * 0: A tensor of {@link OperandType::TENSOR_BOOL8}.
     */
    GREATER = 53,
    /**
     * For input tensors x and y, computes x >= y elementwise.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_BOOL8}
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: from 1
     *
     * This operation supports broadcasting.
     *
     * Inputs:
     * * 0: A tensor.
     * * 1: A tensor of the same {@link OperandType} and dimensions compatible
     *      with input0.
     *
     * Outputs:
     * * 0: A tensor of {@link OperandType::TENSOR_BOOL8}.
     */
    GREATER_EQUAL = 54,

    /**
     * Performs a grouped 2-D convolution operation.
     *
     * Given an input tensor of shape [batches, height, width, depth_in] and a
     * filter tensor of shape [depth_out, filter_height, filter_width, depth_group]
     * containing depth_out convolutional filters of depth depth_group, GROUPED_CONV
     * applies a group of different filters to each input channel group, then
     * concatenates the results together.
     *
     * Specifically, the input channels are divided into num_groups groups, each with
     * depth depth_group, i.e. depth_in = num_groups * depth_group. The convolutional
     * filters are also divided into num_groups groups, i.e. depth_out is divisible
     * by num_groups. GROUPED_CONV applies each group of filters to the corresponding
     * input channel group, and the result are concatenated together.
     *
     * The output dimensions are functions of the filter dimensions, stride, and
     * padding.
     *
     * The values in the output tensor are computed as:
     *
     *     output[b, i, j, g * channel_multiplier + q] =
     *         sum_{di, dj, dk} (
     *             input[b, strides[1] * i + di, strides[2] * j + dj,
     *                   g * depth_group + dk] *
     *             filter[g * channel_multiplier + q, di, dj, dk]
     *         ) + bias[channel]
     *
     * where channel_multiplier = depth_out / num_groups
     *
     * Supported tensor {@link OperandType} configurations:
     * * 16 bit floating point:
     * * * {@link OperandType::TENSOR_FLOAT16} for input, filter, output, and bias.
     *
     * * 32 bit floating point:
     * * * {@link OperandType::TENSOR_FLOAT32} for input, filter, output, and bias.
     *
     * * Quantized:
     * * * {@link OperandType::TENSOR_QUANT8_ASYMM} for input, filter, and output.
     * * * {@link OperandType::TENSOR_INT32} for bias (with scale set to
     * * * input.scale * filter.scale).
     *
     * * Quantized signed (since HAL version 1.3):
     * * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} for input, filter, and output.
     * * * {@link OperandType::TENSOR_INT32} for bias (with scale set to
     * * * input.scale * filter.scale).
     *
     * * Quantized with symmetric per channel quantization for the filter:
     * * * {@link OperandType::TENSOR_QUANT8_ASYMM} for input, and output.
     * * * {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL} for filter.
     * * * {@link OperandType::TENSOR_INT32} for bias (scale set to 0.0,
     * * * each value scaling is separate and equal to input.scale * filter.scales[channel]).
     *
     * * Quantized signed with filter symmetric per channel quantization
     *   (since HAL version 1.3):
     * * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} for input, and output.
     * * * {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL} for filter.
     * * * {@link OperandType::TENSOR_INT32} for bias (scale set to 0.0,
     * * * each value scaling is separate and equal to input.scale * filter.scales[channel]).
     *
     * Supported tensor rank: 4, with "NHWC" or "NCHW" data layout.
     * With the default data layout NHWC, the data is stored in the order of:
     * [batch, height, width, channels]. Alternatively, the data layout could
     * be NCHW, the data storage order of: [batch, channels, height, width].
     *
     * Both explicit padding and implicit padding are supported.
     *
     * Inputs (explicit padding):
     * * 0: A 4-D tensor, of shape [batches, height, width, depth_in],
     *      specifying the input, where depth_in = num_groups * depth_group.
     * * 1: A 4-D tensor, of shape
     *      [depth_out, filter_height, filter_width, depth_group], specifying
     *      the filter, where depth_out must be divisible by num_groups.  For
     *      tensor of type {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL}
     *      the channel dimension (channelDim at
     *      {@link SymmPerChannelQuantParams}) must be set to 0.
     * * 2: A 1-D tensor, of shape [depth_out], specifying the bias. For input
     *      tensor of type {@link OperandType::TENSOR_FLOAT32} or
     *      {@link OperandType::TENSOR_FLOAT16}, the bias must be of the same type.
     *      For filter tensor of {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}
     *      the bias should be of {@link OperandType::TENSOR_INT32}, with zeroPoint
     *      of 0 and bias_scale == input_scale * filter_scale. For filter tensor
     *      of {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL}, the bias
     *      should be of {@link OperandType::TENSOR_INT32}, with zeroPoint of
     *      0 and bias_scale of 0. The actual scale of each value 'i' is equal to
     *      bias_scale[i] = input_scale * filter_scale[i].
     * * 3: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the left, in the ‘width’ dimension.
     * * 4: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the right, in the ‘width’ dimension.
     * * 5: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the top, in the ‘height’ dimension.
     * * 6: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the bottom, in the ‘height’ dimension.
     * * 7: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘width’ dimension.
     * * 8: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘height’ dimension.
     * * 9: An {@link OperandType::INT32} scalar, specifying the number of
     *      groups.
     * * 10: An {@link OperandType::INT32} scalar, and has to be one of the
     *       {@link FusedActivationFunc} values. Specifies the activation to
     *       invoke on the result.
     * * 11: An {@link OperandType::BOOL} scalar, set to true to specify
     *       NCHW data layout for input0 and output0. Set to false for NHWC.
     *
     * Inputs (implicit padding):
     * * 0: A 4-D tensor, of shape [batches, height, width, depth_in],
     *      specifying the input, where depth_in = num_groups * depth_group.
     * * 1: A 4-D tensor, of shape
     *      [depth_out, filter_height, filter_width, depth_group], specifying
     *      the filter, where depth_out must be divisible by num_groups.  For
     *      tensor of type {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL}
     *      the channel dimension (SymmPerChannelQuantParams::channelDim)
     *      must be set to 0.
     * * 2: A 1-D tensor, of shape [depth_out], specifying the bias. For input
     *      tensor of type {@link OperandType::TENSOR_FLOAT32} or
     *      {@link OperandType::TENSOR_FLOAT16}, the bias must be of the same
     *      {@link OperandType::TENSOR_FLOAT16}, the bias must be of the same type.
     *      For filter tensor of {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}
     *      the bias should be of {@link OperandType::TENSOR_INT32}, with zeroPoint
     *      of 0 and bias_scale == input_scale * filter_scale. For filter tensor
     *      of {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL}, the bias
     *      should be of {@link OperandType::TENSOR_INT32}, with zeroPoint of
     *      0 and bias_scale of 0. The actual scale of each value 'i' is equal to
     *      bias_scale[i] = input_scale * filter_scale[i].
     * * 3: An {@link OperandType::INT32} scalar, specifying the implicit
     *      padding scheme, has to be one of the
     *      following values: {0 (NONE), 1 (SAME), 2 (VALID)}.
     * * 4: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘width’ dimension.
     * * 5: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘height’ dimension.
     * * 6: An {@link OperandType::INT32} scalar, specifying the number of
     *      groups.
     * * 7: An {@link OperandType::INT32} scalar, and has to be one of the
     *      {@link FusedActivationFunc} values. Specifies the activation to
     *      invoke on the result.
     * * 8: An {@link OperandType::BOOL} scalar, set to true to specify
     *      NCHW data layout for input0 and output0. Set to false for NHWC.
     *
     * Outputs:
     * * 0: The output 4-D tensor, of shape
     *      [batches, out_height, out_width, depth_out].
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint can be different from inputs' scale and zeroPoint.
     */
    GROUPED_CONV_2D = 55,

    /**
     * Localize the maximum keypoints from heatmaps.
     *
     * This operation approximates the accurate maximum keypoint scores and
     * indices after bicubic upscaling by using Taylor expansion up to the
     * quadratic term.
     *
     * The bounding box is represented by its upper-left corner coordinate
     * (x1,y1) and lower-right corner coordinate (x2,y2) in the original image.
     * A valid bounding box should satisfy x1 <= x2 and y1 <= y2.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: 4, with "NHWC" or "NCHW" data layout.
     * With the default data layout NHWC, the data is stored in the order of:
     * [batch, height, width, channels]. Alternatively, the data layout could
     * be NCHW, the data storage order of: [batch, channels, height, width].
     *
     * Inputs:
     * * 0: A 4-D Tensor of shape
     *      [num_boxes, heatmap_size, heatmap_size, num_keypoints],
     *      specifying the heatmaps, the height and width of heatmaps should
     *      be the same, and must be greater than or equal to 2.
     * * 1: A 2-D Tensor of shape [num_boxes, 4], specifying the bounding boxes,
     *      each with format [x1, y1, x2, y2]. For input0 of type
     *      {@link OperandType::TENSOR_QUANT8_ASYMM}, this tensor should
     *      be of {@link OperandType::TENSOR_QUANT16_ASYMM}, with zeroPoint
     *      of 0 and scale of 0.125.
     *      For input0 of type
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}, this tensor
     *      should be of {@link OperandType::TENSOR_QUANT16_ASYMM}, with
     *      zeroPoint of -128 and scale of 0.125.
     * * 2: An {@link OperandType::BOOL} scalar, set to true to specify
     *      NCHW data layout for input0. Set to false for NHWC.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0, with shape
     *      [num_boxes, num_keypoints], specifying score of the keypoints.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} or
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint can be different from input0 scale and zeroPoint.
     * * 1: A tensor of the same {@link OperandType} as input1, with shape
     *      [num_boxes, num_keypoints, 2], specifying the location of
     *      the keypoints, the second dimension is organized as
     *      [keypoint_x, keypoint_y].
     *      For type of {@link OperandType::TENSOR_QUANT16_ASYMM}, the
     *      scale must be 0.125 and the zero point must be 0.
     */
    HEATMAP_MAX_KEYPOINT = 56,

    /**
     * Applies instance normalization to the input tensor.
     *
     * The values in the output tensor are computed as:
     *
     *     output[b, h, w, c] =
     *         (input[b, h, w, c] - mean[b, c]) * gamma /
     *         sqrt(var[b, c] + epsilon) + beta
     *
     * Where the mean and variance are computed across the spatial dimensions:
     *
     *     mean[b, c] =
     *         sum_{h, w}(input[b, h, w, c]) / sum(1)
     *
     *     var[b, c] =
     *         sum_{h, w}(pow(input[b, h, w, c] - mean[b, c], 2)) / sum(1)
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * Supported tensor rank: 4, with "NHWC" or "NCHW" data layout.
     * With the default data layout NHWC, the data is stored in the order of:
     * [batch, height, width, channels]. Alternatively, the data layout could
     * be NCHW, the data storage order of: [batch, channels, height, width].
     *
     * Inputs:
     * * 0: An n-D tensor, specifying the tensor to be normalized.
     * * 1: A scalar, specifying gamma, the scale applied to the normalized
     *      tensor. The scalar must be of {@link OperandType::FLOAT16} if
     *      input0 is of {@link OperandType::TENSOR_FLOAT16} and of
     *      {@link OperandType::FLOAT32} if input0 is of
     *      {@link OperandType::TENSOR_FLOAT32}.
     * * 2: A scalar, specifying beta, the offset applied to the normalized
     *      tensor. The scalar must be of {@link OperandType::FLOAT16} if
     *      input0 is of {@link OperandType::TENSOR_FLOAT16} and of
     *      {@link OperandType::FLOAT32} if input0 is of
     *      {@link OperandType::TENSOR_FLOAT32}.
     * * 3: A scalar, specifying epsilon, the small value added to variance to
     *      avoid dividing by zero. The scalar must be of {@link OperandType::FLOAT16} if
     *      input0 is of {@link OperandType::TENSOR_FLOAT16} and of
     *      {@link OperandType::FLOAT32} if input0 is of
     *      {@link OperandType::TENSOR_FLOAT32}.
     * * 4: An {@link OperandType::BOOL} scalar, set to true to specify
     *      NCHW data layout for input0 and output0. Set to false for NHWC.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} and same shape as input0.
     */
    INSTANCE_NORMALIZATION = 57,

    /**
     * For input tensors x and y, computes x < y elementwise.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_BOOL8}
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: from 1
     *
     * This operation supports broadcasting.
     *
     * Inputs:
     * * 0: A tensor.
     * * 1: A tensor of the same {@link OperandType} and dimensions compatible
     *      with input0.
     *
     * Outputs:
     * * 0: A tensor of {@link OperandType::TENSOR_BOOL8}.
     */
    LESS = 58,

    /**
     * For input tensors x and y, computes x <= y elementwise.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_BOOL8}
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: from 1
     *
     * This operation supports broadcasting.
     *
     * Inputs:
     * * 0: A tensor.
     * * 1: A tensor of the same {@link OperandType} and dimensions compatible
     *      with input0.
     *
     * Outputs:
     * * 0: A tensor of {@link OperandType::TENSOR_BOOL8}.
     */
    LESS_EQUAL = 59,

    /**
     * Computes natural logarithm of x element-wise.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * Supported tensor rank: from 1.
     *
     * Inputs:
     * * 0: A tensor.
     *
     * Outputs:
     * * 0: The output tensor of same shape as input0.
     */
    LOG = 60,

    /**
     * Returns the truth value of x AND y element-wise.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_BOOL8}
     *
     * Supported tensor rank: from 1
     *
     * This operation supports broadcasting.
     *
     * Inputs:
     * * 0: A tensor of {@link OperandType::TENSOR_BOOL8}.
     * * 1: A tensor of {@link OperandType::TENSOR_BOOL8} and dimensions
     *      compatible with input0.
     *
     * Outputs:
     * * 0: A tensor of {@link OperandType::TENSOR_BOOL8}.
     */
    LOGICAL_AND = 61,

    /**
     * Computes the truth value of NOT x element-wise.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_BOOL8}
     *
     * Supported tensor rank: from 1.
     *
     * Inputs:
     * * 0: A tensor.
     *
     * Outputs:
     * * 0: The output tensor of same shape as input0.
     */
    LOGICAL_NOT = 62,

    /**
     * Returns the truth value of x OR y element-wise.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_BOOL8}
     *
     * Supported tensor rank: from 1
     *
     * This operation supports broadcasting.
     *
     * Inputs:
     * * 0: A tensor of {@link OperandType::TENSOR_BOOL8}.
     * * 1: A tensor of {@link OperandType::TENSOR_BOOL8} and dimensions
     *      compatible with input0.
     *
     * Outputs:
     * * 0: A tensor of {@link OperandType::TENSOR_BOOL8}.
     */
    LOGICAL_OR = 63,

    /**
     * Computes the log softmax activations given logits.
     *
     * The output is calculated using this formula:
     *
     *     output = logits * beta - log(reduce_sum(exp(logits * beta), axis))
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * Supported tensor rank: from 1.
     *
     * Inputs:
     * * 0: A tensor specifying the input logits.
     * * 1: A scalar, specifying the positive scaling factor for the exponent,
     *      beta.
     *      For input tensor of {@link OperandType::TENSOR_FLOAT16}, the beta
     *      value must be of {@link OperandType::FLOAT16}.
     *      For input tensor of {@link OperandType::TENSOR_FLOAT32}, the beta
     *      value must be of {@link OperandType::FLOAT32}.
     * * 2: An {@link OperandType::INT32} scalar specifying the axis to
     *      reduce across. Negative index is used to specify axis from the
     *      end (e.g. -1 for the last axis). Must be in the range [-n, n).
     *
     * Outputs:
     * * 0: The output tensor of the same {@link OperandType} and shape as
     *      input0.
     */
    LOG_SOFTMAX = 64,

    /**
     * Returns the element-wise maximum of two tensors.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: from 1.
     *
     * Inputs:
     * * 0: A tensor.
     * * 1: A tensor of the same {@link OperandType} and compatible dimensions
     *      with input0.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} tensor,
     *      the scales and zeroPoint can be different from input0 scale and zeroPoint.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM} tensor,
     *      the scale and zeroPoint can be different from inputs' scale and zeroPoint.
     */
    MAXIMUM = 65,

    /**
     * Returns the element-wise minimum of two tensors.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: from 1.
     *
     * Inputs:
     * * 0: A tensor.
     * * 1: A tensor of the same {@link OperandType} and compatible dimensions
     *      with input0.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} tensor,
     *      the scales and zeroPoint can be different from input0 scale and zeroPoint.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM} tensor,
     *      the scale and zeroPoint can be different from inputs' scale and zeroPoint.
     */
    MINIMUM = 66,

    /**
     * Computes numerical negative value element-wise.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     *
     * Supported tensor rank: from 1.
     *
     * Inputs:
     * * 0: A tensor.
     *
     * Outputs:
     * * 0: The output tensor of same shape as input0.
     */
    NEG = 67,

    /**
     * For input tensors x and y, computes x != y elementwise.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_BOOL8}
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: from 1
     *
     * This operation supports broadcasting.
     *
     * Inputs:
     * * 0: A tensor.
     * * 1: A tensor of the same {@link OperandType} and dimensions compatible
     *      with input0.
     *
     * Outputs:
     * * 0: A tensor of {@link OperandType::TENSOR_BOOL8}.
     */
    NOT_EQUAL = 68,

    /**
     * Pads a tensor with the given constant value according to the specified
     * paddings.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4
     *
     * Inputs:
     * * 0: An n-D tensor, specifying the tensor to be padded.
     * * 1: A 2-D Tensor of {@link OperandType::TENSOR_INT32}, the paddings
     *      for each spatial dimension of the input tensor. The shape of the
     *      tensor must be {rank(input0), 2}.
     *      padding[i, 0] specifies the number of elements to be padded in the
     *      front of dimension i.
     *      padding[i, 1] specifies the number of elements to be padded after
     *      the end of dimension i.
     * * 2: A scalar specifying the value to use for padding input0.
     *      For input tensor of {@link OperandType::TENSOR_FLOAT16}, the
     *      pad value must be of {@link OperandType::FLOAT16}.
     *      For input tensor of {@link OperandType::TENSOR_FLOAT32}, the
     *      pad value must be of {@link OperandType::FLOAT32}.
     *      For input tensor of {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED},
     *      the pad value must be of {@link OperandType::INT32}. The
     *      scale and zeroPoint are assumed to be the same as in input0.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0. The
     *      output tensor has the same rank as input0, and each
     *      dimension of the output tensor has the same size as the
     *      corresponding dimension of the input tensor plus the size
     *      of the padding:
     *          output0.dimension[i] =
     *              padding[i, 0] + input0.dimension[i] + padding[i, 1]
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    PAD_V2 = 69,

    /**
     * Computes the power of one value to another.
     *
     * Given a tensor base and a tensor exponent, this operation computes
     * base^exponent elementwise.
     *
     * This operations supports broadcasting. The size of the output is the
     * maximum size along each dimension of the input operands. It starts with
     * the trailing dimensions, and works its way forward.
     *
     * For example:
     *     base.dimension     =    {4, 1, 2}
     *     exponent.dimension = {5, 4, 3, 1}
     *     output.dimension   = {5, 4, 3, 2}
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * Supported tensor rank: from 1
     *
     * Inputs:
     * * 0: A tensor specifying the base.
     * * 1: A tensor specifying the exponent.
     *
     * Outputs:
     * * 0: An output tensor.
     */
    POW = 70,

    /**
     * Parametric Rectified Linear Unit.
     *
     * It follows: f(x) = alpha * x for x < 0, f(x) = x for x >= 0, where alpha
     * is a learned array with the same {@link OperandType} and compatible
     * dimensions as input x.
     *
     * Two dimensions are compatible when:
     *     1. they are equal, or
     *     2. one of them is 1
     *
     * The size of the output is the maximum size along each dimension of the
     * input operands. It starts with the trailing dimensions, and works its way
     * forward.
     *
     * Example:
     *     input.dimension  =    {4, 1, 2}
     *     alpha.dimension  = {5, 4, 3, 1}
     *     output.dimension = {5, 4, 3, 2}
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: from 1
     *
     * Inputs:
     * * 0: A tensor, specifying the input.
     * * 1: A tensor of the same {@link OperandType}, and compatible dimensions
     *      as input0, specifying the alpha.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scales and zeroPoint can be different from input0 scale and zeroPoint.
     */
    PRELU = 71,

    /**
     * Quantizes the input tensor.
     *
     * The formula for {@link OperandType::TENSOR_QUANT8_ASYMM} output tensor is:
     *
     *     output = max(0, min(255, round(input / scale) + zeroPoint)
     *
     * The formula for {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} output
     * tensor is:
     *
     *     output = max(-128, min(127, round(input / scale) + zeroPoint)
     *
     * Supported input tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * Supported output tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: from 1
     *
     * Inputs:
     * * 0: A tensor, may be zero-sized.
     *
     * Outputs:
     * * 0: The output tensor of same shape as input0, but with
     *      {@link OperandType::TENSOR_QUANT8_ASYMM} or.
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}.
     */
    QUANTIZE = 72,

    /**
     * A version of quantized LSTM, using 16 bit quantization for internal
     * state.
     *
     * There is no projection layer, so cell state size is equal to the output
     * size.
     *
     * Inputs:
     * * 0: A 2-D tensor of type {@link OperandType::TENSOR_QUANT8_ASYMM}
     *      and shape [numBatches, inputSize] specifying the input to the LSTM
     *      cell. Tensor is quantized with a fixed quantization range of
     *      [-1, 127/128] (scale = 1/128, zeroPoint = 128).
     * * 1: The input-to-input weights.
     *      A 2-D tensor of type {@link OperandType::TENSOR_QUANT8_ASYMM}
     *      and shape [outputSize, inputSize] specifying input-to-input part of
     *      weights for fully-connected layer inside the LSTM cell.
     *      Quantization zero point and scale must be the same across all the
     *      weights.
     * * 2: The input-to-forget weights.
     *      A 2-D tensor of type {@link OperandType::TENSOR_QUANT8_ASYMM}
     *      and shape [outputSize, inputSize] specifying input-to-forget part of
     *      weights for fully-connected layer inside the LSTM cell.
     *      Quantization zero point and scale must be the same across all the
     *      weights.
     * * 3: The input-to-cell weights.
     *      A 2-D tensor of type {@link OperandType::TENSOR_QUANT8_ASYMM}
     *      and shape [outputSize, inputSize] specifying input-to-cell part of
     *      weights for fully-connected layer inside the LSTM cell.
     *      Quantization zero point and scale must be the same across all the
     *      weights.
     * * 4: The input-to-output weights.
     *      A 2-D tensor of type {@link OperandType::TENSOR_QUANT8_ASYMM}
     *      and shape [outputSize, inputSize] specifying input-to-output part of
     *      weights for fully-connected layer inside the LSTM cell.
     *      Quantization zero point and scale must be the same across all the
     *      weights.
     * * 5: The recurrent-to-input weights.
     *      A 2-D tensor of type {@link OperandType::TENSOR_QUANT8_ASYMM}
     *      and shape [outputSize, outputSize] specifying recurrent-to-input part
     *      of weights for fully-connected layer inside the LSTM cell.
     *      Quantization zero point and scale must be the same across all the
     *      weights.
     * * 6: The recurrent-to-forget weights.
     *      A 2-D tensor of type {@link OperandType::TENSOR_QUANT8_ASYMM}
     *      and shape [outputSize, outputSize] specifying recurrent-to-forget
     *      part of weights for fully-connected layer inside the LSTM cell.
     *      Quantization zero point and scale must be the same across all the
     *      weights.
     * * 7: The recurrent-to-cell weights.
     *      A 2-D tensor of type {@link OperandType::TENSOR_QUANT8_ASYMM}
     *      and shape [outputSize, outputSize] specifying recurrent-to-cell part
     *      of weights for fully-connected layer inside the LSTM cell.
     *      Quantization zero point and scale must be the same across all the
     *      weights.
     * * 8: The recurrent-to-output weights.
     *      A 2-D tensor of type {@link OperandType::TENSOR_QUANT8_ASYMM}
     *      and shape [outputSize, outputSize] specifying recurrent-to-output
     *      part of weights for fully-connected layer inside the LSTM cell.
     *      Quantization zero point and scale must be the same across all the
     *      weights.
     * * 9: The input gate bias.
     *      A 1-D tensor of type {@link OperandType::TENSOR_INT32} and shape
     *      [outputSize] specifying the bias for the fully-connected layer
     *      inside the LSTM cell. Bias is quantized with scale being a product
     *      of input and weights scales and zeroPoint equal to 0.
     * * 10:The forget gate bias.
     *      A 1-D tensor of type {@link OperandType::TENSOR_INT32} and shape
     *      [outputSize] specifying the bias for the fully-connected layer
     *      inside the LSTM cell. Bias is quantized with scale being a product
     *      of input and weights scales and zeroPoint equal to 0.
     * * 11:The cell bias.
     *      A 1-D tensor of type {@link OperandType::TENSOR_INT32} and shape
     *      [outputSize] specifying the bias for the fully-connected layer
     *      inside the LSTM cell. Bias is quantized with scale being a product
     *      of input and weights scales and zeroPoint equal to 0.
     * * 12:The output gate bias.
     *      A 1-D tensor of type {@link OperandType::TENSOR_INT32} and shape
     *      [outputSize] specifying the bias for the fully-connected layer
     *      inside the LSTM cell. Bias is quantized with scale being a product
     *      of input and weights scales and zeroPoint equal to 0.
     * * 13: A 2-D tensor of type {@link OperandType::TENSOR_QUANT16_SYMM}
     *       and shape [numBatches, outputSize] specifying the cell state from the
     *       previous time step of the LSTM cell. It is quantized using a
     *       quantization range of [-2^4, 2^4 * 32767/32768] (scale = 2^4 /
     *       32768, zeroPoint = 0).
     * * 14: A 2-D tensor of type {@link OperandType::TENSOR_QUANT8_ASYMM}
     *       and shape [numBathes, outputSize] specifying the output of the LSTM
     *       cell from previous time-step. Tensor is quantized with a fixed
     *       quantization range of [-1, 127/128] (scale = 1/128, zeroPoint =
     *       128).
     *
     *
     * Outputs:
     * * 0: A 2-D tensor of type {@link OperandType::TENSOR_QUANT16_SYMM}
     *      and shape [numBatches, outputSize] which contains a cell state from
     *      the current time step. Tensor is quantized using a quantization
     *      range of [-2^4, 2^4 * 32767/32768] (scale = 2^4 / 32768, zeroPoint =
     *      0).
     * * 1: A 2-D tensor of type {@link OperandType::TENSOR_QUANT8_ASYMM}
     *      and shape [numBathes, outputSize] which contains the output value.
     *      Tensor is quantized with a fixed quantization range of [-1, 127/128]
     *      (scale = 1/128, zeroPoint = 128).
     */
    QUANTIZED_16BIT_LSTM = 73,

    /**
     * Draws samples from a multinomial distribution.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * Inputs:
     * * 0: A 2-D tensor with shape [batches, classes], specifying the
     *      unnormalized log-probabilities for all classes.
     * * 1: A scalar {@link OperandType::INT32}, specifying the number of
     *      independent samples to draw for each row slice.
     * * 2: A 1-D {@link OperandType::TENSOR_INT32} tensor with shape [2],
     *      specifying seeds used to initialize the random distribution. If both
     *      provided seeds are 0, both will be randomly generated.
     * Outputs:
     * * 0: A 2-D {@link OperandType::TENSOR_INT32} tensor with shape
     *      [batches, samples], containing the drawn samples.
     */
    RANDOM_MULTINOMIAL = 74,

    /**
     * Reduces a tensor by computing the "logical and" of elements along given
     * dimensions.
     *
     * If keep_dims is true, the reduced dimensions are
     * retained with length 1. Otherwise, the rank of the tensor is reduced by
     * 1 for each entry in dimensions.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_BOOL8}
     *
     * Supported tensor rank: up to 4
     *
     * Inputs:
     * * 0: An n-D tensor.
     * * 1: A 1-D tensor of {@link OperandType::TENSOR_INT32}. The dimensions
     *      to reduce. Dimension values must be in the range [-n, n).
     * * 2: An {@link OperandType::BOOL} scalar, keep_dims. If true,
     *      retains reduced dimensions with length 1.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0.
     *      If all dimensions are reduced and keep_dims is false, the output
     *      shape is [1].
     */
    REDUCE_ALL = 75,

    /**
     * Reduces a tensor by computing the "logical or" of elements along given
     * dimensions.
     *
     * If keep_dims is true, the reduced dimensions are
     * retained with length 1. Otherwise, the rank of the tensor is reduced by
     * 1 for each entry in dimensions.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_BOOL8}
     *
     * Supported tensor rank: up to 4
     *
     * Inputs:
     * * 0: An n-D tensor.
     * * 1: A 1-D tensor of {@link OperandType::TENSOR_INT32}. The dimensions
     *      to reduce. Dimension values must be in the range [-n, n).
     * * 2: An {@link OperandType::BOOL} scalar, keep_dims. If true,
     *      retains reduced dimensions with length 1.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0.
     *      If all dimensions are reduced and keep_dims is false, the output
     *      shape is [1].
     */
    REDUCE_ANY = 76,

    /**
     * Reduces a tensor by computing the maximum of elements along given
     * dimensions.
     *
     * If keep_dims is true, the reduced dimensions are
     * retained with length 1. Otherwise, the rank of the tensor is reduced by
     * 1 for each entry in dimensions.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4
     *
     * Inputs:
     * * 0: An n-D tensor.
     * * 1: A 1-D tensor of {@link OperandType::TENSOR_INT32}. The dimensions
     *      to reduce. Dimension values must be in the range [-n, n).
     * * 2: An {@link OperandType::BOOL} scalar, keep_dims. If true,
     *      retains reduced dimensions with length 1.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0.
     *      If all dimensions are reduced and keep_dims is false, the output
     *      shape is [1].
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    REDUCE_MAX = 77,

    /**
     * Reduces a tensor by computing the minimum of elements along given
     * dimensions.
     *
     * If keep_dims is true, the reduced dimensions are
     * retained with length 1. Otherwise, the rank of the tensor is reduced by
     * 1 for each entry in dimensions.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: up to 4
     *
     * Inputs:
     * * 0: An n-D tensor.
     * * 1: A 1-D tensor of {@link OperandType::TENSOR_INT32}. The dimensions
     *      to reduce. Dimension values must be in the range [-n, n).
     * * 2: An {@link OperandType::BOOL} scalar, keep_dims. If true,
     *      retains reduced dimensions with length 1.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0.
     *      If all dimensions are reduced and keep_dims is false, the output
     *      shape is [1].
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    REDUCE_MIN = 78,

    /**
     * Reduces a tensor by multiplying elements along given dimensions.
     *
     * If keep_dims is true, the reduced dimensions are
     * retained with length 1. Otherwise, the rank of the tensor is reduced by
     * 1 for each entry in dimensions.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * Supported tensor rank: up to 4
     *
     * Inputs:
     * * 0: An n-D tensor.
     * * 1: A 1-D tensor of {@link OperandType::TENSOR_INT32}. The dimensions
     *      to reduce. Dimension values must be in the range [-n, n).
     * * 2: An {@link OperandType::BOOL} scalar, keep_dims. If true,
     *      retains reduced dimensions with length 1.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0.
     *      If all dimensions are reduced and keep_dims is false, the output
     *      shape is [1].
     */
    REDUCE_PROD = 79,

    /**
     * Reduces a tensor by summing elements along given dimensions.
     *
     * If keep_dims is true, the reduced dimensions are
     * retained with length 1. Otherwise, the rank of the tensor is reduced by
     * 1 for each entry in dimensions.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * Supported tensor rank: up to 4
     *
     * Inputs:
     * * 0: An n-D tensor.
     * * 1: A 1-D tensor of {@link OperandType::TENSOR_INT32}. The dimensions
     *      to reduce. Dimension values must be in the range [-n, n).
     * * 2: An {@link OperandType::BOOL} scalar, keep_dims. If true,
     *      retains reduced dimensions with length 1.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0.
     *      If all dimensions are reduced and keep_dims is false, the output
     *      shape is [1].
     */
    REDUCE_SUM = 80,

    /**
     * Select and scale the feature map of each region of interest to a unified
     * output size by average pooling sampling points from bilinear interpolation.
     *
     * The region of interest is represented by its upper-left corner coordinate
     * (x1,y1) and lower-right corner coordinate (x2,y2) in the original image.
     * A spatial scaling factor is applied to map into feature map coordinate.
     * A valid region of interest should satisfy x1 <= x2 and y1 <= y2.
     *
     * No rounding is applied in this operation. The sampling points are unified
     * distributed in the pooling bin and their values are calculated by bilinear
     * interpolation.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: 4, with "NHWC" or "NCHW" data layout.
     * With the default data layout NHWC, the data is stored in the order of:
     * [batch, height, width, channels]. Alternatively, the data layout could
     * be NCHW, the data storage order of: [batch, channels, height, width].
     *
     * Inputs:
     * * 0: A 4-D tensor, specifying the feature map.
     * * 1: A 2-D Tensor of shape [num_rois, 4], specifying the locations of
     *      the regions of interest, each line with format [x1, y1, x2, y2].
     *      For input0 of type {@link OperandType::TENSOR_QUANT8_ASYMM},
     *      this tensor should be of {@link OperandType::TENSOR_QUANT16_ASYMM},
     *      with zeroPoint of 0 and scale of 0.125. Zero num_rois is
     *      supported for this tensor.
     * * 2: An 1-D {@link OperandType::TENSOR_INT32} tensor, of shape
     *      [num_rois], specifying the batch index of each box. Boxes with
     *      the same batch index are grouped together. Zero num_rois is
     *      supported for this tensor.
     * * 3: An {@link OperandType::INT32} scalar, specifying the output
     *      height of the output tensor.
     * * 4: An {@link OperandType::INT32} scalar, specifying the output
     *      width of the output tensor.
     * * 5: An {@link OperandType::FLOAT32} scalar, specifying the ratio
     *      from the height of original image to the height of feature map.
     * * 6: An {@link OperandType::FLOAT32} scalar, specifying the ratio
     *      from the width of original image to the width of feature map.
     * * 7: An {@link OperandType::INT32} scalar, specifying the number of
     *      sampling points in height dimension used to compute the output.
     *      Set to 0 for adaptive value of ceil(roi_height/out_height).
     * * 8: An {@link OperandType::INT32} scalar, specifying the number of
     *      sampling points in width dimension used to compute the output.
     *      Set to 0 for adaptive value of ceil(roi_width/out_width).
     * * 9: An {@link OperandType::BOOL} scalar, set to true to specify
     *      NCHW data layout for input0 and output0. Set to false for NHWC.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0. The output
     *      shape is [num_rois, out_height, out_width, depth].
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint can be different from the input0 scale and zeroPoint.
     */
    ROI_ALIGN = 81,

    /**
     * Select and scale the feature map of each region of interest to a unified
     * output size by max-pooling.
     *
     * The region of interest is represented by its upper-left corner coordinate
     * (x1,y1) and lower-right corner coordinate (x2,y2) in the original image.
     * A spatial scaling factor is applied to map into feature map coordinate.
     * A valid region of interest should satisfy x1 <= x2 and y1 <= y2.
     *
     * Rounding is applied in this operation to ensure integer boundary for
     * regions of interest and pooling bins.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: 4, with "NHWC" or "NCHW" data layout.
     * With the default data layout NHWC, the data is stored in the order of:
     * [batch, height, width, channels]. Alternatively, the data layout could
     * be NCHW, the data storage order of: [batch, channels, height, width].
     *
     * Inputs:
     * * 0: A 4-D tensor, specifying the feature map.
     * * 1: A 2-D Tensor of shape [num_rois, 4], specifying the locations of
     *      the regions of interest, each line with format [x1, y1, x2, y2].
     *      For input0 of type {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      this tensor should be of {@link OperandType::TENSOR_QUANT16_ASYMM},
     *      with zeroPoint of 0 and scale of 0.125.
     * * 2: An 1-D {@link OperandType::TENSOR_INT32} tensor, of shape
     *      [num_rois], specifying the batch index of each box. Boxes with
     *      the same batch index are grouped together.
     * * 3: An {@link OperandType::INT32} scalar, specifying the output
     *      height of the output tensor.
     * * 4: An {@link OperandType::INT32} scalar, specifying the output
     *      width of the output tensor.
     * * 5: An {@link OperandType::FLOAT32} scalar, specifying the ratio
     *      from the height of original image to the height of feature map.
     * * 6: An {@link OperandType::FLOAT32} scalar, specifying the ratio
     *      from the width of original image to the width of feature map.
     * * 7: An {@link OperandType::BOOL} scalar, set to true to specify
     *      NCHW data layout for input0 and output0. Set to false for NHWC.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0. The output
     *      shape is [num_rois, out_height, out_width, depth].
     *      For input0 of type {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    ROI_POOLING = 82,

    /**
     * Computes reciprocal of square root of x element-wise.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM} (since NNAPI feature level 7)
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since NNAPI feature level 7)
     *
     * Supported tensor rank: from 1.
     *
     * Inputs:
     * * 0: A tensor.
     *
     * Outputs:
     * * 0: The output tensor of same shape as input0.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint can be different from inputs' scale and zeroPoint.
     */
    RSQRT = 83,

    /**
     * Using a tensor of booleans c and input tensors x and y select values
     * elementwise from both input tensors:
     *
     * O[i] = C[i] ? x[i] : y[i].
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: from 1
     *
     * Inputs:
     * * 0: A tensor of type {@link OperandType::TENSOR_BOOL8} acting as a
     *      mask that chooses, based on the value at each element, whether the
     *      corresponding element in the output should be taken from input1 (if
     *      true) or input2 (if false).
     * * 1: An input tensor of the same shape as input0.
     * * 2: An input tensor of the same shape and type as input1.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM}
     *      and {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scales and zeroPoint can be different from input1 scale and zeroPoint.
     *
     * Outputs:
     * * 0: A tensor of the same type and shape as input1 and input2.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} tensor,
     *      the scale and zeroPoint can be different from inputs' scale and zeroPoint.
     */
    SELECT = 84,

    /**
     * Computes sin of x element-wise.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * Supported tensor rank: from 1.
     *
     * Inputs:
     * * 0: A tensor.
     *
     * Outputs:
     * * 0: The output tensor of same shape as input0.
     */
    SIN = 85,

    /**
     * Extracts a slice of specified size from the input tensor starting at a
     * specified location.
     *
     * The starting location is specified as a 1-D tensor containing offsets
     * for each dimension. The size is specified as a 1-D tensor containing
     * either size of a slice along corresponding dimension or -1. In the latter
     * case, all the remaining elements in dimension are included in the slice.
     *
     * A sum of begin offset and a size of a slice must not exceed size of a
     * corresponding dimension.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: from 1
     *
     * Inputs:
     * * 0: An n-D tensor to take slice from, may be zero-sized.
     * * 1: A 1-D tensor of type {@link OperandType::TENSOR_INT32} specifying
     *      the beginning indices of the slice in each dimension.
     * * 2: A 1-D tensor of type {@link OperandType::TENSOR_INT32} specifying
     *      the size of the slice in each dimension.
     *
     * Outputs:
     * * 0: An n-D tensor of the same type as the input containing the slice.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      its scale and zeroPoint has to be same as the input0 scale and zeroPoint.
     */
    SLICE = 86,

    /**
     * Splits a tensor along a given axis into num_splits subtensors.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: from 1
     *
     * Inputs:
     * * 0: An n-D tensor to split.
     * * 1: An {@link OperandType::INT32} scalar specifying the axis along
     *      which to split.
     * * 2: An {@link OperandType::INT32} scalar indicating the number of
     *      splits along given axis. Must evenly divide axis size.
     *
     * Outputs:
     * * 0 ~ (num_splits - 1): Resulting subtensors.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    SPLIT = 87,

    /**
     * Computes square root of x element-wise.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * Supported tensor rank: from 1.
     *
     * Inputs:
     * * 0: A tensor.
     *
     * Outputs:
     * * 0: The output tensor of same shape as input0.
     */
    SQRT = 88,

    /**
     * Constructs a tensor by tiling a given tensor.
     *
     * This operation creates a new tensor by replicating `input` `multiples`
     * times. The output tensor's i-th dimension has `input.dims(i) * multiples[i]`
     * elements, and the values of `input` are replicated `multiples[i]` times
     * along the i-th dimension.
     * For example, tiling `[a b c d]` by `[2]` produces `[a b c d a b c d]`.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: from 1
     *
     * Inputs:
     * * 0: input, an n-D tensor specifying the input.
     * * 1: multiples, a 1-D tensor of {@link OperandType::TENSOR_INT32}.
     *      The length of multiples must be n.
     *
     * Outputs:
     * * 0: A tiled tensor of the same {@link OperandType} and rank as `input`.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    TILE = 89,

    /**
     * Finds values and indices of the k largest entries for the last dimension.
     *
     * Resulting values in each dimensions are sorted in descending order. If
     * two values are equal, the one with larger index appears first.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} (since HAL version 1.3)
     *
     * Supported tensor rank: from 1
     *
     * Inputs:
     * * 0: input, an n-D tensor specifying the input.
     * * 1: k, an {@link OperandType::INT32} scalar, specifying the number of
     *      top elements to look for along the last dimension.
     *
     * Outputs:
     * * 0: An n-D tensor of the same type as the input, containing the k
     *      largest elements along each last dimensional slice.
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     * * 1: An n-D tensor of type {@link OperandType::TENSOR_INT32}
     *      containing the indices of values within the last dimension of input.
     */
    TOPK_V2 = 90,

    /**
     * Performs the transpose of 2-D convolution operation.
     *
     * This operation is sometimes called "deconvolution" after Deconvolutional
     * Networks, but is actually the transpose (gradient) of
     * {@link OperandType::CONV_2D} rather than an actual deconvolution.
     *
     * The output dimensions are functions of the filter dimensions, stride, and
     * padding.
     *
     * Supported tensor {@link OperandType} configurations:
     * * 16 bit floating point:
     * * * {@link OperandType::TENSOR_FLOAT16} for input, filter, output, and bias.
     *
     * * 32 bit floating point:
     * * * {@link OperandType::TENSOR_FLOAT32} for input, filter, output, and bias.
     *
     * * Quantized:
     * * * {@link OperandType::TENSOR_QUANT8_ASYMM} for input, filter, and output.
     * * * {@link OperandType::TENSOR_INT32} for bias (with scale set to
     * * * input.scale * filter.scale).
     *
     * * Quantized with symmetric per channel quantization for the filter:
     * * * {@link OperandType::TENSOR_QUANT8_ASYMM} for input, and output.
     * * * {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL} for filter.
     * * * {@link OperandType::TENSOR_INT32} for bias (scale set to 0.0,
     * * * each value scaling is separate and equal to input.scale * filter.scales[channel]).
     *
     * Available since HAL version 1.3:
     * * Quantized signed (since HAL version 1.3):
     * * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} for input, filter, and output.
     * * * {@link OperandType::TENSOR_INT32} for bias (with scale set to
     * * * input.scale * filter.scale).
     *
     * * Quantized signed with filter symmetric per channel quantization
     *   (since HAL version 1.3):
     * * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} for input, and output.
     * * * {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL} for filter.
     * * * {@link OperandType::TENSOR_INT32} for bias (scale set to 0.0,
     * * * each value scaling is separate and equal to input.scale * filter.scales[channel]).
     *
     * Supported tensor rank: 4, with "NHWC" or "NCHW" data layout.
     * With the default data layout NHWC, the data is stored in the order of:
     * [batch, height, width, channels]. Alternatively, the data layout could
     * be NCHW, the data storage order of: [batch, channels, height, width].
     *
     * Both explicit padding and implicit padding are supported.
     *
     * Inputs (explicit padding):
     * * 0: A 4-D tensor, of shape [batches, height, width, depth_in],
     *      specifying the input.
     * * 1: A 4-D tensor, of shape
     *      [depth_out, filter_height, filter_width, depth_in], specifying the
     *      filter. For tensor of type
     *      {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL} the channel
     *      dimension (SymmPerChannelQuantParams::channelDim) must be set to 0.
     * * 2: A 1-D tensor, of shape [depth_out], specifying the bias. For input
     *      tensor of type {@link OperandType::TENSOR_FLOAT32} or
     *      {@link OperandType::TENSOR_FLOAT16}, the bias must be of the
     *      same type.
     *      For filter tensor of {@link OperandType::TENSOR_QUANT8_ASYMM}
     *      and {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED},
     *      the bias should be of {@link OperandType::TENSOR_INT32},
     *      with zeroPoint of 0 and bias_scale == input_scale * filter_scale.
     *      For filter tensor of {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL},
     *      the bias must be of {@link OperandType::TENSOR_INT32}, with zeroPoint of 0
     *      and bias_scale of 0. The actual scale of each value 'i' is equal to
     *      bias_scale[i] = input_scale * filter_scale[i].
     * * 3: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the left, in the ‘width’ dimension.
     * * 4: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the right, in the ‘width’ dimension.
     * * 5: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the top, in the ‘height’ dimension.
     * * 6: An {@link OperandType::INT32} scalar, specifying the padding on
     *      the bottom, in the ‘height’ dimension.
     * * 7: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘width’ dimension.
     * * 8: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘height’ dimension.
     * * 9: An {@link OperandType::INT32} scalar, and has to be one of the
     *      {@link FusedActivationFunc} values. Specifies the activation to
     *      invoke on the result.
     * * 10: An {@link OperandType::BOOL} scalar, set to true to specify
     *       NCHW data layout for input0 and output0. Set to false for NHWC.
     *
     * Inputs (implicit padding):
     * * 0: A 4-D tensor, of shape [batches, height, width, depth_in],
     *      specifying the input.
     * * 1: A 4-D tensor, of shape
     *      [depth_out, filter_height, filter_width, depth_in], specifying the
     *      filter. For tensor of type
     *      {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL} the channel
     *      dimension (SymmPerChannelQuantParams::channelDim) must be set to 0.
     * * 2: A 1-D tensor, of shape [depth_out], specifying the bias. For input
     *      tensor of type {@link OperandType::TENSOR_FLOAT32} or
     *      {@link OperandType::TENSOR_FLOAT16}, the bias should be of the
     *      same type.
     *      For filter tensor of {@link OperandType::TENSOR_QUANT8_ASYMM}
     *      and {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED},
     *      the bias should be of {@link OperandType::TENSOR_INT32},
     *      with zeroPoint of 0 and bias_scale == input_scale * filter_scale.
     *      For filter tensor of {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL},
     *      the bias must be of {@link OperandType::TENSOR_INT32}, with zeroPoint of 0
     *      and bias_scale of 0. The actual scale of each value 'i' is equal to
     *      bias_scale[i] = input_scale * filter_scale[i].
     * * 3: An {@link OperandType::TENSOR_INT32} tensor, specifying the output
     *      tensor shape.
     * * 4: An {@link OperandType::INT32} scalar, specifying the implicit
     *      padding scheme, has to be one of the
     *      following values: {0 (NONE), 1 (SAME), 2 (VALID)}.
     * * 5: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘width’ dimension.
     * * 6: An {@link OperandType::INT32} scalar, specifying the stride when
     *      walking through input in the ‘height’ dimension.
     * * 7: An {@link OperandType::INT32} scalar, and has to be one of the
     *      {@link FusedActivationFunc} values. Specifies the activation to
     *      invoke on the result.
     * * 8: An {@link OperandType::BOOL} scalar, set to true to specify
     *      NCHW data layout for input0 and output0. Set to false for NHWC.
     *
     * Outputs:
     * * 0: The output 4-D tensor, of shape
     *      [batches, out_height, out_width, depth_out].
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint can be different from inputs' scale and zeroPoint.
     */
    TRANSPOSE_CONV_2D = 91,

    /**
     * A recurrent neural network specified by an LSTM cell.
     *
     * Performs (fully) dynamic unrolling of input.
     *
     * This Op unrolls the input along the time dimension, and implements the
     * following operation for each element in the sequence
     * s = 1...sequence_length:
     *   outputs[s] = projection(state = activation(LSTMOp(inputs[s])))
     *
     * Where LSTMOp is the LSTM op as in {@link OperandType::LSTM},
     * the "projection" is an optional projection layer from state and output
     * and the “activation” is the function passed as the
     * “fused_activation_function” argument (if not “NONE”).
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * Supported tensor rank: 3, either time-major or batch-major.
     *
     * All input and output tensors must be of the same type.
     *
     * Inputs:
     * * 0: The input (\f$x_t\f$).
     *      A 3-D tensor of shape:
     *        If time-major: [max_time, batch_size, input_size]
     *        If batch-major: [batch_size, max_time, input_size]
     *      where “max_time” is the number of timesteps (sequence length),
     *      “batch_size” corresponds to the batching dimension, and
     *      “input_size” is the size of the input.
     * * 1: The input-to-input weights (\f$W_{xi}\f$). Optional.
     *      A 2-D tensor of shape [num_units, input_size], where “num_units”
     *      corresponds to the number of cell units.
     * * 2: The input-to-forget weights (\f$W_{xf}\f$).
     *      A 2-D tensor of shape [num_units, input_size].
     * * 3: The input-to-cell weights (\f$W_{xc}\f$).
     *      A 2-D tensor of shape [num_units, input_size].
     * * 4: The input-to-output weights (\f$W_{xo}\f$).
     *      A 2-D tensor of shape [num_units, input_size].
     * * 5: The recurrent-to-input weights (\f$W_{hi}\f$). Optional.
     *      A 2-D tensor of shape [num_units, output_size], where “output_size”
     *      corresponds to either the number of cell units (i.e., “num_units”),
     *      or the second dimension of the “projection_weights”, if defined.
     * * 6: The recurrent-to-forget weights (\f$W_{hf}\f$).
     *      A 2-D tensor of shape [num_units, output_size].
     * * 7: The recurrent-to-cell weights (\f$W_{hc}\f$).
     *      A 2-D tensor of shape [num_units, output_size].
     * * 8: The recurrent-to-output weights (\f$W_{ho}\f$).
     *      A 2-D tensor of shape [num_units, output_size].
     * * 9: The cell-to-input weights (\f$W_{ci}\f$). Optional.
     *      A 1-D tensor of shape [num_units].
     * * 10:The cell-to-forget weights (\f$W_{cf}\f$). Optional.
     *      A 1-D tensor of shape [num_units].
     * * 11:The cell-to-output weights (\f$W_{co}\f$). Optional.
     *      A 1-D tensor of shape [num_units].
     * * 12:The input gate bias (\f$b_i\f$). Optional.
     *      A 1-D tensor of shape [num_units].
     * * 13:The forget gate bias (\f$b_f\f$).
     *      A 1-D tensor of shape [num_units].
     * * 14:The cell bias (\f$b_c\f$).
     *      A 1-D tensor of shape [num_units].
     * * 15:The output gate bias (\f$b_o\f$).
     *      A 1-D tensor of shape [num_units].
     * * 16:The projection weights (\f$W_{proj}\f$). Optional.
     *      A 2-D tensor of shape [output_size, num_units].
     * * 17:The projection bias (\f$b_{proj}\f$). Optional.
     *      A 1-D tensor of shape [output_size].
     * * 18:The output state (in) (\f$h_{t-1}\f$).
     *      A 2-D tensor of shape [batch_size, output_size].
     * * 19:The cell state (in) (\f$C_{t-1}\f$).
     *      A 2-D tensor of shape [batch_size, num_units].
     * * 20:The activation function (\f$g\f$).
     *      A value indicating the activation function:
     *      <ul>
     *      <li>0: None;
     *      <li>1: Relu;
     *      <li>3: Relu6;
     *      <li>4: Tanh;
     *      <li>6: Sigmoid.
     *      </ul>
     * * 21:The clipping threshold (\f$t_{cell}\f$) for the cell state, such
     *      that values are bound within [-cell_clip, cell_clip]. If set to 0.0
     *      then clipping is disabled.
     * * 22:The clipping threshold (\f$t_{proj}\f$) for the output from the
     *      projection layer, such that values are bound within
     *      [-proj_clip, proj_clip]. If set to 0.0 then clipping is disabled.
     * * 23:Time-major if true, batch-major if false.
     * * 24:The input layer normalization weights. Optional.
     *      A 1-D tensor of shape [num_units]. Used to rescale normalized inputs
     *      to activation at input gate.
     * * 25:The forget layer normalization weights. Optional.
     *      A 1-D tensor of shape [num_units]. Used to rescale normalized inputs
     *      to activation at forget gate.
     * * 26:The cell layer normalization weights. Optional.
     *      A 1-D tensor of shape [num_units]. Used to rescale normalized inputs
     *      to activation at cell gate.
     * * 27:The output layer normalization weights. Optional.
     *      A 1-D tensor of shape [num_units]. Used to rescale normalized inputs
     *      to activation at output gate.
     *
     * Outputs:
     * * 0: The output (\f$o_t\f$).
     *      A 3-D tensor of shape:
     *        If time-major: [max_time, batch_size, output_size]
     *        If batch-major: [batch_size, max_time, output_size]
     * * 1: A tensor of shape [batch_size, output_size] containing a hidden
     *      state from the last time step in the sequence. This output is
     *      optional and can be omitted. If this output is present then
     *      output #2 must be present as well.
     *      Available since HAL version 1.3.
     * * 2: A tensor of shape [batch_size, cell_size] containing a cell state
     *      from the last time step in the sequence. This output is optional
     *      and can be omitted.
     *      Available since HAL version 1.3.
     */
    UNIDIRECTIONAL_SEQUENCE_LSTM = 92,

    /**
     * A recurrent neural network layer that applies a basic RNN cell to a
     * sequence of inputs.
     *
     * This layer unrolls the input along the sequence dimension, and implements
     * the following operation
     * for each element in the sequence s = 1...sequence_length:
     *   outputs[s] = state = activation(inputs[s] * input_weights’ + state *
     *   recurrent_weights’ + bias)
     *
     * Where:
     * * “input_weights” is a weight matrix that multiplies the inputs;
     * * “recurrent_weights” is a weight matrix that multiplies the current
     *    “state” which itself is the output from the previous time step
     *    computation;
     * * “bias” is a bias vector (added to each output vector in the batch);
     * * “activation” is the function passed as the “fused_activation_function”
     *   argument (if not “NONE”).
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * The input tensors must all be the same type.
     *
     * Inputs:
     * * 0: input.
     *      A 3-D tensor. The shape is defined by the input 6 (timeMajor). If
     *      it is set to 1, then the input has a shape [maxTime, batchSize,
     *      inputSize], otherwise the input has a shape [batchSize, maxTime,
     *      inputSize].
     * * 1: weights.
     *      A 2-D tensor of shape [numUnits, inputSize].
     * * 2: recurrent_weights.
     *      A 2-D tensor of shape [numUnits, numUnits].
     * * 3: bias.
     *      A 1-D tensor of shape [numUnits].
     * * 4: hidden state
     *      A 2-D tensor of shape [batchSize, numUnits]. Specifies a hidden
     *      state input for the first time step of the computation.
     * * 5: fusedActivationFunction.
     *      A {@link FusedActivationFunc} value indicating the activation function. If
     *      “NONE” is specified then it results in a linear activation.
     * * 6: timeMajor
     *      An {@link OperandType::INT32} scalar specifying the shape format
     *      of input and output tensors. Must be set to either 0 or 1.
     * Outputs:
     * * 0: output.
     *      A 3-D tensor. The shape is defined by the input 6 (timeMajor). If
     *      it is set to 1, then the output has a shape [maxTime, batchSize,
     *      numUnits], otherwise the output has a shape [batchSize, maxTime,
     *      numUnits].
     */
    UNIDIRECTIONAL_SEQUENCE_RNN = 93,

    /**
     * Resizes images to given size using the nearest neighbor interpretation.
     *
     * Resized images must be distorted if their output aspect ratio is not the
     * same as input aspect ratio. The corner pixels of output may not be the
     * same as corner pixels of input.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     *
     * Supported tensor rank: 4, with "NHWC" or "NCHW" data layout.
     * With the default data layout NHWC, the data is stored in the order of:
     * [batch, height, width, channels]. Alternatively, the data layout could
     * be NCHW, the data storage order of: [batch, channels, height, width].
     *
     * Both resizing by shape and resizing by scale are supported.
     *
     * Inputs (resizing by shape):
     * * 0: A 4-D tensor, of shape [batches, height, width, depth], specifying
     *      the input. Zero batches is supported for this tensor.
     * * 1: An {@link OperandType::INT32} scalar, specifying the output
     *      width of the output tensor.
     * * 2: An {@link OperandType::INT32} scalar, specifying the output
     *      height of the output tensor.
     * * 3: An {@link OperandType::BOOL} scalar, default to false.
     *      Set to true to specify NCHW data layout for input0 and output0.
     * * 4: Align corners. An optional {@link OperandType::BOOL}
     *      scalar, default to false.  If True, the centers of the 4 corner
     *      pixels of the input and output tensors are aligned, preserving the
     *      values at the corner pixels.
     *      Available since HAL version 1.3.
     * * 5: Half pixel centers. An optional {@link OperandType::BOOL}
     *      scalar, default to false. If True, the pixel centers are assumed to
     *      be at (0.5, 0.5). This is the default behavior of image.resize in
     *      TF 2.0. If this parameter is True, then align_corners parameter
     *      must be False.
     *      Available since HAL version 1.3.
     *
     * Inputs (resizing by scale):
     * * 0: A 4-D tensor, of shape [batches, height, width, depth], specifying
     *      the input. Zero batches is supported for this tensor.
     * * 1: A scalar, specifying width_scale, the scaling factor of the width
     *      dimension from the input tensor to the output tensor. The output
     *      width is calculated as new_width = floor(width * width_scale).
     *      The scalar must be of {@link OperandType::FLOAT16} if input0 is
     *      of {@link OperandType::TENSOR_FLOAT16} and of
     *      {@link OperandType::FLOAT32} otherwise.
     * * 2: A scalar, specifying height_scale, the scaling factor of the height
     *      dimension from the input tensor to the output tensor. The output
     *      height is calculated as new_height = floor(height * height_scale).
     *      The scalar must be of {@link OperandType::FLOAT16} if input0 is
     *      of {@link OperandType::TENSOR_FLOAT16} and of
     *      {@link OperandType::FLOAT32} otherwise.
     * * 3: An {@link OperandType::BOOL} scalar, default to false.
     *      Set to true to specify NCHW data layout for input0 and output0.
     * * 4: Align corners. An optional {@link OperandType::BOOL}
     *      scalar, default to false.  If True, the centers of the 4 corner
     *      pixels of the input and output tensors are aligned, preserving the
     *      values at the corner pixels.
     *      Available since HAL version 1.3.
     * * 5: Half pixel centers. An optional {@link OperandType::BOOL}
     *      scalar, default to false. If True, the pixel centers are assumed to
     *      be at (0.5, 0.5). This is the default behavior of image.resize in
     *      TF 2.0. If this parameter is True, then align_corners parameter
     *      must be False.
     *      Available since HAL version 1.3.
     *
     * Outputs:
     * * 0: The output 4-D tensor, of shape
     *      [batches, new_height, new_width, depth].
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    RESIZE_NEAREST_NEIGHBOR = 94,

    /**
     * Quantized version of {@link OperationType::LSTM}.
     *
     * The input and the output use asymmetric quantized types, while the rest
     * use symmetric ones.
     *
     * Inputs:
     * * 0: The input to the LSTM cell.
     *      Type: {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}
     *      Shape: [batchSize, inputSize]
     * * 1: The input-to-input weights. Optional.
     *      Type: {@link OperandType::TENSOR_QUANT8_SYMM}
     *      Shape: [numUnits, inputSize]
     * * 2: The input-to-forget weights.
     *      Type: {@link OperandType::TENSOR_QUANT8_SYMM}
     *      Shape: [numUnits, inputSize]
     * * 3: The input-to-cell weights.
     *      Type: {@link OperandType::TENSOR_QUANT8_SYMM}
     *      Shape: [numUnits, inputSize]
     * * 4: The input-to-output weights.
     *      Type: {@link OperandType::TENSOR_QUANT8_SYMM}
     *      Shape: [numUnits, inputSize]
     * * 5: The recurrent-to-input weights. Optional.
     *      Type: {@link OperandType::TENSOR_QUANT8_SYMM}
     *      Shape: [numUnits, outputSize]
     * * 6: The recurrent-to-forget weights.
     *      Type: {@link OperandType::TENSOR_QUANT8_SYMM}
     *      Shape: [numUnits, outputSize]
     * * 7: The recurrent-to-cell weights.
     *      Type: {@link OperandType::TENSOR_QUANT8_SYMM}
     *      Shape: [numUnits, outputSize]
     * * 8: The recurrent-to-output weights.
     *      Type: {@link OperandType::TENSOR_QUANT8_SYMM}
     *      Shape: [numUnits, outputSize]
     * * 9: The cell-to-input weights (for peephole). Optional.
     *      Type: {@link OperandType::TENSOR_QUANT16_SYMM}
     *      Shape: [numUnits]
     * * 10: The cell-to-forget weights (for peephole). Optional.
     *       Type: {@link OperandType::TENSOR_QUANT16_SYMM}
     *       Shape: [numUnits]
     * * 11: The cell-to-output weights (for peephole). Optional.
     *       Type: {@link OperandType::TENSOR_QUANT16_SYMM}
     *       Shape: [numUnits]
     * * 12: The input gate bias. Quantized with scale being the
     *       product of input and weights scales and zeroPoint equal to 0.
     *       Optional.
     *       Type: {@link OperandType::TENSOR_INT32}
     *       Shape: [numUnits]
     * * 13: The forget gate bias. Quantized with scale being the
     *       product of input and weights scales and zeroPoint equal to 0.
     *       Type: {@link OperandType::TENSOR_INT32}
     *       Shape: [numUnits]
     * * 14: The cell bias. Quantized with scale being the
     *       product of input and weights scales and zeroPoint equal to 0.
     *       Type: {@link OperandType::TENSOR_INT32}
     *       Shape: [numUnits]
     * * 15: The output gate bias. Quantized with scale being the
     *       product of input and weights scales and zeroPoint equal to 0.
     *       Type: {@link OperandType::TENSOR_INT32}
     *       Shape: [numUnits]
     * * 16: The projection weights. Optional.
     *       Type: {@link OperandType::TENSOR_QUANT8_SYMM}
     *       Shape: [outputSize, numUnits]
     * * 17: The projection bias. Quantized with scale being the
     *       product of input and weights scales and zeroPoint equal to 0.
     *       Optional.
     *       Type: {@link OperandType::TENSOR_INT32}
     *       Shape: [outputSize]
     * * 18: The output from the previous time step.
     *       Type: {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}
     *       Shape: [batchSize, outputSize]
     * * 19: The cell state from the previous time step.
     *       Type: {@link OperandType::TENSOR_QUANT16_SYMM}
     *       Shape: [batchSize, numUnits]
     * * 20: The input layer normalization weights. Used to rescale
     *       normalized inputs to activation at input gate. Optional.
     *       Type: {@link OperandType::TENSOR_QUANT16_SYMM}
     *       Shape: [numUnits]
     * * 21: The forget layer normalization weights. Used to
     *       rescale normalized inputs to activation at forget gate. Optional.
     *       Type: {@link OperandType::TENSOR_QUANT16_SYMM}
     *       Shape: [numUnits]
     * * 22: The cell layer normalization weights. Used to rescale
     *       normalized inputs to activation at cell gate. Optional.
     *       Type: {@link OperandType::TENSOR_QUANT16_SYMM}
     *       Shape: [numUnits]
     * * 23: The output layer normalization weights. Used to
     *       rescale normalized inputs to activation at output gate. Optional.
     *       Type: {@link OperandType::TENSOR_QUANT16_SYMM}
     *       Shape: [numUnits]
     * * 24: The cell clip. If provided the cell state is clipped
     *       by this value prior to the cell output activation. Optional.
     *       Type: {@link OperandType::FLOAT32}.
     * * 25: The projection clip. If provided and projection is enabled,
     *       this is used for clipping the projected values. Optional.
     *       Type: {@link OperandType::FLOAT32}.
     * * 26: The scale of the intermediate result of matmul,
     *       i.e. input to layer normalization, at input gate.
     *       Type: {@link OperandType::FLOAT32}.
     * * 27: The scale of the intermediate result of matmul,
     *       i.e. input to layer normalization, at forget gate.
     *       Type: {@link OperandType::FLOAT32}.
     * * 28: The scale of the intermediate result of matmul,
     *       i.e. input to layer normalization, at cell gate.
     *       Type: {@link OperandType::FLOAT32}.
     * * 29: The scale of the intermediate result of matmul,
     *       i.e. input to layer normalization, at output gate.
     *       Type: {@link OperandType::FLOAT32}.
     * * 30: The zero point of the hidden state, i.e. input to
     *       projection.
     *       Type: {@link OperandType::INT32}.
     * * 31: The scale of the hidden state, i.e. input to
     *       projection.
     *       Type: {@link OperandType::FLOAT32}.
     *
     * Outputs:
     * * 0: The output state (out).
     *      Type: {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}
     *      Shape: [batchSize, outputSize]
     * * 1: The cell state (out).
     *      Type: {@link OperandType::TENSOR_QUANT16_SYMM}
     *      Shape: [batchSize, numUnits]
     * * 2: The output. This is effectively the same as the current
     *      "output state (out)" value.
     *      Type: {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}
     *      Shape: [batchSize, outputSize]
     */
    QUANTIZED_LSTM = 95,

    /**
     * Executes one of the two referenced subgraphs as determined by a boolean
     * value.
     *
     * The inputs and outputs of the two referenced subgraphs must agree with the
     * signature of this operation. That is, if the operation has (3 + n) inputs
     * and m outputs, both subgraphs must have n inputs and m outputs with the same
     * types, ranks, dimensions, scales,
     * zeroPoints, and extraParams as the corresponding operation
     * inputs and outputs.
     * All of the operands mentioned must have fully specified dimensions.
     *
     * Inputs:
     * * 0: A value of type {@link OperandType::TENSOR_BOOL8} and shape [1]
     *      that determines which of the two referenced subgraphs to execute.
     *      The operand must have fully specified dimensions.
     * * 1: A {@link OperandType::SUBGRAPH} reference to the subgraph to be
     *      executed if the condition is true.
     * * 2: A {@link OperandType::SUBGRAPH} reference to the subgraph to be
     *      executed if the condition is false.
     * * 3 ~ (n + 2): Inputs to be passed to the subgraph selected for execution.
     *
     * Outputs:
     * * 0 ~ (m - 1): Outputs produced by the selected subgraph.
     */
    IF = 96,

    /**
     * Executes the body subgraph until the condition subgraph outputs false.
     *
     * The inputs to this operation are the condition subgraph, the body subgraph,
     * and operand values for the first iteration of the loop. The values are
     * implicitly split into three groups of input-output, state-only, and
     * input-only values, as described below.
     *
     * The outputs of this operation are the final values of input-output
     * operands.
     *
     * Both the condition and body subgraph receive (m + k + n) inputs.
     * * The first m (m >= 1) inputs are input-output operands. For the first
     *   iteration, these are initialized from the corresponding inputs of the
     *   WHILE operation. In subsequent iterations, their values come from the
     *   corresponding outputs of the body subgraph produced during the previous
     *   iteration.
     * * The next k (k >= 0) inputs are state-only operands. They are similar to
     *   the input-output operands, except that their values are no longer
     *   available after the loop terminates.
     * * The last n (n >= 0) inputs are input-only operands. Their values come
     *   from the corresponding inputs of the WHILE operation.
     *
     * The body subgraph produces (m + k) outputs.
     * * The first m outputs are input-output operands. They become the outputs
     *   of the WHILE operation when a termination condition is reached.
     * * The last k outputs are state-only operands. Their values are no longer
     *   available after the loop terminates.
     *
     * The numbers m, k, and n are inferred by the driver as follows:
     *     m = (WHILE operation output count)
     *     k = (body subgraph output count) - m
     *     n = (body subgraph input count) - m - k
     *
     * The pseudo-code below illustrates the flow of a WHILE operation with
     * inputs condition, body, initial_input_output, initial_state, input_only
     * (m = 1, k = 1, n = 1):
     *
     *     input_output = initial_input_output
     *     state = initial_state
     *     while condition(input_output, state, input_only):
     *         input_output, state = body(input_output, state, input_only)
     *     return input_output
     *
     * Inputs:
     * * 0: A {@link OperandType::SUBGRAPH} reference to the condition
     *      subgraph. The subgraph must have (m + k + n) inputs with
     *      the same types, ranks, dimensions,
     *      scales, zeroPoints, and extraParams as the
     *      corresponding inputs of the WHILE operation and exactly one output
     *      of {@link OperandType::TENSOR_BOOL8} and shape [1].
     *      All of the operands mentioned must have fully specified dimensions.
     * * 1: A {@link OperandType::SUBGRAPH} reference to the body subgraph.
     *      The subgraph must have (m + k + n) inputs and (m + k) outputs with
     *      the same types, ranks, dimensions,
     *      scales, zeroPoints, and extraParams as the
     *      corresponding inputs and outputs of the WHILE operation.
     *      All of the operands mentioned must have fully specified dimensions.
     * * (m inputs): Initial values for input-output operands.
     * * (k inputs): Initial values for state-only operands.
     * * (n inputs): Values for input-only operands.
     *
     * Outputs:
     * * 0 ~ (m - 1): Outputs produced by the loop.
     */
    WHILE = 97,

    /**
     * Computes exponential linear activation on the input tensor element-wise.
     *
     * The output is calculated using the following formula:
     *
     *     ELU(x) = max(0, x) + min(0, alpha * (exp(x) - 1))
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     *
     * Supported tensor rank: from 1.
     *
     * Inputs:
     * * 0: A tensor, specifying the input. May be zero-sized.
     * * 1: A scalar, specifying the alpha parameter.
     *      For input tensor of {@link OperandType::TENSOR_FLOAT16},
     *      the alpha value must be of {@link OperandType::FLOAT16}.
     *      For input tensor of {@link OperandType::TENSOR_FLOAT32},
     *      the alpha value must be of {@link OperandType::FLOAT32}.
     *
     * Outputs:
     * * 0: The output tensor of same shape and type as input0.
     */
    ELU = 98,

    /**
     * Computes hard-swish activation on the input tensor element-wise.
     *
     * Hard swish activation is introduced in
     * https://arxiv.org/pdf/1905.02244.pdf
     *
     * The output is calculated using the following formula:
     *
     *     h-swish(x) = x * max(0, min(6, (x + 3))) / 6
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}
     *
     * Supported tensor rank: from 1.
     *
     * Inputs:
     * * 0: A tensor, specifying the input. May be zero-sized.
     *
     * Outputs:
     * * 0: The output tensor of same shape and type as input0.
     *      Scale and zero point of this tensor may be different from the input
     *      tensor's parameters.
     */
    HARD_SWISH = 99,

    /**
     * Creates a tensor filled with a scalar value.
     *
     * Supported output tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     *
     * Supported tensor rank: from 1.
     *
     * Inputs:
     * * 0: A 1-D tensor, specifying the desired output tensor shape.
     * * 1: A scalar, specifying the value to fill the output tensors with.
     *      For output tensor of {@link OperandType::TENSOR_FLOAT16},
     *      the scalar must be of {@link OperandType::FLOAT16}.
     *      For output tensor of {@link OperandType::TENSOR_FLOAT32},
     *      the scalar must be of {@link OperandType::FLOAT32}.
     *      For output tensor of {@link OperandType::TENSOR_INT32},
     *      the scalar must be of {@link OperandType::INT32}.
     *
     * Outputs:
     * * 0: The output tensor.
     */
    FILL = 100,

    /**
     * Returns the rank of a tensor.
     *
     * The rank of a tensor is the number of dimensions in it. Also known as
     * "order", "degree", "ndims".
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT16_SYMM}
     * * {@link OperandType::TENSOR_BOOL8}
     * * {@link OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL}
     * * {@link OperandType::TENSOR_QUANT16_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_SYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}
     *
     * Supported tensor rank: from 1.
     *
     * Inputs:
     * * 0: The input tensor.
     *
     * Outputs:
     * * 0: A scalar of {@link OperandType::INT32}, specifying the rank
     *      of the input tensor.
     */
    RANK = 101,

    /**
     * Performs multiplication of two tensors in batches.
     *
     * Multiplies all slices of two input tensors and arranges the individual
     * results in a single output tensor of the same batch size. Each pair of
     * slices in the same batch have identical {@link OperandType}. Each
     * slice can optionally be adjointed (transpose and conjugate) before
     * multiplication.
     *
     * The two input tensors and the output tensor must be 2-D or higher and
     * have the same batch size.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}
     * * {@link OperandType::TENSOR_INT32}
     *
     * Supported tensor rank: at least 2 and up to 4
     *
     * Inputs:
     * * 0: A tensor with 2-D or higher shape [..., r_x, c_x].
     * * 1: A tensor with 2-D or higher shape [..., r_y, c_y]. It has the same
     *      {@link OperandType} and batch size as input0.
     * * 2: An optional {@link OperandType::BOOL} scalar adj_x, default
     *      to false. Set to true to adjoint the slices of input0.
     * * 3: An optional {@link OperandType::BOOL} scalar adj_y, default
     *      to false. Set to true to adjoint the slices of input1.
     *
     * Outputs:
     * * 0: A tensor with 2-D or higher shape [..., r_o, c_o], where
     *      r_o = c_x if adj_x else r_x
     *      c_o = r_y if adj_y else c_y
     */
    BATCH_MATMUL = 102,

    /**
     * Packs N input tensors (N >= 1) of rank R into one output tensor of rank R+1.
     * The tensors are packed along a given axis.
     *
     * The input tensors must have identical {@link OperandType} and dimensions.
     *
     * For example, suppose there are N input tensors of shape (A, B, C).
     * If axis is 0, the output tensor will have shape (N, A, B, C).
     * If axis is 1, the output tensor will have shape (A, N, B, C).
     *
     * All dimensions through the axis dimension determine the output tile count;
     * the remaining dimensions determine the tile shape.
     *
     * Return to the example of N input tensors of shape (A, B, C).
     * If axis is 0, there are N tiles in the output, each of shape (A, B, C).
     * If axis is 1, there are A*N tiles in the output, each of shape (B, C).
     *
     * The coordinates of a tile within the output tensor are (t[0],...,t[axis]).
     * The coordinates of a tile within an input tensor are (t[0],...,t[axis-1]).
     * (If axis is 0, an input tensor consists of a single tile.)
     * If we index input tensors starting with 0 (rather than by operand number),
     * then output_tile[t[0],...,t[axis]] = input_tile[t[axis]][t[0],...,t[axis-1]].
     * That is, all output tile coordinates except for the axis coordinate select
     * the corresponding location within some input tensor; and the axis coordinate
     * selects the input tensor.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}
     * * {@link OperandType::TENSOR_INT32}
     *
     * Supported input tensor rank: from 1
     *
     * Inputs:
     * * 0: A scalar of type {@link OperandType::INT32}, specifying
     *      the axis along which to pack.  The valid range is [0, R+1).
     * * 1 ~ N: Input tensors to be packed together.
     *          For {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *          {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensors,
     *          the scales and zeroPoint must be the same for all input tensors,
     *          and will be the same for the output tensor.
     *
     * Outputs:
     * * 0: The packed tensor.
     */
    PACK = 103,

    /**
     * Pads a tensor with mirrored values.
     *
     * This operator specifies one of two padding modes: REFLECT or SYMMETRIC.
     * In the case of REFLECT mode, the mirroring excludes the border element
     * on the padding side.
     * In the case of SYMMETRIC mode, the mirroring includes the border element
     * on the padding side.
     *
     * For example, if the input is the 1-D tensor `[1, 2, 3]` and the padding
     * is `[0, 2]` (i.e., pad no elements before the first (and only) dimension,
     * and two elements after the first (and only) dimension), then:
     *     - REFLECT mode produces the output `[1, 2, 3, 2, 1]`
     *     - SYMMETRIC mode produces the output `[1, 2, 3, 3, 2]`
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}
     * * {@link OperandType::TENSOR_INT32}
     *
     * Supported tensor rank: from 1.
     *
     * Inputs:
     * * 0: An n-D tensor, specifying the tensor to be padded.
     * * 1: A 2-D tensor of {@link OperandType::TENSOR_INT32}, the paddings
     *      for each spatial dimension of the input tensor. The shape of the
     *      tensor must be {rank(input0), 2}.
     *      padding[i, 0] specifies the number of elements to be padded in the
     *      front of dimension i.
     *      padding[i, 1] specifies the number of elements to be padded after the
     *      end of dimension i.
     *      Each padding value must be nonnegative.
     *      In the case of REFLECT mode, each padding value must be less than the
     *      corresponding dimension.
     *      In the case of SYMMETRIC mode, each padding value must be less than or
     *      equal to the corresponding dimension.
     * * 2: An {@link OperandType::INT32} scalar, specifying the mode.
     *      Options are 0:REFLECT and 1:SYMMETRIC.
     *
     * Outputs:
     * * 0: A tensor of the same {@link OperandType} as input0. The
     *      output tensor has the same rank as input0, and each
     *      dimension of the output tensor has the same size as the
     *      corresponding dimension of the input tensor plus the size
     *      of the padding:
     *          output0.dimension[i] =
     *              padding[i, 0] + input0.dimension[i] + padding[i, 1]
     *      For a {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensor,
     *      the scale and zeroPoint must be the same as input0.
     */
    MIRROR_PAD = 104,

    /**
     * Reverses a specified dimension of a tensor.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}
     * * {@link OperandType::TENSOR_INT32}
     *
     * Supported tensor rank: up to 8.
     *
     * Inputs:
     * * 0: Input tensor of rank n.
     * * 1: Axis tensor of type {@link OperandType::TENSOR_INT32} and shape [1],
     *      specifying which dimension of the input tensor is to be reversed. The dimension
     *      must be in the range [0, n).
     *
     * Outputs:
     * * 0: The reversed tensor of the same shape as the input tensor.
     *      For {@link OperandType::TENSOR_QUANT8_ASYMM} and
     *      {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED} tensors,
     *      the scales and zeroPoint must be the same as input0.
     */
    REVERSE = 105,

    /**
     * DEPRECATED. Since HAL version 1.2, extensions are the preferred
     * alternative to OEM operation and data types.
     *
     * This operation is OEM specific. It should only be used for OEM
     * applications.
     */
    OEM_OPERATION = 10000,

#ifdef NN_EXPERIMENTAL_FEATURE
    /**
     * Expands a representation of a sparse tensor to a dense tensor.
     *
     * To encode a conceptual n-dimensional dense tensor with dims [D0, ..., Dn-1], potentially with
     * a k-dimensional block (0 <= k <= n) with dims [Dn, ..., Dn+k-1], the format specifies:
     * * 1: In what order to traverse these dimensions. For example, to store a 2-D matrix in row
     *      major order, the traversal order would be [D0, D1], whereas to store it in column major
     *      order, the traversal order would be [D1, D0]. If the 2-D matrix has a 2-D inner block,
     *      the traversal order could be [D0, D1, D2, D3].
     * * 2: How each block dimension in [Dn, ..., Dn+k-1] maps to the original tensor dimension in
     *      [D0, ..., Dn-1].
     * * 3: In the traversal order defined above, the format (dense vs. sparse) and index metadata
     *      for each dimension. For a dense dimension, this is just the size of that dimension. For
     *      a sparse dimension, it's the same as the compressed index defined in the Compressed
     *      Sparse Row (CSR) format.
     *      (http://scipy-lectures.org/advanced/scipy_sparse/csr_matrix.html)
     *
     * The number of inputs to this operation is determined by the number of dimensions (including
     * the block dimensions) of the sparsity parameters. Currently, the only formats supported are
     * DENSE and SPARSE_CSR, but additional sparsity formats may be added in later versions of this
     * operation.
     *
     * Supported tensor {@link OperandType}:
     * * {@link OperandType::TENSOR_FLOAT16}
     * * {@link OperandType::TENSOR_FLOAT32}
     * * {@link OperandType::TENSOR_QUANT8_SYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM}
     * * {@link OperandType::TENSOR_QUANT8_ASYMM_SIGNED}
     * * {@link OperandType::TENSOR_BOOL8}
     * * {@link OperandType::TENSOR_INT32}
     * * {@link OperandType::TENSOR_QUANT16_SYMM}
     * * {@link OperandType::TENSOR_QUANT16_ASYMM}
     *
     *
     * Reference:
     * * This implementation is a modification of the TACO format.
     *   http://tensor-compiler.org/kjolstad-oopsla17-tensor-compiler.pdf
     *
     * Inputs:
     * * 0: A 1-D tensor representing the compressed sparse tensor data of a conceptual
     *      n-dimensional tensor.
     * * 1: A 1-D {@link OperandType::TENSOR_INT32} tensor defining the traversal order for reading
     *      the non-zero blocks. For an n-dimensional tensor with dimensions [D0, D1, …, Dn-1]: if
     *      block sparse with a k-dimensional block (0 < k <= n), the traversal order has n+k
     *      elements. The first n elements are still a permutation of [D0, …, Dn-1]. The last k
     *      elements are a permutation of [Dn, …, Dn+k-1], defining how to traverse a block
     *      internally. If not block sparse, the traversal order is just a permutation of [D0, …,
     *      Dn-1].
     * * 2: An optional 1-D {@link OperandType::TENSOR_INT32} tensor defining the block map. For a
     *      block sparse n-dimensional tensor with a k-dimensional block (0 < k <= n), it stores how
     *      a block dimension [Dn, …, Dn+k-1] maps to the original tensor dimension in [D0, …,
     *      Dn-1]. For i, j where 0 <= i < j < k, blockMap[i] < blockMap[j]. If not block sparse,
     *      this is null.
     * * 3: A 1-D {@link OperandType::TENSOR_INT32} tensor with n+k elements defining the format of
     *      each dimension in the traversal order (listed above). The format is either DENSE (where
     *      DENSE = 0) or SPARSE_CSR (where SPARSE_CSR = 1). DENSE means that each coordinate in
     *      this dimension is stored implicitly. SPARSE_CSR means only the coordinates with non-zero
     *      elements are stored.
     * * 4: A 1-D {@link OperandType::TENSOR_INT32} tensor with n+k elements defining the size of
     *      each dimension or block. The product of all these sizes totals the number of elements in
     *      the dense tensor. First n elements represent the sparse tensor’s shape, and the last k
     *      elements represent the block’s shape.
     * * 5 ~ (5 + 2 * (n+k)): An optional pair of {@link OperandType::TENSOR_INT32} tensors which
     *      together specify the sparse indices along that dimension. The first pair of arguments
     *      corresponds to D0, the second to D1, and so on until Dn+k-1. If the dimension is DENSE,
     *      both arguments in the pair are null and the dimension is implicitly specified by the
     *      corresponding element in Input 4. If the dimension is SPARSE_CSR, then we use the pair
     *      of array segments and array indices to encode that dimension:
     * * * +0: An optional list of n+k input 1-D {@link OperandType::TENSOR_INT32} tensors, defining
     *         the array segments. The array segments represent how to segment the indices array,
     *         each segment corresponds to one element in the previous dimension. Array segments are
     *         interspersed with array indices (listed below), so this input could be input (5, 5 +
     *         2, …, 5 + 2*(n+k-1)). For i, j where 0 =< i < j, arraySegments[i] <=
     *         arraySegments[j]. Used if the dimension is SPARSE_CSR, omitted if the dimension is
     *         DENSE.
     * * * +1: An optional list of n+k input 1-D {@link OperandType::TENSOR_INT32} tensors, defining
     *         the array indices. The array indices represent the index of the non-zero elements
     *         within this dimension (as those in the CSR matrix format, where the first array is
     *         row pointers and the second array is column indices). Array indices are interspersed
     *         with array segments (listed above), so this input could be input (6, 6 + 2, …, 6 +
     *         2*(n+k-1)). Used if the dimension is SPARSE_CSR, omitted if the dimension is DENSE.
     *
     * Outputs:
     * * 0: An n-D dense tensor. The output tensor has the same {@link OperandType} as input 0.
     */
    DENSIFY = 20000,
#endif  // NN_EXPERIMENTAL_FEATURE
};

}  // namespace android::nn

#endif  // ANDROID_PACKAGES_MODULES_NEURALNETWORKS_COMMON_NNAPI_OPERATION_TYPES_H