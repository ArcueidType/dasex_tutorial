//
// Created by zyy on 24-11-25.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "abstract_expression.h"
#include "logical_type.hpp"
#include "value_type.hpp"

namespace DaseX {



class CaseExpression : public AbstractExpression {
   public:


	CaseExpression(std::vector<AbstractExpressionRef> when_expr_, std::vector<AbstractExpressionRef> then_expr_,
				   AbstractExpressionRef else_expr_) : AbstractExpression({}, LogicalType::FLOAT,
																					   ExpressionClass::CASE),when_expr(when_expr_),then_expr(then_expr_),else_expr(else_expr_) { }

	std::vector<AbstractExpressionRef> when_expr;
	std::vector<AbstractExpressionRef> then_expr;
	AbstractExpressionRef else_expr;

	EXPR_CLONE_WITH_CHILDREN(CaseExpression);



};
} // namespace DaseX
