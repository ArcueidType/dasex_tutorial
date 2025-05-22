//
// Created by zyy on 25-3-5.
//

#ifndef DASEX_LLM_EXPRESSION_H
#define DASEX_LLM_EXPRESSION_H

#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>
#include "catalog.hpp"
#include "abstract_expression.h"
#include "value_type.hpp"

namespace DaseX {

enum class LlmType {
	Decide,
};

class LlmExpression : public AbstractExpression {
   public:
	LlmExpression(AbstractExpressionRef prompt, AbstractExpressionRef left, AbstractExpressionRef right, LlmType compute_type)
		: AbstractExpression({std::move(prompt), std::move(left), std::move(right)}, LogicalType::STRING,
							 ExpressionClass::LLM) {

	}

	EXPR_CLONE_WITH_CHILDREN(LlmExpression);


};
}
#endif	// DASEX_LLM_EXPRESSION_H
