#include "abstract_expression_utils.h"

#include "abstract_expression.h"
#include "logic_expression.h"

namespace DaseX {

void AbstractExpressionUtils::EnumerateChildren(
	AbstractExpressionRef &expr,
	const std::function<void(AbstractExpressionRef &child)> &callback) {
	for (AbstractExpressionRef &child : expr->children_) {
		callback(child);
	}
}

auto AbstractExpressionUtils::SplitConjunctionPredicate(
	const AbstractExpressionRef &expr,
	std::vector<AbstractExpressionRef> &split_exprs) -> bool {
	if (expr->GetExpressionClass() == ExpressionClass::CONJUNCTION) {
		auto logicExpr = reinterpret_cast<LogicExpression &>(*expr);
		assert(logicExpr.logic_type_ == LogicType::And);
		std::vector<AbstractExpressionRef> splited_exprs;
		for (size_t i = 0; i < expr->children_.size(); i++) {
			SplitConjunctionPredicate(expr->children_[i], split_exprs);
		}
		return true;
	}
	split_exprs.push_back(expr);
	return false;
}

}  // namespace DaseX
