/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-27 18:49:34
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2023-12-28 16:48:34
 * @FilePath: /task_sche-binder/src/binder/bound_binary_op.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../bound_expression.h"

namespace DaseX {

/**
 * A bound binary operator, e.g., `a+b`.
 */
class BoundBinaryOp : public BoundExpression {
public:
  explicit BoundBinaryOp(std::string op_name,
                         std::unique_ptr<BoundExpression> larg,
                         std::unique_ptr<BoundExpression> rarg)
      : BoundExpression(ExpressionType::BINARY_OP),
        op_name_(std::move(op_name)), larg_(std::move(larg)),
        rarg_(std::move(rarg)) {}

  auto HasAggregation() const -> bool override {
    return larg_->HasAggregation() || rarg_->HasAggregation();
  }

  std::string op_name_;

  std::unique_ptr<BoundExpression> larg_;

  std::unique_ptr<BoundExpression> rarg_;
};
} // namespace DaseX
