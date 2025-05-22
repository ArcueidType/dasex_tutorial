/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-25 14:53:03
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2023-12-25 14:53:04
 * @FilePath: /task_sche-binder/src/binder/bound_alias.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include "../bound_expression.h"
#include <memory>
#include <string>
#include <utility>

namespace DaseX {

/**
 * The alias in SELECT list, e.g. `SELECT count(x) AS y`, the `y` is an alias.
 */
class BoundAlias : public BoundExpression {
public:
  explicit BoundAlias(std::string alias, std::unique_ptr<BoundExpression> child)
      : BoundExpression(ExpressionType::ALIAS), alias_(std::move(alias)),
        child_(std::move(child)) {}

  auto HasAggregation() const -> bool override {
    return child_->HasAggregation();
  }

  std::string alias_;

  std::unique_ptr<BoundExpression> child_;
};
} // namespace DaseX
