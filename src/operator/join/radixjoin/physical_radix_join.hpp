#pragma once

#include "arrow_help.hpp"
#include "common_macro.hpp"
#include "config.hpp"
#include "execute_context.hpp"
#include "operator_result_type.hpp"
#include "physical_operator.hpp"
#include "physical_operator_states.hpp"
#include "physical_operator_type.hpp"
#include "pipeline_group.hpp"
#include "time_help.hpp"
#include <arrow/api.h>
#include <chrono>
#include <sched.h>
#include "spdlog/spdlog.h"
#include <thread>
#include <unordered_map>
#include <vector>

namespace DaseX
{

// 这里hash_table必须要自己重新实现了，因为不确定Key的类型，所以不好使用自带的map。因为我不想将RadixJoinLocalSinkState实现为一个模板类
// 目前只支持int32_t作为key
class RadixJoinLocalSinkState : public LocalSinkState
{
public:
    int64_t processed_chunks = 0;								   // 记录处理的Chunk块数（RecordBatch）
    int64_t processed_rows = 0;									   // 记录处理的数据总行数
    std::vector<std::shared_ptr<arrow::RecordBatch>> buffer_chunk; // 缓存上游算子的处理结果
    // Radix分区变量
    int32_t partition_nums;																	   // 分区数量（必须为2的指数）
    int build_col_nums;																	   // build端的列数
    int probe_col_nums;																	   // probe端的列数
    std::vector<int32_t> histogram_build;													   // 直方图
    std::vector<int32_t> prefix_sum_build;													   // 前缀和
    std::vector<int32_t> histogram_probe;													   // 直方图
    std::vector<int32_t> prefix_sum_probe;													   // 前缀和
    std::vector<std::shared_ptr<arrow::RecordBatch>> output_build;							   // 记录分区重排后的结果
    std::vector<std::shared_ptr<arrow::RecordBatch>> output_probe;							   // 记录分区重排后的结果
    std::vector<std::unordered_map<int32_t, std::shared_ptr<arrow::RecordBatch>>> hash_tables; // build结果
    std::vector<std::shared_ptr<arrow::RecordBatch>> result;								   // probe结果
public:
    RadixJoinLocalSinkState(int32_t partition_nums_ = Numa::CORES);
};

class RadixJoinLocalSourceState : public LocalSourceState
{
public:
    uint64_t idx_nums = 0; // 需要读取的RecordBatch
    // 这里必须使用指针保留位置，运行时可获取实际结果
    std::vector<std::shared_ptr<arrow::RecordBatch>> *result; // join结果
public:
    RadixJoinLocalSourceState(RadixJoinLocalSinkState &input) : result(&(input.result)) {}
};

class PhysicalRadixJoin : public PhysicalOperator
{
public:
    static constexpr const PhysicalOperatorType TYPE = PhysicalOperatorType::RADIX_JOIN;
    int build_id;
    int probe_id;
    bool is_build;
    int32_t partition_nums;

public:
    PhysicalRadixJoin(int build_id, int probe_id, bool is_build = true, int32_t partition_nums = 4);

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
    /**
     * @description: 对缓存的数据进行Radix分区
     * @param {vector<shared_ptr<arrow::RecordBatch>>} &input           输入数据
     * @return {*}
     */
    void radix_partition(LocalSinkState &input, bool is_build) const;

    void build(LocalSinkState &input) const;

    void probe(std::shared_ptr<PipelineGroup> &pipe_group, LocalSinkState &input) const;

    std::shared_ptr<PhysicalOperator> Copy(int arg) override;

    void build_pipelines(std::shared_ptr<Pipeline> &current, std::shared_ptr<PipelineGroup> &pipelineGroup) override;
};

} // DaseX