/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-25 14:56:26
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2023-12-25 14:56:27
 * @FilePath: /task_sche-binder/src/binder/bound_constant.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置:
 * https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AEint
 */
#pragma once

#include <string>
#include <utility>

#include "../bound_expression.h"
#include "value_type.hpp"

namespace DaseX {

class BoundExpression;

/**
 * A bound constant, e.g., `1`.
 */
class BoundConstant : public BoundExpression {
public:
  explicit BoundConstant(const Value &val)
      : BoundExpression(ExpressionType::CONSTANT), val_(val) {}

  auto HasAggregation() const -> bool override { return false; }

  // Value 的 string 处理是否要修改
  Value val_;
};
} // namespace DaseX
