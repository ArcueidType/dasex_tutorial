#include "optimizer.hpp"

#include "abstract_plan.h"
#include "binder.h"
#include "filter_pushdown.hpp"

namespace DaseX {

Optimizer::Optimizer(Binder &binder) : binder(binder) {}

auto Optimizer::Optimize(AbstractPlanNodeRef plan_p) -> AbstractPlanNodeRef {
	this->plan_ = std::move(plan_p);
	RunOptimizers();
	return std::move(this->plan_);
}

void Optimizer::RunOptimizers() {
	switch (plan_->GetType()) {
		case PlanType::Projection:
		case PlanType::Filter:
		case PlanType::NestedLoopJoin:
		case PlanType::SeqScan:
		case PlanType::Aggregation:
		case PlanType::Sort:
			break;
		default:
			throw DaseX::Exception(
				"Optimization not supported for plan type {} ");
	}

	RunOptimizer(OptimizerType::FILTER_PUSHDOWN, [&]() {
		FilterPushdown filter_pushdown(*this);
		plan_ = filter_pushdown.Rewrite(std::move(plan_));
	});
}

void Optimizer::RunOptimizer(OptimizerType type,
							 const std::function<void()> &callback) {
	callback();
}

}  // namespace DaseX