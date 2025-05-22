/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-25 14:29:39
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-01-03 19:46:08
 * @FilePath: /task_sche-binder/src/binder/bound_order_by.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#pragma once

#include <memory>
#include <string>
#include <utility>

#include "bound_expression.h"
#include "exception.h"

namespace DaseX {

/**
 * All types of order-bys in binder.
 */
enum class OrderByType : uint8_t {
  INVALID = 0,
  DEFAULT = 1,
  ASC = 2,  /** Asc order by type. */
  DESC = 3, /** Desc order by type. */
};

/**
 * BoundOrderBy is an item in the ORDER BY clause.
 */
class BoundOrderBy {
public:
  explicit BoundOrderBy(OrderByType type, std::unique_ptr<BoundExpression> expr)
      : type_(type), expr_(std::move(expr)) {}

  OrderByType type_;

  /** The order by expression */
  std::unique_ptr<BoundExpression> expr_;
};

} // namespace DaseX
