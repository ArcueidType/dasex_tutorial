//
// Created by cwl on 7/31/2024.
//
#pragma once

#include "physical_operator.hpp"
#include "physical_operator_type.hpp"
#include "expression_type.hpp"
#include "value_type.hpp"

namespace DaseX {

using RecordBatchPtr =  std::shared_ptr<arrow::RecordBatch>;
using RecordBatchPtrVector = std::vector<std::shared_ptr<arrow::RecordBatch>>;
using ValuePtr = std::shared_ptr<Value>;
using ValuePtrVector = std::vector<std::shared_ptr<Value>>;

class NestedLoopJoinLocalSinkState : public LocalSinkState {
public:
    int64_t total_rows = 0;
    RecordBatchPtrVector cache_result;
public:
    NestedLoopJoinLocalSinkState() {}
};

class PhysicalNestedLoopJoin : public PhysicalOperator {
public:
    static constexpr const PhysicalOperatorType TYPE = PhysicalOperatorType::NESTED_LOOP_JOIN;
    JoinTypes join_type = JoinTypes::INNER;
    // join condition
    std::vector<int> build_ids;
    std::vector<int> probe_ids;
    // 比较类型，NLJ支持任意比较连接，包括: =, <, >, <>, >=, <= 六种
    std::vector<ExpressionTypes> compare_types;
    // 构建表的Schema
    std::shared_ptr<arrow::Schema> build_schema;
    // join结果的Schema
    std::shared_ptr<arrow::Schema> return_schema;
    // For debug
    int process_nums = 0;
    std::vector<std::shared_ptr<arrow::RecordBatch>> join_result;
public:
    PhysicalNestedLoopJoin(JoinTypes join_type,
                           std::vector<int> build_ids,
                           std::vector<int> probe_ids,
                           std::vector<ExpressionTypes> compare_types);

    // Sink 接口
    SinkResultType sink(std::shared_ptr<ExecuteContext> &execute_context, LocalSinkState &output) override;
    std::shared_ptr<LocalSinkState> GetLocalSinkState() const override;
    bool is_sink() const override { return true; }
    bool ParallelSink() const override { return true; }
    // Operator 接口
    OperatorResultType execute_operator(std::shared_ptr<ExecuteContext> &execute_context) override;
    bool is_operator() const override { return true; }
    bool ParallelOperator() const override { return true; }
    // Common 接口
    void build_pipelines(std::shared_ptr<Pipeline> &current, std::shared_ptr<PipelineGroup> &pipelineGroup) override;
    std::shared_ptr<PhysicalOperator> Copy(int arg) override;
private:
    /**
     * @description:
     * 获取INNER JOIN的nlj结果
     * @param {RecordBatchPtrVector &} cache_result    缓存的Build端结果
     * @param {RecordBatchPtr &} data_chunk            输入数据块
     * @param {RecordBatchPtr &} result          Join结果
     */
    void GetInnerJoinResult(RecordBatchPtrVector &cache_result, RecordBatchPtr &data_chunk, RecordBatchPtr &result) const;
    // TODO: 后续需要再实现
    void GetLeftJoinResult(RecordBatchPtrVector &cache_result, RecordBatchPtr &data_chunk, RecordBatchPtr &result) const;
    void GetRightJoinResult(RecordBatchPtrVector &cache_result, RecordBatchPtr &data_chunk, RecordBatchPtr &result) const;
    void GetSemiJoinResult(RecordBatchPtrVector &cache_result, RecordBatchPtr &data_chunk, RecordBatchPtr &result) const;
    void GetAntiJoinResult(RecordBatchPtrVector &cache_result, RecordBatchPtr &data_chunk, RecordBatchPtr &result) const;
};

} // DaseX
