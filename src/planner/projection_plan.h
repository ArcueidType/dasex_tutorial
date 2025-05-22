#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "abstract_expression.h"
#include "abstract_plan.h"
#include "catalog.hpp"

namespace DaseX {

/**
 * The ProjectionPlanNode represents a project operation.
 */
class ProjectionPlanNode : public AbstractPlanNode {
public:
  /**
   * Construct a new ProjectionPlanNode instance.
   * @param output The output schema of this projection node
   * @param expressions The expression to evaluate
   * @param child The child plan node
   */
  ProjectionPlanNode(std::shared_ptr<const arrow::Schema> output,
                     std::vector<AbstractExpressionRef> expressions,
                     AbstractPlanNodeRef child)
      : AbstractPlanNode(std::move(output), {std::move(child)}),
        expressions_(std::move(expressions)) {}

  /** @return The type of the plan node */
  auto GetType() const -> PlanType override { return PlanType::Projection; }

  /** @return The child plan node */
  auto GetChildPlan() const -> AbstractPlanNodeRef { return GetChildAt(0); }

  /** @return Projection expressions */
  auto GetExpressions() const -> const std::vector<AbstractExpressionRef> & {
    return expressions_;
  }

  static auto
  InferProjectionSchema(const std::vector<AbstractExpressionRef> &expressions)
      -> arrow::Schema;

  static auto RenameSchema(const arrow::Schema &schema,
                           const std::vector<std::string> &col_names)
      -> arrow::Schema;

  PLAN_NODE_CLONE_WITH_CHILDREN(ProjectionPlanNode);

  std::vector<AbstractExpressionRef> expressions_;

  auto ToString() -> std::string override {
    std::string exprs;
    if (!expressions_.empty()) {
      exprs += " Exprs:";
    }
    for (AbstractExpressionRef &expr : expressions_) {
      exprs += " " + expr->ToString();
    }
    return "[Projection" + exprs + "]";
  }
};

} // namespace DaseX
