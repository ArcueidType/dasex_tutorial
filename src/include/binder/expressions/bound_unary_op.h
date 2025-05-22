/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2024-01-04 15:55:43
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-01-04 15:55:44
 * @FilePath: /task_sche-binder/src/binder/bound_unary_op.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#pragma once

#include "../bound_expression.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace DaseX {

/**
 * A bound unary operation, e.g., `-x`.
 */
class BoundUnaryOp : public BoundExpression {
public:
  explicit BoundUnaryOp(std::string op_name,
                        std::unique_ptr<BoundExpression> arg)
      : BoundExpression(ExpressionType::UNARY_OP), op_name_(std::move(op_name)),
        arg_(std::move(arg)) {}

  auto HasAggregation() const -> bool override {
    return arg_->HasAggregation();
  }

  std::string op_name_;

  std::unique_ptr<BoundExpression> arg_;
};

} // namespace DaseX
