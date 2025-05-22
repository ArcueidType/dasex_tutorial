
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "abstract_expression.h"
#include "catalog.hpp"
#include "logical_type.hpp"

namespace DaseX {
/**
 * ColumnValueExpression maintains the tuple index and column index relative to
 * a particular schema or join.
 */
class ColumnValueExpression : public AbstractExpression {
public:
  /**
   * ColumnValueExpression is an abstraction around "Table.member" in terms of
   * indexes.
   * @param tuple_idx {tuple index 0 = left side of join, tuple index 1 = right
   * side of join}
   * @param col_idx the index of the column in the schema
   * @param ret_type the return type of the expression
   */
  ColumnValueExpression(uint32_t tuple_idx, uint32_t col_idx,
                        LogicalType ret_type, uint32_t depth = 0)
      : AbstractExpression({}, ret_type,ExpressionClass::COLUMN_REF), tuple_idx_{tuple_idx},
        col_idx_{col_idx}, depth_(depth) {}

    static constexpr const ExpressionClass TYPE = ExpressionClass::COLUMN_REF;
  auto GetTupleIdx() const -> uint32_t { return tuple_idx_; }
  auto GetColIdx() const -> uint32_t { return col_idx_; }
  [[nodiscard]] auto ToString() const -> std::string {
    return "T" + std::to_string(tuple_idx_) + "C" + std::to_string(col_idx_);
  }
  EXPR_CLONE_WITH_CHILDREN(ColumnValueExpression);

private:
  /** left side is 0, right side is 1 */
  uint32_t tuple_idx_;
  /** column index in the output_schema, used in translate between operators,
   * e.g. schema {A,B,C} has indexes {0,1,2} */
  uint32_t col_idx_;

  // depth 定义参考 duckdb 的 BoundColumnRefExpression
  //! The subquery depth (i.e. depth 0 = current query, depth 1 = parent query, depth 2 = parent of parent, etc...).
	//! This is only non-zero for correlated expressions inside subqueries.
  uint32_t depth_;
};
} // namespace DaseX
