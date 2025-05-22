//
// Created by root on 24-3-13.
//

#include "pipe.h"
#include <memory>
#include <vector>

#include "abstract_expression_utils.h"
#include "abstract_plan.h"
#include "agg_plan.h"
#include "bound_columnref_expression.hpp"
#include "bound_comparison_expression.hpp"
#include "bound_order_by.h"
#include "column_value_expression.h"
#include "compare_expression.h"
#include "constant_value_expression.h"
#include "exception.h"
#include "expression.hpp"
#include "expression_transformer.hpp"
#include "expression_type.hpp"
#include "filter_condition.hpp"
#include "filter_plan.h"
#include "join/radixjoin/physical_radix_join.hpp"
#include "logical_type.hpp"
#include "merge_sort_task.hpp"
#include "nest_loop_join_plan.h"
#include "order_type.hpp"
#include "physical_filter.hpp"
#include "physical_hash_aggregation.hpp"
#include "physical_hash_aggregation_with_multifield.hpp"
#include "physical_hash_join.hpp"
#include "physical_nested_loop_join.hpp"
#include "physical_operator.hpp"
#include "physical_operator_states.hpp"
#include "physical_operator_type.hpp"
#include "physical_order.hpp"
#include "physical_project.hpp"
#include "physical_result_collector.hpp"
#include "physical_table_scan.hpp"
#include "planner.hpp"
#include "projection_plan.h"
#include "seq_scan_plan.h"
#include "sort_plan.h"
#include "typedefs.hpp"
#include "catalog.hpp"

namespace DaseX {
/*
 * To build Pipe as a ctx by search plan.
 */
auto Pipe::BuildPipe(const AbstractPlanNodeRef &plan)
	-> std::vector<std::shared_ptr<PhysicalOperator>> {
	auto type_ = plan->GetType();

	switch (plan->GetType()) {
		case PlanType::Projection: {
			return BuildPhysicalProjection(plan);
		}
		case PlanType::NestedLoopJoin: {
			return BuildPhysicalNestedLoopJoin(plan);
		}
		case PlanType::SeqScan: {
			return BuildPhysicalSeqScan(plan);
		}
		case PlanType::Filter: {
			return BuildPhysicalFilter(plan);
		}
		case PlanType::Aggregation: {
			return BuildPhysicalAggregation(plan);
		}
		case PlanType::Sort: {
			return BuildPhysicalSort(plan);
		}
		default:
			throw DaseX::Exception("Unsupported abstract plan type");
	}
}

auto Pipe::BuildPhysicalProjection(const AbstractPlanNodeRef &plan)
	-> std::vector<std::shared_ptr<PhysicalOperator>> {
	const auto &proj = plan->Cast<ProjectionPlanNode>();
	std::vector<std::shared_ptr<Expression>> exprs;
	for (const auto& expression : proj.expressions_) {
		auto expr = ExpressionTransformer::Transform(expression);
		exprs.push_back(std::move(expr));
	}

	auto physical_proj =
		std::make_shared<PhysicalProject>(exprs);


    physical_proj->children_operator = BuildPipe(plan->children_[0]);

	// 是否还有必要保留 ResultCollector
//	root = std::make_shared<PhysicalResultCollector>(
//		PhysicalResultCollector(physical_proj));
//	physical_proj->children_operator = BuildPipe(plan->children_[0]);

	// root->children_operator = {physical_proj};

	return {physical_proj};
}

auto Pipe::BuildPhysicalNestedLoopJoin(const AbstractPlanNodeRef &plan)
	-> std::vector<std::shared_ptr<PhysicalOperator>> {

	auto &join = plan->Cast<NestedLoopJoinPlanNode>();
	std::vector<int> left_col_idx;
	std::vector<int> right_col_idx;

	// TODO： join condition 复杂

	auto expr = ExpressionTransformer::Transform(join.predicate_);
	assert(expr->GetExpressionClass() == ExpressionClass::BOUND_COMPARISON || expr->GetExpressionClass() == ExpressionClass::BOUND_CONJUNCTION);
	auto comp_expr = expr->Cast<BoundComparisonExpression>();
	if (comp_expr.left->GetExpressionClass() == ExpressionClass::BOUND_COLUMN_REF) {
		if (comp_expr.left->Cast<BoundColumnRefExpression>().table_index == 0) {
			left_col_idx.push_back(
				comp_expr.left->Cast<BoundColumnRefExpression>().column_index);
		} else {
			right_col_idx.push_back(
					comp_expr.left->Cast<BoundColumnRefExpression>().column_index);
		}
	} else {
		throw Exception("Pipe::BuildPhysicalNestedLoopJoin only support BOUND_COLUMN_REF (left)");
	}
	if (comp_expr.right->GetExpressionClass() == ExpressionClass::BOUND_COLUMN_REF) {
		if (comp_expr.right->Cast<BoundColumnRefExpression>().table_index == 0) {
			left_col_idx.push_back(
				comp_expr.right->Cast<BoundColumnRefExpression>().column_index);
		} else {
			right_col_idx.push_back(
					comp_expr.right->Cast<BoundColumnRefExpression>().column_index);
		}
	} else {
		throw Exception("Pipe::BuildPhysicalNestedLoopJoin only support BOUND_COLUMN_REF (right)");
	}
	assert(left_col_idx.size() == 1);
	assert(right_col_idx.size() == 1);
	auto phy_join_op = std::make_shared<PhysicalHashJoin>(
		JoinTypes::INNER, left_col_idx, right_col_idx);
	auto left = BuildPipe(plan->children_[0]);
	auto right = BuildPipe(plan->children_[1]);
	phy_join_op->AddChild(left[0]);
	phy_join_op->AddChild(right[0]);
	return {phy_join_op};

//	// 目前仅仅处理semi join，其他处理逻辑类似，默认radixjoin
//	// Update: 全部使用 HashJoin, RadixJoin 疑似有问题
//	if(join.join_type_ == JoinType::SEMI) {
//		auto phy_join_op = std::make_shared<PhysicalHashJoin>(
//			PhysicalHashJoin(JoinTypes::SEMI, {left_col_idx}, {right_col_idx}));
//		auto left = BuildPipe(plan->children_[0]);
//		phy_join_op->children_operator.insert(phy_join_op->children_operator.end(),
//											  left.begin(), left.end());
//		auto right = BuildPipe(plan->children_[1]);
//		phy_join_op->children_operator.insert(phy_join_op->children_operator.end(),
//											  right.begin(), right.end());
//		return {phy_join_op};
//	} else {
//		auto phy_join_op = std::make_shared<PhysicalHashJoin>(
//			PhysicalHashJoin(JoinTypes::INNER,{left_col_idx}, {right_col_idx} ));
//		auto left = BuildPipe(plan->children_[0]);
//		phy_join_op->children_operator.insert(phy_join_op->children_operator.end(),
//											  left.begin(), left.end());
//		auto right = BuildPipe(plan->children_[1]);
//		phy_join_op->children_operator.insert(phy_join_op->children_operator.end(),
//											  right.begin(), right.end());
//		return {phy_join_op};

}

auto Pipe::BuildPhysicalSeqScan(const AbstractPlanNodeRef &plan)
	-> std::vector<std::shared_ptr<PhysicalOperator>> {
	const auto &scan = plan->Cast<SeqScanPlanNode>();
	std::shared_ptr<Table> temp_table = nullptr;
	auto rc = open_catalog_ptr.lock()->get_table(scan.table_name_, temp_table);
	std::vector<int> projection_ids;
	projection_ids.reserve(temp_table->column_nums);
	for (int i = 0; i < temp_table->column_nums; i++) {
		projection_ids.push_back(i);
	}
	std::shared_ptr<PhysicalOperator> oper =
		std::make_shared<PhysicalTableScan>(temp_table, -1, projection_ids);
	if (scan.filter_predicate_) {
		// 由于 index scan 未实现，创建一个新的 Filter 算子来过滤 Scan 中的
		// predicate 条件
		auto physical_filter = std::make_shared<PhysicalFilter>(
			ExpressionTransformer::Transform(scan.filter_predicate_));
		physical_filter->AddChild(oper);
		oper = physical_filter;
	}
	return {oper};
}

auto Pipe::BuildPhysicalFilter(const AbstractPlanNodeRef &plan)
	-> std::vector<std::shared_ptr<PhysicalOperator>> {
	auto filter = dynamic_cast<const FilterPlanNode &>(*plan);
	auto expr = ExpressionTransformer::Transform(filter.predicate_);
	std::shared_ptr<PhysicalFilter> physical_filter =
		std::make_shared<PhysicalFilter>(expr);
	physical_filter->children_operator = BuildPipe(plan->children_[0]);
	return {physical_filter};
}

auto Pipe::BuildPhysicalAggregation(const AbstractPlanNodeRef &plan)
	-> std::vector<std::shared_ptr<PhysicalOperator>> {
	const auto &agg = plan->Cast<AggregationPlanNode>();

	std::vector<int> group_by_ids;
	std::vector<int> is_star;
	std::vector<AggFunctionType> agg_types;
	std::vector<int> agg_ids;
    int is_star_idx = 0;

	assert(agg.aggregates_.size() == agg.agg_types_.size());
	size_t agg_size = agg.aggregates_.size();
	size_t group_size = agg.group_bys_.size();

	for (const auto &expr : agg.group_bys_) {
		assert(expr->GetExpressionClass() == ExpressionClass::COLUMN_REF);
		auto &col = expr->Cast<ColumnValueExpression>();
		group_by_ids.push_back((int)col.GetColIdx());
	}
	for (size_t i = 0; i < agg_size; i++) {
		if (agg.aggregates_[i]->GetExpressionClass() == ExpressionClass::COLUMN_REF) {
			auto &col =
				agg.aggregates_.at(i)->Cast<ColumnValueExpression>();
			agg_ids.push_back((int)col.GetColIdx());
		} else if (agg.aggregates_[i]->GetExpressionClass() == ExpressionClass::CONSTANT) {
			assert(agg.agg_types_[i] == AggregationType::CountStarAggregate);
			agg_ids.push_back(0);
		}

		AggregationType type = agg.GetAggregateTypes().at(i);
		switch (type) {

			case AggregationType::CountStarAggregate:
			case AggregationType::CountAggregate: {
				agg_types.push_back(AggFunctionType::COUNT);
				break;
			}
			case AggregationType::SumAggregate: {
				agg_types.push_back(AggFunctionType::SUM);
				break;
			}
			case AggregationType::MinAggregate: {
				agg_types.push_back(AggFunctionType::MIN);
				break;
			}
			case AggregationType::AvgAggregate: {
				agg_types.push_back(AggFunctionType::AVG);
				break;
			}
			case AggregationType::MaxAggregate: {
				agg_types.push_back(AggFunctionType::MAX);
				break;
			}
		}
		if (type == AggregationType::CountStarAggregate) {
			is_star.push_back(1);
            is_star_idx = int(i);
		} else {
			is_star.push_back(0);
		}
	}

	std::shared_ptr<PhysicalOperator> temp_agg;
	if (group_by_ids.empty()) {
		temp_agg = std::make_shared<PhysicalHashAgg>(
			-1, agg_types, agg_ids, is_star_idx);
	} else {
		temp_agg = std::make_shared<PhysicalMultiFieldHashAgg>(
				group_by_ids, agg_ids, is_star, agg_types);
	}

	temp_agg->children_operator = BuildPipe(plan->children_[0]);
    // std::vector<std::shared_ptr<Expression>> proj_expr;
    // for(auto expression : agg.group_bys_) {
    //     auto col_idx = ExpressionTransformer::Transform(expression);
    //     proj_expr.push_back(col_idx);
    // }
    // for (auto expression : agg.aggregates_) {
    //     auto col_idx = ExpressionTransformer::Transform(expression);
    //     proj_expr.push_back(col_idx);
    // }

    // auto physical_proj =
    //         std::make_shared<PhysicalProject>(proj_expr);

	// physical_proj->children_operator = BuildPipe(plan->children_[0]);
//	root = std::make_shared<PhysicalResultCollector>(
//		PhysicalResultCollector(physical_proj));
//	physical_proj->children_operator = BuildPipe(plan->children_[0]);

    // root->children_operator = {physical_proj};

//    std::vector<int> back_gb_ids;
//    std::vector<int> back_agg_ids;
//    for(int i = 0;i<group_size;i++) {
//        back_gb_ids.push_back(i);
//    }
//    for(int j = group_size;j<agg_size+group_size;j++) {
//        if(j != is_star_idx+group_size) {
//            back_agg_ids.push_back(j);
//        } else {
//            // 如果第一个就是 count * 未处理
//            back_agg_ids.push_back(group_size);
//        }
//
//    }
//
//	// 对于无分组聚合
//	if(agg.group_bys_.size() == 0) {
//		auto temp_agg = std::make_shared<PhysicalHashAgg>(
//			 -1, agg_types,back_agg_ids);
//		temp_agg->children_operator = {physical_proj};
//		return {temp_agg};
//	}
//    auto temp_agg = std::make_shared<PhysicalMultiFieldHashAgg>(
//            back_gb_ids, back_agg_ids, is_star, agg_types);

//    auto temp_proj = std::make_shared<PhysicalProject>(
//            group_by_ids, agg_ids, is_star, agg_types);
//


	// temp_agg->children_operator = {physical_proj};


//    auto temp_agg = std::make_shared<PhysicalMultiFieldHashAgg>(
//            group_by_ids, agg_ids, is_star, agg_types);

//    temp_agg->children_operator = BuildPipe(plan->children_[0]);
	return {temp_agg};
}

auto Pipe::BuildPhysicalSort(const AbstractPlanNodeRef &plan)
	-> std::vector<std::shared_ptr<PhysicalOperator>> {
	auto &sort = plan->Cast<SortPlanNode>();
	/// @todo: Fix memory leak: PhysicalOrder must manage the lifetime of the
	/// following three vectors
	auto sort_keys = new std::vector<int>;
	auto key_types = new std::vector<LogicalType>;
	auto orders = new std::vector<SortOrder>;
	for (auto orderby : sort.GetOrderBy()) {
		auto &expr = orderby.second;
		assert(expr->GetExpressionClass() == ExpressionClass::COLUMN_REF);
		auto &column_expr = expr->Cast<ColumnValueExpression>();
		sort_keys->push_back((int)column_expr.GetColIdx());
		key_types->push_back(column_expr.GetReturnType());
		// key_types->push_back(LogicalType::STRING);
		orders->push_back(SortOrder(orderby.first));
		// orders->push_back(SortOrder::ASCENDING);
	}
	auto physical_order =
		std::make_shared<PhysicalOrder>(sort_keys, key_types, orders);
	physical_order->children_operator = BuildPipe(plan->children_[0]);
	return {physical_order};
}

auto Pipe::IsCrossProduct(const AbstractExpressionRef &predicate) -> bool {
	bool cross_product = false;
	assert(predicate->GetExpressionClass() == ExpressionClass::COMPARISON);
	auto &comp_expr = predicate->Cast<ComparisonExpression>();
	if (comp_expr.children_[0]->GetExpressionClass() ==
			ExpressionClass::CONSTANT &&
		comp_expr.children_[1]->GetExpressionClass() ==
			ExpressionClass::CONSTANT) {
		auto &left_const = comp_expr.children_[0]->Cast<ConstantValueExpression>();
		auto &right_const = comp_expr.children_[1]->Cast<ConstantValueExpression>();
		if (left_const.val_ == right_const.val_) {
			cross_product = true;
		}
	}
	return cross_product;
}

}  // namespace DaseX