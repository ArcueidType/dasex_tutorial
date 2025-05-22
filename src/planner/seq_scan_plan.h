/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2024-01-02 16:53:22
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-01-02 16:53:23
 * @FilePath: /task_sche-binder/src/planner/seq_scan_plan.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#pragma once

#include <istream>
#include <memory>
#include <string>
#include <utility>

#include "abstract_expression.h"
#include "abstract_plan.h"
#include "../include/binder/table_ref/bound_base_table_ref.h"
#include "catalog.hpp"

namespace DaseX {

/**
 * The SeqScanPlanNode represents a sequential table scan operation.
 */
class SeqScanPlanNode : public AbstractPlanNode {
public:
  SeqScanPlanNode(std::shared_ptr<const arrow::Schema> output,
                  std::string table_name,
                  AbstractExpressionRef filter_predicate = nullptr)
      : AbstractPlanNode(std::move(output), {}),
        table_name_(std::move(table_name)),
        filter_predicate_(std::move(filter_predicate)) {}

  /** @return The type of the plan node */
  auto GetType() const -> PlanType override { return PlanType::SeqScan; }

  static auto InferScanSchema(const BoundBaseTableRef &table_ref)
      -> arrow::Schema;

  PLAN_NODE_CLONE_WITH_CHILDREN(SeqScanPlanNode);

  /** The table name */
  std::string table_name_;

  AbstractExpressionRef filter_predicate_;

  auto ToString() -> std::string override {
    std::string predicate = filter_predicate_ ? " Pred: " + filter_predicate_->ToString() : "";
    predicate += " schema_size: " + std::to_string(output_schema_->num_fields());
    // for (const auto& field : output_schema_->fields())
    // {
    //   predicate += " " + field->name();
    // }
    
    return "[SeqScan(" + table_name_ + ")" + predicate + "]"; }
};

} // namespace DaseX