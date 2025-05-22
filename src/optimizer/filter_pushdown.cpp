#include "filter_pushdown.hpp"

#include <memory>
#include <unordered_set>

#include "abstract_expression.h"
#include "abstract_expression_utils.h"
#include "abstract_plan.h"
#include "agg_plan.h"
#include "column_value_expression.h"
#include "compare_expression.h"
#include "constant_value_expression.h"
#include "exception.h"
#include "expression.hpp"
#include "expression_type.hpp"
#include "filter_plan.h"
#include "logic_expression.h"
#include "nest_loop_join_plan.h"
#include "optimizer.hpp"
#include "projection_plan.h"
#include "seq_scan_plan.h"
#include "typedefs.hpp"

namespace DaseX {

/// @todo: join side clean up
class JoinSide {
   public:
	enum JoinValue : uint8_t { NONE, LEFT, RIGHT, BOTH };

	JoinSide() = default;
	constexpr JoinSide(JoinValue val)
		: value(val) {	// NOLINT: Allow implicit conversion from `join_value`
	}
	auto operator==(JoinSide rhs) const -> bool { return value == rhs.value; }
	auto operator!=(JoinSide rhs) const -> bool { return value != rhs.value; }
	static auto CombineJoinSide(JoinSide left, JoinSide right) -> JoinSide {
		if (left == JoinSide::NONE) {
			return right;
		}
		if (right == JoinSide::NONE) {
			return left;
		}
		if (left != right) {
			return JoinSide::BOTH;
		}
		return left;
	}

   private:
	JoinValue value;
};

static auto GetJoinSide(NestedLoopJoinPlanNode &join,
						AbstractExpressionRef expr) -> JoinSide {
	if (expr->GetExpressionClass() == ExpressionClass::COLUMN_REF) {
		auto &colref = expr->Cast<ColumnValueExpression>();
		size_t leftFieldSize =
			join.GetLeftPlan()->OutputSchema().fields().size();
		return colref.GetColIdx() < leftFieldSize ? JoinSide::LEFT
												  : JoinSide::RIGHT;
	}
	JoinSide side = JoinSide::NONE;
	AbstractExpressionUtils::EnumerateChildren(
		expr, [&](AbstractExpressionRef &child) {
			side = JoinSide::CombineJoinSide(side, GetJoinSide(join, child));
		});
	return side;
}

static auto ReplaceProjectionBindings(ProjectionPlanNode &proj,
									  AbstractExpressionRef expr)
	-> AbstractExpressionRef {
	if (expr->GetExpressionClass() == ExpressionClass::COLUMN_REF) {
		auto &colref = expr->Cast<ColumnValueExpression>();
		assert(colref.GetColIdx() < proj.expressions_.size());
		// replace the binding with a copy to the expression at the referenced
		// index
		/// @fix: deep copy or ref copy
		return proj.expressions_[colref.GetColIdx()];
	}
	AbstractExpressionUtils::EnumerateChildren(
		expr, [&](AbstractExpressionRef &child) {
			child = ReplaceProjectionBindings(proj, std::move(child));
		});
	return expr;
}

static auto ReplaceJoinBindings(NestedLoopJoinPlanNode &join,
								AbstractExpressionRef expr,
								bool mark_tuple = false)
	-> AbstractExpressionRef {
	if (expr->GetExpressionClass() == ExpressionClass::COLUMN_REF) {
		auto &colref = expr->Cast<ColumnValueExpression>();
		int left_size = join.GetLeftPlan()->OutputSchema().num_fields();
		uint32_t col_idx = colref.GetColIdx();
		bool in_right = false;
		if (col_idx >= left_size) {
			in_right = true;
			col_idx -= left_size;
		}
		uint32_t tuple_idx = (mark_tuple && in_right) ? 1 : 0;
		return std::make_shared<ColumnValueExpression>(
			tuple_idx ? 1 : 0, col_idx, colref.GetReturnType());
	}
	AbstractExpressionUtils::EnumerateChildren(
		expr, [&](AbstractExpressionRef &child) {
			child = ReplaceJoinBindings(join, std::move(child), mark_tuple);
		});
	return expr;
}

// static auto ReplaceNestedLoopJoinBindings(NestedLoopJoinPlanNode &join,
// 									  	  AbstractExpressionRef expr)
// 	-> AbstractExpressionRef {
// 	if (expr->GetExpressionClass() == ExpressionClass::COLUMN_REF) {
// 		auto &colref = reinterpret_cast<ColumnValueExpression &>(*expr);
// 		assert(colref.GetColIdx() < join.size());
// 		// replace the binding with a copy to the expression at the referenced
// 		// index
// 		/// @fix: deep copy or ref copy
// 		return prjoinoj.expressions_[colref.GetColIdx()];
// 	}
// 	AbstractExpressionUtils::EnumerateChildren(
// 		expr, [&](AbstractExpressionRef &child) {
// 			child = ReplaceProjectionBindings(proj, std::move(child));
// 		});
// 	return expr;
// }

FilterPushdown::FilterPushdown(Optimizer &optimizer) : optimizer(optimizer) {}

auto FilterPushdown::Rewrite(AbstractPlanNodeRef plan) -> AbstractPlanNodeRef {
	switch (plan->GetType()) {
		case PlanType::Projection:
			return PushdownProjection(std::move(plan));
		case PlanType::Filter:
			return PushdownFilter(std::move(plan));
		case PlanType::NestedLoopJoin:
			return PushdownNestedLoopJoin(std::move(plan));
		case PlanType::SeqScan:
			return PushdownSeqScan(std::move(plan));
		case PlanType::Aggregation:
			return PushdownAggregate(std::move(plan));
		case PlanType::Sort:
			// we can just push directly through these operations without any
			// rewriting
			plan->children_[0] = Rewrite(std::move(plan->children_[0]));
			return plan;
		default:
			throw DaseX::Exception("Unimplemented plan type!");
	}
}

auto FilterPushdown::PushdownFilter(AbstractPlanNodeRef plan)
	-> AbstractPlanNodeRef {
	assert(plan->GetType() == PlanType::Filter);
	auto &filter = plan->Cast<FilterPlanNode>();
	// filter: gather the filters and remove the filter from the set of
	if (!AddFilter(filter.predicate_)) {
		throw DaseX::Exception("Unexpected!");
	}
	return Rewrite(std::move(plan->children_[0]));
}

auto FilterPushdown::PushdownProjection(AbstractPlanNodeRef plan)
	-> AbstractPlanNodeRef {
	assert(plan->GetType() == PlanType::Projection);
	auto &proj = plan->Cast<ProjectionPlanNode>();

	FilterPushdown child_pushdown(optimizer);
	std::vector<AbstractExpressionRef> remain_expressions;
	for (auto &filter : filters_) {
		if (!child_pushdown.AddFilter(filter)) {
			remain_expressions.push_back(std::move(filter));
		}
		// rebind with proj->expressions_
		filter = ReplaceProjectionBindings(proj, std::move(filter));
	}
	/// @todo: volatile expression should not be pushdown (figure out any cases)
	if (!remain_expressions.empty()) {
		throw DaseX::Exception("Unexpected!");
	}
	plan->children_[0] = child_pushdown.Rewrite(std::move(plan->children_[0]));
	return plan;
}

auto FilterPushdown::PushdownNestedLoopJoin(AbstractPlanNodeRef plan)
	-> AbstractPlanNodeRef {
	assert(plan->GetType() == PlanType::NestedLoopJoin);
	auto &join = plan->Cast<NestedLoopJoinPlanNode>();
	std::vector<AbstractExpressionRef> join_expressions;
	std::vector<AbstractExpressionRef> remained_filters;
	FilterPushdown left_pushdown(optimizer);
	FilterPushdown right_pushdown(optimizer);

	for (auto &filter : filters_) {
		JoinSide side = GetJoinSide(join, filter);
		if (filter->GetExpressionClass() != ExpressionClass::COMPARISON ||
			side == JoinSide::NONE) {
			remained_filters.push_back(filter);
			continue;
		}
		// assert(filter->GetExpressionClass() == ExpressionClass::COMPARISON);
		if (side == JoinSide::LEFT) {
			ReplaceJoinBindings(join, filter);
			left_pushdown.AddFilter(filter);
		} else if (side == JoinSide::RIGHT) {
			ReplaceJoinBindings(join, filter);
			right_pushdown.AddFilter(filter);
		} else {
			// This filter involves both sides
			ReplaceJoinBindings(join, filter, true);
			join_expressions.push_back(filter);
		}
	}
	plan->children_[0] = left_pushdown.Rewrite(std::move(plan->children_[0]));
	plan->children_[1] = right_pushdown.Rewrite(std::move(plan->children_[1]));
	// 把同时涉及两边的条件推到 join predicate 上
	/// @TODO: 确保这里只有一个等式表达式（用于哈希连接），其他的放在 Filter 里
	if (!join_expressions.empty()) {
		// 对笛卡尔积特殊处理，检查 predicate 只允许是 1=1，如果不是则抛异常
		if (join.predicate_) {
			bool cartesian_product = false;
			if (join.predicate_->GetExpressionClass() ==
				ExpressionClass::COMPARISON) {
				auto &comp_expr =
					reinterpret_cast<ComparisonExpression &>(*join.predicate_);
				if (comp_expr.children_[0]->GetExpressionClass() ==
						ExpressionClass::CONSTANT &&
					comp_expr.children_[1]->GetExpressionClass() ==
						ExpressionClass::CONSTANT) {
					auto &left_const =
						reinterpret_cast<ConstantValueExpression &>(
							*comp_expr.children_[0]);
					auto &right_const =
						reinterpret_cast<ConstantValueExpression &>(
							*comp_expr.children_[1]);
					if (left_const.val_ == right_const.val_) {
						cartesian_product = true;
					}
				}
			}
			if (!cartesian_product) {
				throw Exception(
					"FilterPushdown::PushdownNestedLoopJoin already has a "
					"predicate for nested loop join");
			}
		}
		// 只保留第一个 predicate
		join.predicate_ = std::move(join_expressions[0]);
		for (size_t i = 1; i < join_expressions.size(); i++)
		{
			remained_filters.push_back(std::move(join_expressions[i]));
		}
	}
	if (!remained_filters.empty()) {
		AbstractExpressionRef filterExpr =
			GenerateFilterExpression(remained_filters);
		auto filterPlan = std::make_shared<FilterPlanNode>(plan->output_schema_,
														   filterExpr, plan);
		return filterPlan;
	}
	return plan;
}

auto FilterPushdown::PushdownSeqScan(AbstractPlanNodeRef plan)
	-> AbstractPlanNodeRef {
	assert(plan->GetType() == PlanType::SeqScan);
	if (!filters_.empty()) {
		auto &seqScan = plan->Cast<SeqScanPlanNode>();
		seqScan.filter_predicate_ =
			std::move(GenerateFilterExpression(filters_));
	}
	return plan;
}

auto FilterPushdown::PushdownAggregate(AbstractPlanNodeRef plan)
	-> AbstractPlanNodeRef {
	assert(plan->GetType() == PlanType::Aggregation);
	auto &agg = plan->Cast<AggregationPlanNode>();
	// we cannot push expressions that refer to the aggregate
	FilterPushdown child_pushdown(optimizer);
	if (!filters_.empty()) {
		AbstractExpressionRef filterExpr = GenerateFilterExpression(filters_);
		auto filterPlan = std::make_shared<FilterPlanNode>(
			plan->output_schema_, filterExpr, child_pushdown.Rewrite(plan));
		return filterPlan;
	}
	plan->children_[0] = child_pushdown.Rewrite(std::move(plan->children_[0]));
	return plan;
}

auto FilterPushdown::AddFilter(const AbstractExpressionRef &expr) -> bool {
	std::vector<AbstractExpressionRef> split_exprs;
	AbstractExpressionUtils::SplitConjunctionPredicate(expr, split_exprs);
	for (auto &sexpr : split_exprs) {
		filters_.push_back(std::move(sexpr));
	}
	return true;
}

// Generate predicate by multiple `And` expression
auto FilterPushdown::GenerateFilterExpression(
	std::vector<AbstractExpressionRef> &expressions) -> AbstractExpressionRef {
	assert(expressions.size() >= 1);
	if (expressions.size() == 1) {
		return expressions[0];
	}
	auto logicExpr = std::make_shared<LogicExpression>(
		expressions[0], expressions[1], LogicType::And);
	for (size_t i = 2; i < expressions.size(); i++) {
		logicExpr = std::make_shared<LogicExpression>(logicExpr, expressions[i],
													  LogicType::And);
	}
	return logicExpr;
}

}  // namespace DaseX