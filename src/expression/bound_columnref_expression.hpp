#pragma once

#include "expression.hpp"
#include "expression_type.hpp"
#include "logical_type.hpp"
#include <string>

namespace DaseX
{
//! A BoundColumnRef expression represents a ColumnRef expression that was bound to an actual table and column index. It
//! is not yet executable, however. The ColumnBindingResolver transforms the BoundColumnRefExpressions into
//! BoundExpressions, which refer to indexes into the physical chunks that pass through the executor.
class BoundColumnRefExpression : public Expression
{
public:
    static constexpr const ExpressionClass TYPE = ExpressionClass::BOUND_COLUMN_REF;
    int table_index; // 这里也可以换成表名
    int column_index;
    //! The subquery depth (i.e. depth 0 = current query, depth 1 = parent query, depth 2 = parent of parent, etc...).
    //! This is only non-zero for correlated expressions inside subqueries.
    int depth;

public:
    BoundColumnRefExpression(LogicalType type,
                             int table_index,
                             int column_index,
                             int depth = 0) : BoundColumnRefExpression(std::string(),
                                                                       std::move(type),
                                                                       table_index,
                                                                       column_index, depth) {}
    BoundColumnRefExpression(std::string alias,
                             LogicalType type,
                             int table_index,
                             int column_index,
                             int depth = 0) : Expression(ExpressionTypes::BOUND_COLUMN_REF, ExpressionClass::BOUND_COLUMN_REF, std::move(type)),
                                              table_index(table_index),
                                              column_index(column_index)
    {
        this->alias = std::move(alias);
    }

    std::shared_ptr<Expression> Copy() override {
        auto expr = std::make_shared<BoundColumnRefExpression>(this->return_type, this->table_index,this->column_index, this->depth);
        expr->alias = alias;
        return expr;
    }
};



} // DaseX
