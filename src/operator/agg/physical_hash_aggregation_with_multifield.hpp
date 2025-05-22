#pragma once

#include "physical_operator.hpp"
#include "physical_operator_type.hpp"
#include "aggregation_state.hpp"
#include "aggregation_hashtable.hpp"
#include "execute_context.hpp"
#include "pipeline_group.hpp"
#include <arrow/api.h>
#include <vector>

namespace DaseX {

class MultiFieldAggLocalSinkState : public LocalSinkState {
public:
    int64_t processed_chunks = 0; // 记录处理的Chunk块数（RecordBatch）
    int64_t processed_rows = 0;	  // 记录处理的数据总行数
    int partition_nums = 0;
    std::shared_ptr<RadixPartitionTable> partition_table; // 记录每个线程聚合的中间数据
    std::shared_ptr<arrow::Schema> result_schema;
    std::shared_ptr<arrow::RecordBatch> result;					  // 聚合结果
public:
    MultiFieldAggLocalSinkState(std::vector<int> group_set, std::vector<int> agg_set,
                                std::vector<int> star_bitmap,
                                std::vector<AggFunctionType> aggregate_function_types) {
        partition_table = std::make_shared<RadixPartitionTable>(group_set, agg_set, star_bitmap, aggregate_function_types);
    }
};

class MultiFieldAggLocalSourceState : public LocalSourceState {
public:
    int64_t idx_nums = 0;
    int64_t result_size = 0;
    std::shared_ptr<arrow::RecordBatch> result; // 聚合结果
public:
    MultiFieldAggLocalSourceState(MultiFieldAggLocalSinkState &input) : result(input.result) {
        result_size = input.result->num_rows();
    }
};

class PhysicalMultiFieldHashAgg : public PhysicalOperator {
public:
    static constexpr const PhysicalOperatorType TYPE = PhysicalOperatorType::MULTI_FIELD_HASH_GROUP_BY;
    std::vector<int> group_set; // GroupBy字段
    std::vector<int> agg_set; // 聚合字段
    std::vector<int> star_bitmap; // 对应字段是否为 ‘*’ 表达式
    std::vector<AggFunctionType> aggregate_function_types;
public:
    PhysicalMultiFieldHashAgg(std::vector<int> group_set,
                              std::vector<int> agg_set, std::vector<int> star_bitmap,
                              std::vector<AggFunctionType> aggregate_function_types);
public:
    // Common interface
    std::shared_ptr<PhysicalOperator> Copy(int arg) override;
    // Source 接口实现
    SourceResultType get_data(std::shared_ptr<ExecuteContext> &execute_context, LocalSourceState &input) const override;
    std::shared_ptr<LocalSourceState> GetLocalSourceState() const override;
    bool is_source() const override { return true; }
    bool ParallelSource() const override { return true; }
    // Sink 接口实现
    SinkResultType sink(std::shared_ptr<ExecuteContext> &execute_context, LocalSinkState &output) override;
    std::shared_ptr<LocalSinkState> GetLocalSinkState() const override;
    bool is_sink() const override { return true; }
    bool ParallelSink() const override { return true; }
private:
    void local_agg(std::shared_ptr<arrow::RecordBatch> &chunk, MultiFieldAggLocalSinkState &state) const;
    arrow::Status global_agg(MultiFieldAggLocalSinkState &lstate, std::shared_ptr<PipelineGroup> &pipeline_group,
                             std::shared_ptr<Pipeline> &pipeline, size_t pipeline_id) const;
    void AddOneRowToValueArray(std::vector<std::vector<std::shared_ptr<Value>>> &key_values,
                               std::shared_ptr<EntrySet> &entrySet, int row_idx, int column_nums) const;
    arrow::Status FillArrayVector(std::vector<std::shared_ptr<arrow::Array>> &columns,
                         std::vector<std::vector<std::shared_ptr<Value>>> &values,
                         int column_nums) const;
}; // PhysicalMultiFieldHashAgg








} // DaseX
