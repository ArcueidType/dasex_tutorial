/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2024-01-03 10:05:45
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-01-22 15:23:15
 * @FilePath: /task_sche-binder/src/planner/plan_node.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "catalog.hpp"
#include "logical_type.hpp"
#include "nest_loop_join_plan.h"
#include "planner.hpp"
#include "projection_plan.h"
#include "seq_scan_plan.h"
#include "agg_plan.h"
#include <arrow/type_fwd.h>
#include <cstdio>

namespace DaseX {

auto SeqScanPlanNode::InferScanSchema(const BoundBaseTableRef &table)
    -> arrow::Schema {
  arrow::FieldVector filed_vector;
  std::vector<std::shared_ptr<Field>> fileds;
  auto temp_schema = table.schema_;

  for (auto f : temp_schema.field_names()) {
    std::shared_ptr<arrow::Field> field_ = temp_schema.GetFieldByName(f);
    std::string name = field_->name();

    auto col_name = table.GetBoundTableName() + "." + name;
//    printf("in SeqScanPlanNode InferScanSchema , check table %s field %s\n",
//           table.GetBoundTableName().c_str(), name.c_str());

    auto col_type = field_->type();

    if(field_->type() == arrow::int32()) {
        fileds.push_back(
                std::move(std::make_shared<Field>(col_name, LogicalType::INTEGER)));
    } else if (col_type == arrow::float32() ) {
        fileds.push_back(
                std::move(std::make_shared<Field>(col_name, LogicalType::FLOAT)));
    } else if (col_type == arrow::float64() ) {
        fileds.push_back(
                std::move(std::make_shared<Field>(col_name, LogicalType::DOUBLE)));
    } else {
        // 目前else 为 string
        fileds.push_back(
                std::move(std::make_shared<Field>(col_name, LogicalType::STRING)));
    }

  }

  for (auto i = 0; i < fileds.size(); i++) {
    filed_vector.push_back(fileds[i]->field);
  }
  auto schema = arrow::Schema(filed_vector);
  return arrow::Schema(schema);
}

// 构建 Output
auto AggregationPlanNode::InferAggSchema(const std::vector<AbstractExpressionRef> &group_bys,
                                          const std::vector<AbstractExpressionRef> &aggregates,
                                          const std::vector<AggregationType> &agg_types) -> arrow::Schema {
    arrow::FieldVector filed_vector;
    std::vector<std::shared_ptr<Field>> output;
    output.reserve(group_bys.size() + aggregates.size());
    for (const auto &column : group_bys) {
        output.emplace_back(std::move(std::make_shared<Field>("<unnamed>",column->GetReturnType())));
    }
    for (size_t idx = 0; idx < aggregates.size(); idx++) {
        output.emplace_back(std::move(std::make_shared<Field>("<unnamed>", LogicalType::FLOAT)));
    }
    for (auto i = 0; i < output.size(); i++) {
        filed_vector.push_back(output[i]->field);
    }
    return arrow::Schema(filed_vector);
}

auto NestedLoopJoinPlanNode::InferJoinSchema(const AbstractPlanNode &left,
                                             const AbstractPlanNode &right)
    -> arrow::Schema {
  arrow::FieldVector filed_vector;
  std::vector<std::shared_ptr<Field>> fileds;

  auto temp_schema = left.output_schema_;

  for (auto f : temp_schema->field_names()) {
    std::shared_ptr<arrow::Field> field_ = temp_schema->GetFieldByName(f);
    fileds.push_back(
        std::move(std::make_shared<Field>(f, LogicalType::INTEGER)));
//    printf("in njl left InferScanSchema , check field %s\n", f.c_str());
  }

  temp_schema = right.output_schema_;
  for (auto f : temp_schema->field_names()) {
    std::shared_ptr<arrow::Field> field_ = temp_schema->GetFieldByName(f);
    std::string name = field_->name();
    fileds.push_back(
        std::move(std::make_shared<Field>(f, LogicalType::INTEGER)));
    printf("in njl right InferScanSchema , check field %s\n", name.c_str());
  }
  for (auto i = 0; i < fileds.size(); i++) {
    filed_vector.push_back(fileds[i]->field);
  }
  auto schema = arrow::Schema(filed_vector);

//  for (auto f : schema.field_names()) {
//    printf("final join outputschema : %s\n", f.c_str());
//  }
  return arrow::Schema(schema);
}

auto ProjectionPlanNode::InferProjectionSchema(
    const std::vector<AbstractExpressionRef> &expressions) -> arrow::Schema {
  arrow::FieldVector filed_vector;
  std::vector<std::shared_ptr<Field>> fileds;

  for (const auto &expr : expressions) {
     auto type = expr->GetReturnType();
     fileds.push_back(
        std::make_shared<Field>("unnamed_expr", type));
  }

  for (auto i = 0; i < fileds.size(); i++) {
    filed_vector.push_back(fileds[i]->field);
  }

  auto schema = arrow::Schema(filed_vector);
  return schema;
}

auto ProjectionPlanNode::RenameSchema(const arrow::Schema &schema,
                                      const std::vector<std::string> &col_names)
    -> arrow::Schema {

    if (col_names.size() != schema.num_fields()) {
        throw std::invalid_argument("The number of column names does not match the number of fields in the schema.");
    }

    arrow::FieldVector field_vector;

    // 按顺序遍历 schema 中的字段，并重命名为 col_names 中的名字
    for (size_t idx = 0; idx < schema.num_fields(); ++idx) {
        auto field = schema.field(idx);
        auto new_field = std::make_shared<arrow::Field>(col_names[idx], field->type(), field->nullable(), field->metadata());
        field_vector.push_back(new_field);
    }


    return arrow::Schema(field_vector);
}


} // namespace DaseX