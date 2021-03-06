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

#ifndef MINDSPORE_CCSRC_FRONTEND_OPTIMIZER_IRPASS_SPARSE_TENSOR_ELIMINATE_H_
#define MINDSPORE_CCSRC_FRONTEND_OPTIMIZER_IRPASS_SPARSE_TENSOR_ELIMINATE_H_

#include <vector>
#include <algorithm>

#include "frontend/optimizer/irpass.h"
#include "frontend/optimizer/optimizer.h"
#include "ir/visitor.h"
#include "frontend/operator/ops.h"

namespace mindspore {
namespace opt {
namespace irpass {
// {prim::kPrimSparseTensorGetIndices, {prim::kPrimMakeSparseTensor, Xs}}
// {prim::kPrimSparseTensorGetValues, {prim::kPrimMakeSparseTensor, Xs}}
// {prim::kPrimSparseTensorGetDenseShape, {prim::kPrimMakeSparseTensor, Xs}}
class SparseTensorEliminater : public AnfVisitor {
 public:
  AnfNodePtr operator()(const OptimizerPtr &, const AnfNodePtr &node) override {
    Reset();
    AnfVisitor::Match(prim::kPrimSparseTensorGetIndices, {IsCNode})(node);

    if (is_match_) {
      return tuple_->input(1);
    }
    AnfVisitor::Match(prim::kPrimSparseTensorGetValues, {IsCNode})(node);

    if (is_match_) {
      return tuple_->input(2);
    }
    AnfVisitor::Match(prim::kPrimSparseTensorGetDenseShape, {IsCNode})(node);

    if (is_match_) {
      return tuple_->input(3);
    }
    return nullptr;
  }

  void Visit(const CNodePtr &cnode) override {
    if (IsPrimitiveCNode(cnode, prim::kPrimMakeSparseTensor)) {
      tuple_ = cnode;
      is_match_ = true;
    }
  }

  void Reset() {
    tuple_ = nullptr;
    is_match_ = false;
  }

 private:
  bool is_match_{false};
  CNodePtr tuple_{nullptr};
};
}  // namespace irpass
}  // namespace opt
}  // namespace mindspore
#endif  // MINDSPORE_CCSRC_FRONTEND_OPTIMIZER_IRPASS_SPARSE_TENSOR_ELIMINATE_H_
