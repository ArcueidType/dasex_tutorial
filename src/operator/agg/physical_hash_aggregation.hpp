#pragma once

#include "physical_operator.hpp"
#include "operator_result_type.hpp"
#include "physical_operator_type.hpp"
#include "physical_operator_states.hpp"
#include "aggregation_state.hpp"
#include "execute_context.hpp"
#include "common_macro.hpp"
#include "arrow_help.hpp"
#include "pipeline_group.hpp"
#include "config.hpp"
#include "time_help.hpp"
#include <string>
#include <arrow/api.h>
#include <unordered_map>
#include <vector>
#include <sched.h>
#include <spdlog/spdlog.h>

namespace DaseX {

class AggLocalSinkState : public LocalSinkState {
public:
    int64_t processed_chunks = 0; // 记录处理的Chunk块数（RecordBatch）
    int64_t processed_rows = 0;	  // 记录处理的数据总行数
    int partition_nums = 0;
    std::vector<std::unordered_map<int32_t, std::vector<std::unique_ptr<Aggstate>>>> partitioned_hash_tables; // 记录每个线程聚合的中间数据
    std::shared_ptr<arrow::Schema> result_schema;
    std::shared_ptr<arrow::RecordBatch> result;					  // 聚合结果
    // 当不存在GroupBy字段时使用
    std::vector<std::shared_ptr<arrow::RecordBatch>> cache_data;
    std::vector<std::unique_ptr<Aggstate>> local_state;
public:
    AggLocalSinkState() {}
};

class AggLocalSourceState : public LocalSourceState {
public:
    uint64_t idx_nums = 0;
    int64_t result_size = 0;
    std::shared_ptr<arrow::RecordBatch> result; // 聚合结果
public:
    AggLocalSourceState(AggLocalSinkState &input) : result(input.result) {
        result_size = input.result->num_rows();
    }
};

class PhysicalHashAgg : public PhysicalOperator {
public:
    static constexpr const PhysicalOperatorType TYPE = PhysicalOperatorType::HASH_GROUP_BY;
    int16_t group_by_id = -1; // 分组的字段的ColumnID
    std::vector<AggFunctionType> aggregate_function_types;        // 聚合函数类型
    std::vector<int32_t> group_item_idxs;            			  // 聚合的字段
    bool is_star;                                                 // 聚合字段为*
    int precision = 2;                                            // 指定聚合时保留数据的精度
public:
    PhysicalHashAgg(int16_t group_by_id_,
                    const std::vector<AggFunctionType> &aggregate_function_types_,
                    const std::vector<int32_t> &group_item_idxs_,
                    bool is_star_ = false)
            : PhysicalOperator(PhysicalOperatorType::HASH_GROUP_BY),
              group_by_id(group_by_id_),
              aggregate_function_types(aggregate_function_types_),
              group_item_idxs(group_item_idxs_),
              is_star(is_star_) {}

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
    bool is_sink() const override { return true; }
    bool ParallelSink() const override { return true; }
    std::shared_ptr<LocalSinkState> GetLocalSinkState() const override;
private:
    void local_agg(std::shared_ptr<arrow::RecordBatch> &chunk, AggLocalSinkState &state) const;
    void local_agg_without_groupby(AggLocalSinkState &lstate);
    arrow::Status global_agg(AggLocalSinkState &lstate, std::shared_ptr<PipelineGroup> &pipeline_group, std::shared_ptr<Pipeline> &pipeline, size_t pipeline_id) const;
    arrow::Status global_agg_without_groupby(AggLocalSinkState &lstate, std::shared_ptr<PipelineGroup> &pipeline_group, std::shared_ptr<Pipeline> &pipeline, int pipeline_id);
};

} // DaseX