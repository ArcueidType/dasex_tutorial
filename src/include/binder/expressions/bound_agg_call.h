/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-27 18:48:40
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-01-03 13:06:07
 * @FilePath: /task_sche-binder/src/binder/bound_agg_call.h
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
 * A bound aggregate call, e.g., `sum(x)`.
 */
class BoundAggCall : public BoundExpression {
public:
  explicit BoundAggCall(std::string func_name, bool is_distinct,
                        std::vector<std::unique_ptr<BoundExpression>> args)
      : BoundExpression(ExpressionType::AGG_CALL),
        func_name_(std::move(func_name)), is_distinct_(is_distinct),
        args_(std::move(args)) {}

  auto HasAggregation() const -> bool override { return true; }

  std::string func_name_;

  bool is_distinct_;

  std::vector<std::unique_ptr<BoundExpression>> args_;
};
} // namespace DaseX
