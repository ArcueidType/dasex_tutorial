/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2023-12-20 15:32:16
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-01-07 21:36:31
 * @FilePath: /task_sche_parser/src/planner/planner.hpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#pragma once

#include "abstract_expression.h"
#include "abstract_plan.h"
#include "../include/binder/binder.h"
#include "bound_subquery.h"
#include "subquery_expression.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../include/binder/binder.h"
#include "abstract_expression.h"
#include "abstract_plan.h"
#include "bound_case.h"
#include "bound_func_call.h"

namespace DaseX {

class BoundStatement;
class SelectStatement;
class DeleteStatement;
class AbstractPlanNode;
class InsertStatement;
class BoundExpression;
class BoundTableRef;
class BoundBinaryOp;
class BoundConstant;
class BoundColumnRef;
class BoundUnaryOp;
class BoundBaseTableRef;
class BoundSubqueryRef;
class BoundCrossProductRef;
class BoundJoinRef;
class BoundExpressionListRef;
class BoundAggCall;
class BoundCTERef;
class ColumnValueExpression;

class PlannerContext {
public:
  PlannerContext() = default;
  bool allow_aggregation_{false};
  size_t next_aggregation_{0};
  std::vector<std::unique_ptr<BoundExpression>> aggregations_;

  std::vector<AbstractExpressionRef> expr_in_agg_;

  const CTEList *cte_list_{nullptr};
};



class Planner {
public:
  explicit Planner(std::weak_ptr<CataLog> catalog) : catalog_(catalog) {}

  void PlanQuery(const BoundStatement &statement);

  auto PlanSelect(const SelectStatement &statement) -> AbstractPlanNodeRef;

  auto PlanExpression(const BoundExpression &expr,
                      const std::vector<AbstractPlanNodeRef> &children)
      -> std::tuple<std::string, AbstractExpressionRef>;

  auto PlanBinaryOp(const BoundBinaryOp &expr,
                    const std::vector<AbstractPlanNodeRef> &children)
      -> AbstractExpressionRef;

  auto PlanColumnRef(const BoundColumnRef &expr,
                     const std::vector<AbstractPlanNodeRef> &children)
      -> std::tuple<std::string, std::shared_ptr<ColumnValueExpression>>;

  auto PlanConstant(const BoundConstant &expr,
                    const std::vector<AbstractPlanNodeRef> &children)
      -> AbstractExpressionRef;


  auto PlanSubqueryExpression(const BoundSubquery &expr,
    const std::vector<AbstractPlanNodeRef> &children) -> AbstractExpressionRef;

  auto PlanCase(const BoundCase &expr,
					const std::vector<AbstractPlanNodeRef> &children)
	  -> AbstractExpressionRef;


  auto PlanTableRef(const BoundTableRef &table_ref) -> AbstractPlanNodeRef;

  auto PlanBaseTableRef(const BoundBaseTableRef &table_ref)
      -> AbstractPlanNodeRef;

  auto PlanSubquery(const BoundSubqueryRef &table_ref, const std::string &alias) -> AbstractPlanNodeRef;

  auto PlanCrossProductRef(const BoundCrossProductRef &table_ref)
      -> AbstractPlanNodeRef;

  auto PlanCTERef(const BoundCTERef &table_ref) -> AbstractPlanNodeRef;

  auto PlanJoinRef(const BoundJoinRef &table_ref) -> AbstractPlanNodeRef;

  auto PlanSubqueryRef(const BoundSubqueryRef &table_ref) -> AbstractPlanNodeRef;
  
  void PlanSubqueries(AbstractExpressionRef &expr, AbstractPlanNodeRef &plan);

  auto PlanUncorrelatedSubquery(SubqueryExpression &expr, AbstractPlanNodeRef &root, AbstractPlanNodeRef plan)
      -> AbstractExpressionRef;

  auto PlanSubquery(SubqueryExpression &expr, AbstractPlanNodeRef &plan) -> AbstractExpressionRef;


  auto PlanSelectAgg(const SelectStatement &statement,
                     AbstractPlanNodeRef child) -> AbstractPlanNodeRef;


  void AddAggCallToContext(BoundExpression &expr, std::vector<std::string> &schema_names, std::string schema_name = "");

  auto PlanFuncCall(const BoundFuncCall &expr, const std::vector<AbstractPlanNodeRef> &children)
	  -> AbstractExpressionRef;

//  void AddAggCallToContext(BoundExpression &expr);
//>>>>>>> case_expression

  AbstractPlanNodeRef plan_;

  std::weak_ptr<CataLog> catalog_;

  PlannerContext ctx_;

    class ContextGuard {
    public:
        explicit ContextGuard(PlannerContext *ctx) : old_ctx_(std::move(*ctx)), ctx_ptr_(ctx) {
            *ctx = PlannerContext();
            ctx->cte_list_ = old_ctx_.cte_list_;
        }
        ~ContextGuard() { *ctx_ptr_ = std::move(old_ctx_); }


    private:
        PlannerContext old_ctx_;
        PlannerContext *ctx_ptr_;
    };
    auto NewContext() -> ContextGuard { return ContextGuard(&ctx_); }
};

} // namespace DaseX