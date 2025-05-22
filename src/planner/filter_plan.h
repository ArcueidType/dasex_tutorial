
#pragma once

#include <memory>
#include <string>
#include <utility>

#include "abstract_expression.h"
#include "abstract_plan.h"
#include "catalog.hpp"

namespace DaseX {

class FilterPlanNode : public AbstractPlanNode {
public:
  FilterPlanNode(std::shared_ptr<const arrow::Schema> output,
                 AbstractExpressionRef predicate, AbstractPlanNodeRef child)
      : AbstractPlanNode(std::move(output), {std::move(child)}),
        predicate_{std::move(predicate)} {}

  auto GetType() const -> PlanType override { return PlanType::Filter; }

  auto GetPredicate() const -> const AbstractExpressionRef & {
    return predicate_;
  }

  auto GetChildPlan() const -> AbstractPlanNodeRef { return GetChildAt(0); }

  PLAN_NODE_CLONE_WITH_CHILDREN(FilterPlanNode);

  /** The predicate that all returned tuples must satisfy */
  AbstractExpressionRef predicate_;
  
  auto ToString() -> std::string override {
    std::string predicate = predicate_ ? " " + predicate_->ToString() : "";
    return "[Filter" + predicate + "]";
  }

};

} // namespace DaseX
