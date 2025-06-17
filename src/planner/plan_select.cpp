/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-19 16:12:34
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-01-18 13:12:46
 * @FilePath: /task_sche_parser/src/planner/plan_select.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#include <arrow/type.h>

#include <array>
#include <cstdio>

#include "../include/binder/bound_expression.h"
#include "../include/binder/bound_table_ref.h"
#include "../include/binder/expressions/bound_alias.h"
#include "../include/binder/expressions/bound_binary_op.h"
#include "../include/binder/table_ref/bound_base_table_ref.h"
#include "../include/binder/table_ref/bound_cross_product_ref.h"
#include "../include/binder/table_ref/bound_cte_ref.h"
#include "../include/binder/table_ref/bound_expression_list_ref.h"
#include "../include/binder/table_ref/bound_join_ref.h"
#include "abstract_expression_utils.h"
#include "bound_column_ref.h"
#include "bound_columnref_expression.hpp"
#include "bound_constant.h"
#include "bound_constant_expression.hpp"
#include "bound_subquery.h"
#include "bound_subquery_ref.h"
#include "calc_expression.h"
#include "expression_type.hpp"
#include "like_expression.h"
#include "abstract_expression.h"
#include "abstract_plan.h"
#include "agg_plan.h"
#include "bound_agg_call.h"
#include "bound_case.h"
#include "bound_func_call.h"
#include "calc_expression.h"
#include "case_expression.h"
#include "column_value_expression.h"
#include "compare_expression.h"
#include "constant_value_expression.h"
#include "exception.h"
#include "filter_plan.h"
#include "like_expression.h"
#include "logic_expression.h"
#include "logical_type.hpp"
#include "nest_loop_join_plan.h"
#include "physical_operator_type.hpp"
#include "planner.hpp"
#include "projection_plan.h"
#include "seq_scan_plan.h"
#include <array>
#include <cstdio>
#include <memory>
#include <arrow/type.h>
#include "bound_agg_call.h"
#include "sort_plan.h"
#include "subquery_expression.h"
#include "planner.hpp"
#include "projection_plan.h"
#include "seq_scan_plan.h"
#include "sort_plan.h"
#include "llm_expression.h"
#include <spdlog/spdlog.h>

namespace DaseX {

auto Planner::PlanSelect(const SelectStatement &statement)
    -> AbstractPlanNodeRef {

	auto ctx_guard = NewContext();
	if (!statement.ctes_.empty()) {
		ctx_.cte_list_ = &statement.ctes_;
	}

  AbstractPlanNodeRef plan = nullptr;

  switch (statement.table_->type_) {

    // tableref between from - where
    case TableReferenceType::BASE_TABLE:
    case TableReferenceType::CROSS_PRODUCT:
    case TableReferenceType::JOIN:
    case TableReferenceType::SUBQUERY:
      plan = PlanTableRef(*statement.table_);
      break;
    default:
      throw NotImplementedException("Planner::PlanSelect unsupported table type");

  }

//  // plan where, to construct filter node
  if (!statement.where_->IsInvalid()) {
    auto schema = plan->OutputSchema();
    
    /// 这里实际上对应了 duckdb 的 BindNode，做表达式绑定
    auto temp_tuple = PlanExpression(*statement.where_, {plan});
    /// 子查询改写
    auto expr = std::get<1>(temp_tuple);
    PlanSubqueries(expr, plan);

    plan = std::make_shared<FilterPlanNode>(
        std::make_shared<arrow::Schema>(schema), std::move(expr),
        std::move(plan));
  }

  // printf("where finish\n");

  bool has_agg = false;
  for (const auto &item : statement.select_list_) {
    if (item->HasAggregation()) {
      has_agg = true;
      break;
    }
  }

  if (!statement.having_->IsInvalid() || !statement.group_by_.empty() ||
      has_agg) {
    // TODO process agg
    plan = PlanSelectAgg(statement, std::move(plan));
  } else {
    // 普通select
    std::vector<AbstractExpressionRef> exprs;
    std::vector<std::string> column_names;
    std::vector<AbstractPlanNodeRef> children = {plan};
    for (const auto &item : statement.select_list_) {
      auto [name, expr] = PlanExpression(*item, {plan});
      exprs.emplace_back(std::move(expr));
      column_names.emplace_back(std::move(name));
    }
    // like 也应该和 聚合 一样放到特殊处理

    if(exprs[0]->expression_class_ == ExpressionClass::LIKE) {
        plan = std::make_shared<ProjectionPlanNode>(
                std::make_shared<arrow::Schema>(ProjectionPlanNode::RenameSchema(
                        ProjectionPlanNode::InferProjectionSchema({exprs[0]->children_[0]}), column_names)),
                std::move(exprs), std::move(plan));
    } else {
        plan = std::make_shared<ProjectionPlanNode>(
                std::make_shared<arrow::Schema>(ProjectionPlanNode::RenameSchema(
                        ProjectionPlanNode::InferProjectionSchema(exprs), column_names)),
                std::move(exprs), std::move(plan));
    }

  }

    if (!statement.sort_.empty()) {
        std::vector<std::pair<OrderByType, AbstractExpressionRef>> order_bys;
        for (const auto &order_by : statement.sort_) {
            auto [_, expr] = PlanExpression(*order_by->expr_, {plan});
            auto abstract_expr = std::move(expr);
            order_bys.emplace_back(std::make_pair(order_by->type_, abstract_expr));
        }
        plan = std::make_shared<SortPlanNode>(std::make_shared<arrow::Schema>(plan->OutputSchema()), plan, std::move(order_bys));
    }

  	return plan;
}

auto Planner::PlanTableRef(const BoundTableRef &table_ref)
    -> AbstractPlanNodeRef {

  switch (table_ref.type_) {
  case TableReferenceType::BASE_TABLE: {
    const auto &base_table_ref =
        dynamic_cast<const BoundBaseTableRef &>(table_ref);
    return PlanBaseTableRef(base_table_ref);
  }
  case TableReferenceType::CROSS_PRODUCT: {
    const auto &cross_product =
        dynamic_cast<const BoundCrossProductRef &>(table_ref);
    return PlanCrossProductRef(cross_product);
  }
  case TableReferenceType::JOIN: {
    const auto &join = dynamic_cast<const BoundJoinRef &>(table_ref);
    printf("join weak support now\n");
    return PlanJoinRef(join);
  }
  case TableReferenceType::EXPRESSION_LIST: {
    const auto &expression_list =
        dynamic_cast<const BoundExpressionListRef &>(table_ref);
    // return PlanExpressionListRef(expression_list);
    printf("expressonlist not support now\n");
  }
  case TableReferenceType::SUBQUERY: {
    const auto &subquery = dynamic_cast<const BoundSubqueryRef &>(table_ref);
    return PlanSubqueryRef(subquery);

//    return PlanSubquery(subquery, subquery.alias_);
  }
  case TableReferenceType::CTE: {
	  const auto &cte = dynamic_cast<const BoundCTERef &>(table_ref);
	  return PlanCTERef(cte);

  }

  default:
    break;
  }
  throw DaseX::Exception(
      "the table ref type {} is not supported in planner yet");
}

auto Planner::PlanBaseTableRef(const BoundBaseTableRef &table_ref)
    -> AbstractPlanNodeRef {
  //
  std::shared_ptr<Table> table;
  auto RC = catalog_.lock()->get_table(table_ref.table_, table);

  // plan as normal SeqScan.
  // printf("seq scan plan node\n");
  // TODO:fix plan node.
  // 目前我直接把schema返回去了，对多表应该返回一个x.y列名的临时元数据表的
  // inferscanschema 中 finish todo return std::make_shared<SeqScanPlanNode>(
  // table->schema,table->table_name);
  return std::make_shared<SeqScanPlanNode>(
      std::make_shared<arrow::Schema>(
          SeqScanPlanNode::InferScanSchema(table_ref)),
      table->table_name);
}

auto Planner::PlanCrossProductRef(const BoundCrossProductRef &table_ref)
    -> AbstractPlanNodeRef {
  auto left = PlanTableRef(*table_ref.left_);
  auto right = PlanTableRef(*table_ref.right_);

  // check
//  printf("%d\n", left->GetType());
//  printf("%d\n", right->GetType());
//  printf("笛卡尔积\n");
  // crossproduct
  std::shared_ptr<Table> table;

  return std::make_shared<NestedLoopJoinPlanNode>(
      std::make_shared<arrow::Schema>(
          NestedLoopJoinPlanNode::InferJoinSchema(*left, *right)),
      std::move(left), std::move(right),
      std::make_shared<ComparisonExpression>(std::make_shared<ConstantValueExpression>(Value(1)),std::make_shared<ConstantValueExpression>(Value(1)),ComparisonType::Equal), JoinType::INNER);
}

void Planner::PlanSubqueries(AbstractExpressionRef &expr, AbstractPlanNodeRef &plan) {
  if (!expr) {
    return;
  }

  AbstractExpressionUtils::EnumerateChildren(expr, [&](AbstractExpressionRef &expr) { PlanSubqueries(expr, plan); });
  
  if (expr->expression_class_ == ExpressionClass::SUBQUERY) {
    auto &subquery = expr->Cast<SubqueryExpression>();
    expr = PlanSubquery(subquery, plan);
  }
	// if (!expr_ptr) {
	// 	return;
	// }
	// auto &expr = *expr_ptr;
	// // first visit the children of the node, if any
	// ExpressionIterator::EnumerateChildren(expr, [&](unique_ptr<Expression> &expr) { PlanSubqueries(expr, root); });

	// // check if this is a subquery node
	// if (expr.expression_class == ExpressionClass::BOUND_SUBQUERY) {
	// 	auto &subquery = expr.Cast<SubqueryExpression>();
	// 	// subquery node! plan it
	// 	if (subquery.IsCorrelated() && !is_outside_flattened) {
	// 		// detected a nested correlated subquery
	// 		// we don't plan it yet here, we are currently planning a subquery
	// 		// nested subqueries will only be planned AFTER the current subquery has been flattened entirely
	// 		has_unplanned_dependent_joins = true;
	// 		return;
	// 	}
	// 	expr_ptr = PlanSubquery(subquery, root);
	// }
}

auto Planner::PlanUncorrelatedSubquery(SubqueryExpression &expr, AbstractPlanNodeRef &root,
  AbstractPlanNodeRef plan) -> AbstractExpressionRef
{
  switch (expr.subquery_type)
  {
    case SubqueryType::SCALAR:
    {
      std::vector<AbstractExpressionRef> group_by_expr;
      std::vector<AbstractExpressionRef> input_exprs;
      std::vector<AggregationType> agg_types;
      auto schema = plan->OutputSchema();
      // 子查询必须只有一列
      if (schema.fields().size() != 1)
        throw Exception("Planner::PlanUncorrelatedSubquery unexpected fields size");

      // 现在 tuple_idx 始终是 0，col_idx = 0 对应唯一的一列
      auto input_expr = std::make_shared<ColumnValueExpression>(0, 0, 
        SchemaTypeToLogicalType(schema.field(0)->type()));
      input_exprs.push_back(input_expr);

      // duckdb 用的是 Func First，这里使用 Max 聚合，在结果只有一行时是等价的。暂时不考虑结果超过一行的情况
      // 计划阶段无法判断结果行数
      agg_types.push_back(AggregationType::MaxAggregate);

      auto agg_output_schema = std::make_shared<arrow::Schema>(
        AggregationPlanNode::InferAggSchema(group_by_expr, input_exprs, agg_types));
      const std::vector<std::string> outputschema_col_names = {"scalar subquery"};
      agg_output_schema = std::make_shared<arrow::Schema>(ProjectionPlanNode::RenameSchema(
        *agg_output_schema, outputschema_col_names));

      auto agg_plan = std::make_shared<AggregationPlanNode>(
        agg_output_schema, plan, group_by_expr, input_exprs, agg_types);
      plan = std::move(agg_plan);
      
      // 按照 Planner::PlanCrossProductRef 的实现，对于笛卡尔积，需要在 condition 上加 1=1...
      auto join_condition = std::make_shared<ComparisonExpression>(
        std::make_shared<ConstantValueExpression>(Value(1)),
        std::make_shared<ConstantValueExpression>(Value(1)),
        ComparisonType::Equal);

      auto crossproduct_plan = std::make_shared<NestedLoopJoinPlanNode>(std::make_shared<arrow::Schema>(
          NestedLoopJoinPlanNode::InferJoinSchema(*root, *plan)),
          std::move(root), std::move(plan), std::move(join_condition),
          JoinType::INNER);
      
      root = std::move(crossproduct_plan);
      
      // 子查询转换后的聚合列在最后一列
      return std::make_shared<ColumnValueExpression>(0, root->OutputSchema().num_fields() - 1, expr.GetReturnType());
    }
    case DaseX::SubqueryType::ANY:
    {
      auto schema = plan->OutputSchema();
      // 子查询必须只有一列
      if (schema.fields().size() != 1)
        throw Exception("Planner::PlanUncorrelatedSubquery unexpected fields size");

      auto join_condition = std::make_shared<ComparisonExpression>(
        expr.child,
        std::make_shared<ColumnValueExpression>(1, 0, expr.GetReturnType()),
        expr.comparison_type);

      auto semijoin_plan = std::make_shared<NestedLoopJoinPlanNode>(std::make_shared<arrow::Schema>(
          NestedLoopJoinPlanNode::InferJoinSchema(*root, *plan)),
          std::move(root), std::move(plan), std::move(join_condition),
          JoinType::SEMI);
      
      root = std::move(semijoin_plan);
      // 将原来的表达式替换为恒等式 1=1
      return std::make_shared<ComparisonExpression>(
        std::make_shared<ConstantValueExpression>(Value(1)),
        std::make_shared<ConstantValueExpression>(Value(1)),
        ComparisonType::Equal);
    }
    // case SubqueryType::EXISTS:
    // {
    //   // int a = 1;
    //   // auto join_plan = std::make_shared<NestedLoopJoinPlanNode>(JoinType::SEMI);
    //   break;
    // }
    default:
      break;
  }
  throw NotImplementedException("Planner::PlanUncorrelatedSubquery not implemented subquery type");
}

auto Planner::PlanSubquery(SubqueryExpression &expr, AbstractPlanNodeRef &root) 
  -> AbstractExpressionRef {
    auto &plan = expr.subquery;
    AbstractExpressionRef result_expr;
    /// 目前只考虑非相关子查询
    result_expr = PlanUncorrelatedSubquery(expr, root, plan);
    return result_expr;
}

auto Planner::PlanColumnRef(const BoundColumnRef &expr,
                            const std::vector<AbstractPlanNodeRef> &children)
    -> std::tuple<std::string, std::shared_ptr<ColumnValueExpression>> {
  if (children.empty()) {
    throw Exception("column ref should have at least one child");
  }

  auto col_name = expr.ToString();
//  printf("expr name :%s\n", col_name.c_str());

  if (children.size() == 1) {
    // Projections, Filters, and other executors evaluating expressions with one
    // single child will use this branch.
//    printf("Projections, Filters,  expressions with one single child will use "
//           "this branch.\n");
    const auto &child = children[0];
    auto schema = child->OutputSchema();
    auto fields = schema.field_names();
//	for(auto field : fields) {
//		printf("fields name : %s\n", field.c_str());
//;	}
    /// @TODO: 处理col_idx为-1，相关列的情况
    uint32_t col_idx = schema.GetFieldIndex(col_name);
    // assert(col_idx != -1);
    auto col_type = schema.field(col_idx)->type();
    /// @todo: 使用 SchemaTypeToLogicalType 替换下面的逻辑
    auto internal_col_type = LogicalType::INTEGER;

    if( col_type == arrow::int32()) {
        internal_col_type = LogicalType::INTEGER;
    } else if (col_type == arrow::float32() ) {
        internal_col_type = LogicalType::FLOAT;
    } else if (col_type == arrow::float64() ) {
        internal_col_type = LogicalType::DOUBLE;
    } else {
        // 目前else 为 string
        // join 暂未改
        internal_col_type = LogicalType::STRING;
    }

    return std::make_tuple(col_name, std::make_shared<ColumnValueExpression>(
                                         0, col_idx, internal_col_type));
  }
  if (children.size() == 2) {
    /*
     * Joins will use this branch to plan expressions.
     */
    printf("Joins will use this branch to plan expressions.\n");
    const auto &left = children[0];
    const auto &right = children[1];
    auto left_schema = left->OutputSchema();
    auto right_schema = right->OutputSchema();

    auto col_idx_left = left_schema.GetFieldIndex(col_name);
    auto col_idx_right = right_schema.GetFieldIndex(col_name);
    if (col_idx_left != -1) {
      // auto col_type = left_schema.GetColumn(*col_idx_left).GetType();

	  auto col_type = left_schema.field(col_idx_left)->type();
	  auto internal_col_type = LogicalType::INTEGER;
	  if( col_type == arrow::int32()) {
		  internal_col_type = LogicalType::INTEGER;
	  } else if (col_type == arrow::float32() ) {
		  internal_col_type = LogicalType::FLOAT;
	  } else if (col_type == arrow::float64() ) {
		  internal_col_type = LogicalType::DOUBLE;
	  } else {
		  // 目前else 为 string
		  // join 暂未改
		  internal_col_type = LogicalType::STRING;
	  }
      return std::make_tuple(col_name, std::make_shared<ColumnValueExpression>(
                                           0, col_idx_left, internal_col_type));
    }
    if (col_idx_right != -1) {
      // auto col_type = right_schema.GetColumn(*col_idx_right).GetType();
	  auto col_type = right_schema.field(col_idx_right)->type();
	  auto internal_col_type = LogicalType::INTEGER;
	  if( col_type == arrow::int32()) {
		  internal_col_type = LogicalType::INTEGER;
	  } else if (col_type == arrow::float32() ) {
		  internal_col_type = LogicalType::FLOAT;
	  } else if (col_type == arrow::float64() ) {
		  internal_col_type = LogicalType::DOUBLE;
	  } else {
		  // 目前else 为 string
		  // join 暂未改
		  internal_col_type = LogicalType::STRING;
	  }
      return std::make_tuple(col_name, std::make_shared<ColumnValueExpression>(
                                           1, col_idx_right, internal_col_type));
    }
  }
  throw NotImplementedException("Planner::PlanColumnRef children.size()");
}

auto Planner::PlanBinaryOp(const BoundBinaryOp &expr,
                           const std::vector<AbstractPlanNodeRef> &children)
    -> AbstractExpressionRef {
  auto [_1, left] = PlanExpression(*expr.larg_, children);
  auto [_2, right] = PlanExpression(*expr.rarg_, children);
  const auto &op_name = expr.op_name_;
  if (op_name == "=" || op_name == "==") {
    return std::make_shared<ComparisonExpression>(
        std::move(left), std::move(right), ComparisonType::Equal);
  }
  if (op_name == "!=" || op_name == "<>") {
    return std::make_shared<ComparisonExpression>(
        std::move(left), std::move(right), ComparisonType::NotEqual);
  }
  if (op_name == "<") {
    return std::make_shared<ComparisonExpression>(
        std::move(left), std::move(right), ComparisonType::LessThan);
  }
  if (op_name == "<=") {
    return std::make_shared<ComparisonExpression>(
        std::move(left), std::move(right), ComparisonType::LessThanOrEqual);
  }
  if (op_name == ">") {
    return std::make_shared<ComparisonExpression>(
        std::move(left), std::move(right), ComparisonType::GreaterThan);
  }
  if (op_name == ">=") {
    return std::make_shared<ComparisonExpression>(
        std::move(left), std::move(right), ComparisonType::GreaterThanOrEqual);
  }
  if (op_name == "and") {
    return std::make_shared<LogicExpression>(std::move(left), std::move(right),
                                             LogicType::And);
  }
  if (op_name == "or") {
	  return std::make_shared<LogicExpression>(std::move(left), std::move(right),
											   LogicType::Or);
  }
  if(op_name == "+") {
    return std::make_shared<CalcExpression>(left,right,CalcType::Plus);
  }
    if(op_name == "-") {
        return std::make_shared<CalcExpression>(left,right,CalcType::Minus);
    }
    if(op_name == "*") {
        return std::make_shared<CalcExpression>(left,right,CalcType::Multiply);
    }
    if(op_name == "/") {
        return std::make_shared<CalcExpression>(left,right,CalcType::Divide);
    }

  if(op_name == "~~") {
      return std::make_shared<LikeExpression>(std::move(left), std::move(right),
                                              false);
  }
  if(op_name == "!~~") {
      return std::make_shared<LikeExpression>(std::move(left), std::move(right),
                                              true);
  }
  throw NotImplementedException("Planner::PlanBinaryOp op_name " + op_name);
  // return GetBinaryExpression(op_name, std::move(left), std::move(right));
}

auto Planner::PlanConstant(const BoundConstant &expr,
                           const std::vector<AbstractPlanNodeRef> &children)
    -> AbstractExpressionRef {
  return std::make_shared<ConstantValueExpression>(
      expr.val_);
}


auto Planner::PlanSubqueryExpression(const BoundSubquery &expr,
  const std::vector<AbstractPlanNodeRef> &children) -> AbstractExpressionRef {
	auto plan_node = PlanSelect(*expr.subquery);
	if (expr.subquery_type != SubqueryType::EXISTS &&
		plan_node->OutputSchema().num_fields() > 1) {
		throw Exception("Subquery returns more than 1 columns (expected 1)");
	}
	AbstractExpressionRef child;
	if (expr.child) {
		auto result = PlanExpression(*expr.child, children);
		child = std::get<1>(result);
	}

	LogicalType return_type =
		expr.subquery_type == SubqueryType::SCALAR
			? SchemaTypeToLogicalType(
				  plan_node->OutputSchema().field(0)->type())
			: LogicalType::BOOL;

	auto result = std::make_unique<SubqueryExpression>(return_type);
	if (expr.subquery_type == SubqueryType::ANY) {
		// ANY comparison
		assert(child);
		auto child_type = child->GetReturnType();
		LogicalType compare_type;
		/// @TODO: 隐式类型转换
		if (child_type != SchemaTypeToLogicalType(
							  plan_node->OutputSchema().field(0)->type())) {
			throw NotImplementedException(
				"Cannot compare values of type in IN/ANY/ALL clause. Implicit type cast is not implemented");
		}
		result->child_type = child_type;
		result->child = std::move(child);
	}
	result->subquery = std::move(plan_node);
	result->subquery_type = expr.subquery_type;
	result->comparison_type = expr.comparison_type;
	return result;
}
auto Planner::PlanFuncCall(const BoundFuncCall &expr, const std::vector<AbstractPlanNodeRef> &children)
	-> AbstractExpressionRef {
	std::vector<AbstractExpressionRef> args;
	for (const auto &arg : expr.args_) {
		auto [_1, arg_expr] = PlanExpression(*arg, children);
		args.push_back(std::move(arg_expr));
	}
	if (expr.func_name_ == "llm") {
		printf("Planner::PlanFuncCall func_name: %s " , expr.func_name_.c_str());
		// TODO 接受变长 args ，可能要写一个递归变长的模版类？
		return std::make_shared<LlmExpression>(args[0], args[1], args[2], LlmType::Decide);
		// return std::make_shared<ArrayExpression>(args);
	}
	throw NotImplementedException("Planner::PlanFuncCall func_name:  " + expr.func_name_);

}

auto Planner::PlanExpression(const BoundExpression &expr,
                             const std::vector<AbstractPlanNodeRef> &children)
    -> std::tuple<std::string, AbstractExpressionRef> {
  switch (expr.type_) {

  case ExpressionType::AGG_CALL: {
	  if (ctx_.next_aggregation_ >= ctx_.expr_in_agg_.size()) {
		  printf("agg size is error\n");
	  }
    return std::make_tuple(
        "UNNAMED_COLUMN",
        std::move(ctx_.expr_in_agg_[ctx_.next_aggregation_++]));
  }
  case ExpressionType::FUNC_CALL: {
	  const auto &func_call_expr = dynamic_cast<const BoundFuncCall &>(expr);
	  return std::make_tuple("unnameed func call",PlanFuncCall(func_call_expr, children));
  }
  case ExpressionType::COLUMN_REF: {
    const auto &column_ref_expr = dynamic_cast<const BoundColumnRef &>(expr);
    return PlanColumnRef(column_ref_expr, children);
  }
  case ExpressionType::BINARY_OP: {
    const auto &binary_op_expr = dynamic_cast<const BoundBinaryOp &>(expr);
    return std::make_tuple("unnamed binary_op",
                           PlanBinaryOp(binary_op_expr, children));
  }
  case ExpressionType::CONSTANT: {
    const auto &constant_expr = dynamic_cast<const BoundConstant &>(expr);
    return std::make_tuple("unnamed const",
                           PlanConstant(constant_expr, children));
  }
  case ExpressionType::ALIAS: {
    const auto &alias_expr = dynamic_cast<const BoundAlias &>(expr);
    auto [_1, expr] = PlanExpression(*alias_expr.child_, children);
    return std::make_tuple(alias_expr.alias_, std::move(expr));
  }

  case ExpressionType::SUBQUERY: {
	  /// @TODO: 处理 Subquery expression 对照 duckdb::ExpressionBinder::BindExpression(SubqueryExpression &expr, idx_t depth)
	  const auto &subquery_expr = dynamic_cast<const BoundSubquery &>(expr);
	  return std::make_tuple("unnamed subquery",
							 PlanSubqueryExpression(subquery_expr, children));
  }
  case ExpressionType::CASE: {
	  const auto &case_expr = dynamic_cast<const BoundCase &>(expr);
	  return std::make_tuple("unnamed case",
							 PlanCase(case_expr, children));

  }
  default:
    break;
  }
  printf("%d", expr.type_);
  throw Exception("expression type {} not supported in planner yet,because "
                  "PlanColumnRef not finish");
}


auto Planner::PlanSubquery(const BoundSubqueryRef &table_ref, const std::string &alias) -> AbstractPlanNodeRef {
	auto select_node = PlanSelect(*table_ref.subquery_);
	std::vector<std::string> output_column_names;
	std::vector<AbstractExpressionRef> exprs;
	size_t idx = 0;

	for (const auto &col : select_node->OutputSchema().fields()) {

		auto expr = std::make_shared<ColumnValueExpression>(0, idx, LogicalType::FLOAT);
		output_column_names.emplace_back(fmt::format("{}.{}", alias, fmt::join(table_ref.select_list_name_[idx], ".")));
		exprs.push_back(std::move(expr));
		idx++;
	}

	auto saved_child = std::move(select_node);

	return std::make_shared<ProjectionPlanNode>(
		std::make_shared<arrow::Schema>(
			ProjectionPlanNode::RenameSchema(ProjectionPlanNode::InferProjectionSchema(exprs), output_column_names)),
		std::move(exprs), saved_child);
}

auto Planner::PlanCTERef(const BoundCTERef &table_ref) -> AbstractPlanNodeRef {
	for (const auto &cte : *ctx_.cte_list_) {
		if (cte->alias_ == table_ref.cte_name_) {
			return PlanSubquery(*cte, table_ref.alias_);
		}
	}
}

auto Planner::PlanJoinRef(const BoundJoinRef &table_ref)
    -> AbstractPlanNodeRef {
  auto left = PlanTableRef(*table_ref.left_);
  auto right = PlanTableRef(*table_ref.right_);
  std::vector<AbstractPlanNodeRef> children;
  children.push_back(left);
  children.push_back(right);
  auto temp_tuple = PlanExpression(*table_ref.condition_, children);
  auto [_, join_condition] =
      PlanExpression(*table_ref.condition_, {left, right});

    printf("Join Type is : %d\n",table_ref.join_type_);
  auto nlj_node = std::make_shared<NestedLoopJoinPlanNode>(
      std::make_shared<arrow::Schema>(
          NestedLoopJoinPlanNode::InferJoinSchema(*left, *right)),
      std::move(left), std::move(right), std::move(join_condition),
      table_ref.join_type_);


  return nlj_node;
}


auto Planner::PlanSubqueryRef(const BoundSubqueryRef &table_ref)
    -> AbstractPlanNodeRef {
  auto plan = PlanSelect(*table_ref.subquery_);
  if (!table_ref.alias_.empty()) {
    // 将别名添加到 schema 里
    auto &schema = plan->OutputSchema();
    arrow::FieldVector field_vector;
    for (size_t idx = 0; idx < schema.num_fields(); ++idx) {
        auto field = schema.field(idx);
        std::string field_name = table_ref.alias_ + "." + field->name();
        auto new_field = std::make_shared<arrow::Field>(field_name, field->type(), field->nullable(), field->metadata());
        field_vector.push_back(new_field);
//=======
//    void Planner::AddAggCallToContext(BoundExpression &expr) {
//        switch (expr.type_) {
//            case ExpressionType::AGG_CALL: {
//                auto &agg_call_expr = dynamic_cast<BoundAggCall &>(expr);
//                auto agg_name = fmt::format("__pseudo_agg#{}", ctx_.aggregations_.size());
//                auto agg_call =
//                        BoundAggCall(agg_name, agg_call_expr.is_distinct_, std::vector<std::unique_ptr<BoundExpression>>{});
//
//                ctx_.aggregations_.push_back(std::make_unique<BoundAggCall>(std::exchange(agg_call_expr, std::move(agg_call))));
//
//                return;
//            }
//            case ExpressionType::COLUMN_REF: {
//                return;
//            }
//            case ExpressionType::BINARY_OP: {
//                auto &binary_op_expr = dynamic_cast<BoundBinaryOp &>(expr);
//                AddAggCallToContext(*binary_op_expr.larg_);
//                AddAggCallToContext(*binary_op_expr.rarg_);
//                return;
//            }
//
//            case ExpressionType::CONSTANT: {
//                return;
//            }
//            case ExpressionType::ALIAS: {
//                auto &alias_expr = dynamic_cast<const BoundAlias &>(expr);
//                AddAggCallToContext(*alias_expr.child_);
//                return;
//            }
//            default:
//                break;
//        }
//>>>>>>> case_expression
    }
    plan->output_schema_ = std::make_shared<arrow::Schema>(field_vector);  
  }
  return plan;
}

void Planner::AddAggCallToContext(BoundExpression &expr, std::vector<std::string> &schema_names, std::string schema_name) {
    switch (expr.type_) {
        case ExpressionType::AGG_CALL: {
            auto &agg_call_expr = dynamic_cast<BoundAggCall &>(expr);
            // auto agg_name = fmt::format("__pseudo_agg#{}", ctx_.aggregations_.size());
            if (schema_name.empty())
              schema_name = fmt::format("__pseudo_agg#{}", ctx_.aggregations_.size());
            auto agg_call =
                    BoundAggCall(schema_name, agg_call_expr.is_distinct_, std::vector<std::unique_ptr<BoundExpression>>{});
            // Replace the agg call in the original bound expression with a pseudo one, add agg call to the context.
            ctx_.aggregations_.push_back(std::make_unique<BoundAggCall>(std::exchange(agg_call_expr, std::move(agg_call))));
            schema_names.push_back(schema_name);
            return;
        }
        case ExpressionType::COLUMN_REF: {
            return;
        }
        case ExpressionType::BINARY_OP: {
            auto &binary_op_expr = dynamic_cast<BoundBinaryOp &>(expr);
            AddAggCallToContext(*binary_op_expr.larg_, schema_names);
            AddAggCallToContext(*binary_op_expr.rarg_, schema_names);
            return;
        }
        case ExpressionType::CONSTANT: {
            return;
        }
        case ExpressionType::ALIAS: {
            auto &alias_expr = dynamic_cast<const BoundAlias &>(expr);
            AddAggCallToContext(*alias_expr.child_, schema_names, alias_expr.alias_);
            return;
        }
        default:
            return;
    }
}

/* 对于查询
SELECT region, SUM(sales)
FROM orders
GROUP BY region;

proj_exprs 表示	region_expr, sales_expr
group_by_expr 表示 region_expr
agg_exprs 表示 sales_expr
现在sum里的东西不能是group by的列，否则会出问题
*/
auto Planner::PlanSelectAgg(const SelectStatement &statement,
                            AbstractPlanNodeRef child) -> AbstractPlanNodeRef {
    auto guard = NewContext();
    ctx_.allow_aggregation_ = true;
    std::vector<std::string> agg_schema_names;
    // 先提取出所有agg表达式
    // having先进ctx，也会先出
    if (!statement.having_->IsInvalid()) {
        AddAggCallToContext(*statement.having_, agg_schema_names);
    }

    for (auto &item : statement.select_list_) {
        AddAggCallToContext(*item, agg_schema_names);
    }
    std::vector<AbstractExpressionRef> proj_exprs;
    std::vector<AbstractExpressionRef> group_by_expr;
    std::vector<std::string> outputschema_col_names;

  // 先处理groupby (output的前半部分)
  size_t proj_idx = 0;
  for (const auto &expr : statement.group_by_) {
      auto [col_name, ab_expr] = PlanExpression(*expr, {child});
      group_by_expr.emplace_back(std::make_shared<ColumnValueExpression>(0, proj_idx++, ab_expr->GetReturnType()));
      proj_exprs.emplace_back(std::move(ab_expr));
      // 这里存在别名问题，SELECT 中对 group by 列做别名，无法正确生效。例如：OptimizerTPCHTestQ2FourThreadComplete
      /// @TODO: 通过在 Aggregate 上套 Projection 来解决
      outputschema_col_names.emplace_back(std::move(col_name));
  }

  // 再处理后半部分的agg
  for (std::string &schema_name : agg_schema_names) {
    outputschema_col_names.emplace_back(std::move(schema_name));
  }

  std::vector<AbstractExpressionRef> agg_exprs;
  std::vector<AggregationType> agg_types;

  size_t term_idx = 0;
  for (const auto &item : ctx_.aggregations_) {

      const auto &agg_call = dynamic_cast<const BoundAggCall &>(*item);
      
      std::vector<AbstractExpressionRef> exprs;
      for (const auto &arg : agg_call.args_) {
          if(agg_call.func_name_ == "count_star") { break;}
          auto [_, ret] = PlanExpression(*arg, {child});
          exprs.emplace_back(std::move(ret));
      }
      auto func_name = agg_call.func_name_;
      if (exprs.empty()) {
          assert(func_name == "count_star");
          agg_types.push_back(AggregationType::CountStarAggregate);
      } else if (exprs.size() == 1) {
          if (func_name == "min") {
              agg_types.push_back(AggregationType::MinAggregate);
          }
          if (func_name == "max") {
              agg_types.push_back(AggregationType::MaxAggregate);
          }
          if (func_name == "sum") {
              agg_types.push_back(AggregationType::SumAggregate);
          }
          if (func_name == "count") {
              agg_types.push_back(AggregationType::CountAggregate);
          }
          if (func_name == "avg") {
              agg_types.push_back(AggregationType::AvgAggregate);
          }
      } else {
        spdlog::info("[{} : {}] agg表达式不能有超过一个参数");
        assert(0);
      }

      //  ctx_.expr_in_agg_.emplace_back(
      //          std::make_unique<ColumnValueExpression>(0, agg_begin_idx + term_idx - 1 , LogicalType::FLOAT));
      if (exprs.empty()) {
          // 用常量 1 表示 count(*) 的表达式，让 ProjectionPlanNode 生成对应列
          // @FIXME: 这里不能 count(1) 但是 count(1+1) 或者 count(column) 是可以的
          // 默认 count 第一列 不是合理的实现方式
          auto count_star_expr = std::make_shared<ConstantValueExpression>(Value(1));
          agg_exprs.emplace_back(std::make_shared<ColumnValueExpression>(0, 0, LogicalType::INTEGER));
          ctx_.expr_in_agg_.emplace_back(std::make_unique<ColumnValueExpression>(
              0, proj_idx, LogicalType::INTEGER));          
      } else {
          // 不能和groupby里的列一样，否则对相同列投影了两遍
          proj_exprs.emplace_back(std::move(exprs[0]));
          agg_exprs.emplace_back(std::make_shared<ColumnValueExpression>(0, proj_idx, proj_exprs.back()->GetReturnType()));
          ctx_.expr_in_agg_.emplace_back(std::make_unique<ColumnValueExpression>(
              0, proj_idx++, agg_exprs.back()->GetReturnType()));  
      }

  }

  // 处理完子agg表达式后，重做这一部分Plan，拥有proj、agg、filter(have)
  AbstractPlanNodeRef proj = std::make_shared<ProjectionPlanNode>(
    std::make_shared<arrow::Schema>(ProjectionPlanNode::InferProjectionSchema(proj_exprs)),
    proj_exprs, std::move(child));

  auto agg_output_schema = AggregationPlanNode::InferAggSchema(group_by_expr, agg_exprs, agg_types);


  AbstractPlanNodeRef plan = std::make_shared<AggregationPlanNode>(
          std::make_shared<arrow::Schema>(ProjectionPlanNode::RenameSchema(agg_output_schema, outputschema_col_names)), std::move(proj),
          std::move(group_by_expr), std::move(agg_exprs), std::move(agg_types));

  if (!statement.having_->IsInvalid()) {
      auto [_, expr] = PlanExpression(*statement.having_, {plan});
      plan = std::make_shared<FilterPlanNode>(std::make_shared<arrow::Schema>(plan->OutputSchema()), std::move(expr),
                                              std::move(plan));
  }

	 // return plan;
  std::vector<AbstractExpressionRef> final_exprs;
  std::vector<std::string> final_output_col_names;

  for (const auto &item : statement.select_list_) {
      auto [name, expr] = PlanExpression(*item, {plan});
      final_exprs.emplace_back(std::move(expr));
      final_output_col_names.emplace_back(std::move(name));
  }

  auto final_schema = ProjectionPlanNode::RenameSchema(
    ProjectionPlanNode::InferProjectionSchema(final_exprs), final_output_col_names);

  return std::make_shared<ProjectionPlanNode>(
      std::make_shared<arrow::Schema>(final_schema),
      std::move(final_exprs), std::move(plan));
}

auto Planner::PlanCase(const BoundCase &expr,
					   const std::vector<AbstractPlanNodeRef> &children)
	-> AbstractExpressionRef {


	std::vector<AbstractExpressionRef> when_checks;
	std::vector<AbstractExpressionRef> then_checks;

	for(auto &check : expr.case_checks) {

		auto [_1, when] = PlanExpression(*check.when_expr, children);
		auto [_2, then] = PlanExpression(*check.then_expr, children);
		when_checks.emplace_back(std::move(when));
		then_checks.emplace_back(std::move(then));
	}
	auto [_3, els] = PlanExpression(*expr.else_expr, children);


	return std::make_shared<CaseExpression>(std::move(when_checks),std::move(then_checks),std::move(els));
}

} // namespace DaseX