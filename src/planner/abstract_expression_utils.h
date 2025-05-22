#pragma once

#include <functional>

#include "abstract_expression.h"

namespace DaseX {

class AbstractExpressionUtils {
   public:
	static void EnumerateChildren(
		AbstractExpressionRef &expr,
		const std::function<void(AbstractExpressionRef &child)> &callback);
	// Split up the filters by AND predicate
	// Return true if expr is LogicExpression, else false
	static auto SplitConjunctionPredicate(
		const AbstractExpressionRef &expr,
		std::vector<AbstractExpressionRef> &split_exprs) -> bool;
};

}  // namespace DaseX