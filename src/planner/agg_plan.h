
#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "abstract_expression.h"
#include "abstract_plan.h"
#include "catalog.hpp"
#include "fmt/format.h"

namespace DaseX {

enum class AggregationType {
  CountStarAggregate,
  CountAggregate,
  SumAggregate,
  MinAggregate,
  MaxAggregate,
  AvgAggregate
};

/**
 * AggregationPlanNode represents the various SQL aggregation functions.
 * For example, COUNT(), SUM(), MIN() and MAX().
 */
class AggregationPlanNode : public AbstractPlanNode {
public:

  AggregationPlanNode(std::shared_ptr<const arrow::Schema> output_schema,
                      AbstractPlanNodeRef child,
                      std::vector<AbstractExpressionRef> group_bys,
                      std::vector<AbstractExpressionRef> aggregates,
                      std::vector<AggregationType> agg_types)
      : AbstractPlanNode(std::move(output_schema), {std::move(child)}),
        group_bys_(std::move(group_bys)), aggregates_(std::move(aggregates)),
        agg_types_(std::move(agg_types)) {}

  auto GetType() const -> PlanType override { return PlanType::Aggregation; }

  auto GetChildPlan() const -> AbstractPlanNodeRef { return GetChildAt(0); }

  auto GetGroupByAt(uint32_t idx) const -> const AbstractExpressionRef & {
    return group_bys_[idx];
  }

  auto GetGroupBys() const -> const std::vector<AbstractExpressionRef> & {
    return group_bys_;
  }

  auto GetAggregateAt(uint32_t idx) const -> const AbstractExpressionRef & {
    return aggregates_[idx];
  }

  auto GetAggregates() const -> const std::vector<AbstractExpressionRef> & {
    return aggregates_;
  }

  auto GetAggregateTypes() const -> const std::vector<AggregationType> & {
    return agg_types_;
  }



  static auto
  InferAggSchema(const std::vector<AbstractExpressionRef> &group_bys,
                 const std::vector<AbstractExpressionRef> &aggregates,
                 const std::vector<AggregationType> &agg_types)
      -> arrow::Schema;

  PLAN_NODE_CLONE_WITH_CHILDREN(AggregationPlanNode);

  /** The GROUP BY expressions */
  std::vector<AbstractExpressionRef> group_bys_;
  /** The aggregation expressions */
  std::vector<AbstractExpressionRef> aggregates_;
  /** The aggregation types */
  std::vector<AggregationType> agg_types_;

  auto ToString() -> std::string override {
    std::string predicate;
    predicate += " group by: (";
    for (const auto &group_by: group_bys_) {
      predicate += " " + group_by->ToString();
    }
    predicate += " ) aggregates: (";
    for (const auto &aggregate: aggregates_) {
      predicate += " " + aggregate->ToString();
    }
    predicate += " )";
    // std::string predicate = predicate_ ? " " + predicate_->ToString() : "";
    return "[Aggreagation" + predicate + "]";
  }
};

/** AggregateKey represents a key in an aggregation operation */
struct AggregateKey {
  /** The group-by values */
  std::vector<int> group_bys_;
  // todo: Value 比较
};

/** AggregateValue represents a value for each of the running aggregates */
struct AggregateValue {
  /** The aggregate values */
  std::vector<int> aggregates_;
};

}; // namespace DaseX
