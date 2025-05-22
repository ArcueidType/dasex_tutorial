/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2024-01-04 16:49:37
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-01-07 20:02:03
 * @FilePath: /task_sche-binder/src/planner/nest_loop_join_plan.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "abstract_expression.h"
#include "abstract_plan.h"
#include "../include/binder/table_ref/bound_join_ref.h"
#include "catalog.hpp"

namespace DaseX {

class NestedLoopJoinPlanNode : public AbstractPlanNode {
public:
  NestedLoopJoinPlanNode(std::shared_ptr<const arrow::Schema> output_schema,
                         AbstractPlanNodeRef left, AbstractPlanNodeRef right,
                         AbstractExpressionRef predicate, JoinType join_type)
      : AbstractPlanNode(std::move(output_schema),
                         {std::move(left), std::move(right)}),
        predicate_(std::move(predicate)), join_type_(join_type) {}

  /** @return The type of the plan node */
  auto GetType() const -> PlanType override { return PlanType::NestedLoopJoin; }

  /** @return The predicate to be used in the nested loop join */
  auto Predicate() const -> const AbstractExpression & { return *predicate_; }

  /** @return The join type used in the nested loop join */
  auto GetJoinType() const -> JoinType { return join_type_; };

  /** @return The left plan node of the nested loop join, TODO: always the
   * smaller table  */
  auto GetLeftPlan() const -> AbstractPlanNodeRef { return GetChildAt(0); }

  /** @return The right plan node of the nested loop join */
  auto GetRightPlan() const -> AbstractPlanNodeRef { return GetChildAt(1); }

  static auto InferJoinSchema(const AbstractPlanNode &left,
                              const AbstractPlanNode &right) -> arrow::Schema;

  PLAN_NODE_CLONE_WITH_CHILDREN(NestedLoopJoinPlanNode);

  /** The join predicate */
  AbstractExpressionRef predicate_;

  /** The join type */
  JoinType join_type_;

  auto ToString() -> std::string override {
    std::string info;
    switch (join_type_) {
      case JoinType::ANTI:
        info += " ANTI";
        break;
      case JoinType::SEMI:
        info += " SEMI";
        break;
      case JoinType::INNER:
        info += " INNER";
        break;
      default:
        break;
    }
    info += predicate_ ? " Pred: " + predicate_->ToString() : "";
    return "[NestedLoopJoin" + info + "]";
  }

  // auto ToString() -> std::string override { return "[NestedLoopJoin]"; }
};

} // namespace DaseX
