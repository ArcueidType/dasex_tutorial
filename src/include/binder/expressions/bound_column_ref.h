/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-14 13:22:18
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-01-03 20:00:10
 * @FilePath: /task_sche/src/binder/bound_column_ref.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include "../bound_expression.h"
#include "spdlog/fmt/bundled/format.h"
#include <algorithm>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace DaseX {

/**
 * A bound column reference, e.g., `y.x` in the SELECT list.
 */
class BoundColumnRef : public BoundExpression {
public:
  explicit BoundColumnRef(std::vector<std::string> col_name)
      : BoundExpression(ExpressionType::COLUMN_REF),
        col_name_(std::move(col_name)) {}

  static auto Prepend(std::unique_ptr<BoundColumnRef> self,
                      std::string prefix) -> std::unique_ptr<BoundColumnRef> {
    if (self == nullptr) {
      return nullptr;
    }
    std::vector<std::string> col_name{std::move(prefix)};
    std::copy(self->col_name_.cbegin(), self->col_name_.cend(),
              std::back_inserter(col_name));
    return std::make_unique<BoundColumnRef>(std::move(col_name));
  }
  auto ToString() const -> std::string override {
    return fmt::format("{}", fmt::join(col_name_, "."));
  }

    auto HasAggregation() const -> bool override { return false; }
  std::vector<std::string> col_name_;

  std::vector<std::string> table_name_;
};
} // namespace DaseX
