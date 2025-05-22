#pragma once

#include "abstract_expression.h"
#include "abstract_plan.h"
#include "expression.hpp"
#include "optimizer.hpp"

namespace DaseX {

class FilterPushdown {
   public:
	explicit FilterPushdown(Optimizer &optimizer);
	auto Rewrite(AbstractPlanNodeRef plan) -> AbstractPlanNodeRef;

   private:
	auto PushdownFilter(AbstractPlanNodeRef plan) -> AbstractPlanNodeRef;
	auto PushdownProjection(AbstractPlanNodeRef plan) -> AbstractPlanNodeRef;
	auto PushdownNestedLoopJoin(AbstractPlanNodeRef plan)
		-> AbstractPlanNodeRef;
	auto PushdownSeqScan(AbstractPlanNodeRef plan) -> AbstractPlanNodeRef;
	auto PushdownAggregate(AbstractPlanNodeRef plan) -> AbstractPlanNodeRef;
	auto AddFilter(const AbstractExpressionRef &expr) -> bool;
	auto GenerateFilterExpression(
		std::vector<AbstractExpressionRef> &expressions)
		-> AbstractExpressionRef;

   private:
	// no logical expression in filters_
	std::vector<AbstractExpressionRef> filters_;
	Optimizer &optimizer;
};

}  // namespace DaseX