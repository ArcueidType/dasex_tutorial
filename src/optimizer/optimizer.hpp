#pragma once

#include "abstract_plan.h"
#include "binder.h"

namespace DaseX {

enum class OptimizerType : uint32_t {
	INVALID = 0,
	EXPRESSION_REWRITER,
	FILTER_PULLUP,
	FILTER_PUSHDOWN,
	CTE_FILTER_PUSHER,
	REGEX_RANGE,
	IN_CLAUSE,
	JOIN_ORDER,
	DELIMINATOR,
	UNNEST_REWRITER,
	UNUSED_COLUMNS,
	STATISTICS_PROPAGATION,
	COMMON_SUBEXPRESSIONS,
	COMMON_AGGREGATE,
	COLUMN_LIFETIME,
	BUILD_SIDE_PROBE_SIDE,
	LIMIT_PUSHDOWN,
	TOP_N,
	COMPRESSED_MATERIALIZATION,
	DUPLICATE_GROUPS,
	REORDER_FILTER,
	JOIN_FILTER_PUSHDOWN,
	EXTENSION
};

class Optimizer {
   public:
	Optimizer(Binder &binder);

	auto Optimize(AbstractPlanNodeRef plan_p) -> AbstractPlanNodeRef;

   public:
	Binder &binder;

   private:
	// void Verify(AbstractPlanNodeRef plan_p);
	void RunOptimizers();
	void RunOptimizer(OptimizerType type,
					  const std::function<void()> &callback);

   private:
	AbstractPlanNodeRef plan_;
};

}  // namespace DaseX