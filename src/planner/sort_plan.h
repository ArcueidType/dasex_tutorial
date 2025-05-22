//
// Created by zyy on 7/23/24.
//
#pragma once

#include "abstract_plan.h"
#include "abstract_expression.h"
#include "bound_order_by.h"

/**
 * The SortPlanNode represents a sort operation. It will sort the input with
 * the given predicate.
 *
 */

namespace DaseX {
class SortPlanNode : public AbstractPlanNode {
public:
    /**
     * Construct a new SortPlanNode instance.
     * @param output The output schema of this sort plan node
     * @param child The child plan node
     * @param order_bys The sort expressions and their order by types.
     */
    SortPlanNode(std::shared_ptr<const arrow::Schema>  output, AbstractPlanNodeRef child,
                 std::vector<std::pair<OrderByType, AbstractExpressionRef>> order_bys)
            : AbstractPlanNode(std::move(output), {std::move(child)}), order_bys_(std::move(order_bys)) {}

    /** @return The type of the plan node */
    auto GetType() const -> PlanType override { return PlanType::Sort; }

    /** @return The child plan node */
    auto GetChildPlan() const -> AbstractPlanNodeRef {
        return GetChildAt(0);
    }

    /** @return Get sort by expressions */
    auto GetOrderBy() const -> const std::vector<std::pair<OrderByType, AbstractExpressionRef>> & { return order_bys_; }

    PLAN_NODE_CLONE_WITH_CHILDREN(SortPlanNode);

    std::vector<std::pair<OrderByType, AbstractExpressionRef>> order_bys_;

    auto ToString() -> std::string override { return "[SortPlanNode]"; }
};

}