/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-25 14:54:01
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2023-12-25 14:54:02
 * @FilePath: /task_sche-binder/src/binder/bound_expression.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include <memory>
#include <string>

namespace DaseX {

/**
 * All types of expressions in binder.
 */
enum class ExpressionType : uint8_t {
  INVALID = 0,    /**< Invalid expression type. */
  CONSTANT = 1,   /**< Constant expression type. */
  COLUMN_REF = 3, /**< A column in a table. */
  TYPE_CAST = 4,  /**< Type cast expression type. */
  FUNCTION = 5,   /**< Function expression type. */
  AGG_CALL = 6,   /**< Aggregation function expression type. */
  STAR = 7,     /**< Star expression type, will be rewritten by binder and won't
                   appear in plan. */
  UNARY_OP = 8, /**< Unary expression type. */
  BINARY_OP = 9,  /**< Binary expression type. */
  ALIAS = 10,     /**< Alias expression type. */
  FUNC_CALL = 11, /**< Function call expression type. */
  WINDOW = 12,    /**< Window Aggregation expression type. */

  SUBQUERY = 13,
  CASE = 14,
};

class BoundExpression {
public:
  explicit BoundExpression(ExpressionType type) : type_(type) {}
  BoundExpression() = default;
  virtual ~BoundExpression() = default;

  virtual auto ToString() const -> std::string { return ""; };

  auto IsInvalid() const -> bool { return type_ == ExpressionType::INVALID; }

  virtual auto HasWindowFunction() const -> bool { return false; }

  virtual auto HasAggregation() const -> bool { return false; }

  ExpressionType type_{ExpressionType::INVALID};
};

} // namespace DaseX
