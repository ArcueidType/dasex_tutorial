//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/planner/expression/bound_subquery_expression.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "abstract_expression.h"
#include "abstract_plan.h"
#include "bound_subquery.h"
#include "expression_type.hpp"
#include "logical_type.hpp"

namespace DaseX {

class SubqueryExpression : public AbstractExpression {
public:

	SubqueryExpression(LogicalType return_type)
    : AbstractExpression({}, return_type, ExpressionClass::SUBQUERY) {}

    static constexpr const ExpressionClass TYPE = ExpressionClass::SUBQUERY;

    EXPR_CLONE_WITH_CHILDREN(SubqueryExpression);

    AbstractPlanNodeRef subquery;
    //! The subquery type
	SubqueryType subquery_type;
	//! the child expression to compare with (in case of IN, ANY, ALL operators)
	std::shared_ptr<AbstractExpression> child;
	//! The comparison type of the child expression with the subquery (in case of ANY, ALL operators)
	ComparisonType comparison_type;
	//! The LogicalType of the subquery result. Only used for ANY expressions.
	LogicalType child_type;
	//! The target LogicalType of the subquery result (i.e. to which type it should be casted, if child_type <>
	//! child_target). Only used for ANY expressions.
	// LogicalType child_target;

// 	bool IsCorrelated() const {
// 		return !binder->correlated_columns.empty();
// 	}

// 	//! The binder used to bind the subquery node
// 	shared_ptr<Binder> binder;
// 	//! The bound subquery node
// 	unique_ptr<BoundQueryNode> subquery;
// 	//! The subquery type
// 	SubqueryType subquery_type;
// 	//! the child expression to compare with (in case of IN, ANY, ALL operators)
// 	unique_ptr<Expression> child;
// 	//! The comparison type of the child expression with the subquery (in case of ANY, ALL operators)
// 	ExpressionType comparison_type;
// 	//! The LogicalType of the subquery result. Only used for ANY expressions.
// 	LogicalType child_type;
// 	//! The target LogicalType of the subquery result (i.e. to which type it should be casted, if child_type <>
// 	//! child_target). Only used for ANY expressions.
// 	LogicalType child_target;

// public:
// 	bool HasSubquery() const override {
// 		return true;
// 	}
// 	bool IsScalar() const override {
// 		return false;
// 	}
// 	bool IsFoldable() const override {
// 		return false;
// 	}

// 	string ToString() const override;

// 	bool Equals(const BaseExpression &other) const override;

// 	unique_ptr<Expression> Copy() const override;

// 	bool PropagatesNullValues() const override;

};

} // namespace DaseX
