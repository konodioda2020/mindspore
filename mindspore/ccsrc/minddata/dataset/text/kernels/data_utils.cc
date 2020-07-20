/**
 * Copyright 2020 Huawei Technologies Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "minddata/dataset/text/kernels/data_utils.h"

#include <algorithm>
#include <limits>
#include <string>
#include <vector>

#include "minddata/dataset/core/pybind_support.h"
#include "minddata/dataset/kernels/data/type_cast_op.h"
#include "minddata/dataset/kernels/data/slice_op.h"
#include "minddata/dataset/kernels/data/concatenate_op.h"

namespace mindspore {
namespace dataset {
Status SlidingWindowHelper(const std::shared_ptr<Tensor> &input, std::shared_ptr<Tensor> *output, TensorShape out_shape,
                           uint32_t width, int32_t axis) {
  // if the data row has fewer items than width, the corresponding result row will be empty
  if (out_shape.Size() == 0) {
    MS_LOG(WARNING) << "The data row has fewer items than width, the result will be empty.";
    if (input->type().value() == DataType::DE_STRING) {
      RETURN_IF_NOT_OK(Tensor::CreateTensor(output, std::vector<std::string>{}, TensorShape({0})));
    } else {
      RETURN_IF_NOT_OK(Tensor::CreateTensor(output, TensorImpl::kFlexible, TensorShape({0}), input->type()));
    }
    return Status::OK();
  }

  axis = Tensor::HandleNeg(axis, input->shape().Size());
  int32_t axis_end = input->shape()[axis];
  std::shared_ptr<Tensor> tmp;
  auto concatenate_op = std::make_unique<ConcatenateOp>(axis, nullptr, nullptr);

  // Slice on specified axis and concatenate on new axis
  for (int32_t i = 0; i + width <= axis_end; i++) {
    auto slice_op = std::make_unique<SliceOp>(Slice(i, i + width, 1));
    slice_op->Compute(input, &tmp);
    if (i == 0) {
      *output = tmp;
    } else {
      TensorRow in({*output, tmp});
      TensorRow out_row;
      concatenate_op->Compute(in, &out_row);
      *output = out_row[0];
    }
  }
  (*output)->Reshape(out_shape);
  return Status::OK();
}
}  // namespace dataset
}  // namespace mindspore