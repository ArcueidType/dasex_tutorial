#pragma once

#include "physical_operator.hpp"
#include "physical_operator_type.hpp"
#include "join_hashtable.hpp"
#include "expression_type.hpp"
#include <set>
#include <chrono>

namespace DaseX {

class HashJoinLocalSinkState : public LocalSinkState {
public:
    JoinTypes join_type;
    std::vector<int> build_ids;
    std::vector<int> probe_ids;
    // TODO: 多比较条件暂时只用于SEMI、ANTI-join，应该扩展到所有类型JOIN，后续优化
    std::vector<ExpressionTypes> comparison_types;
    std::shared_ptr<JoinHashTable> hash_table;
public:
    HashJoinLocalSinkState(std::vector<int> build_ids, std::vector<int> probe_ids, std::vector<ExpressionTypes> comparison_types, JoinTypes join_type)
            : build_ids(build_ids), probe_ids(probe_ids), comparison_types(comparison_types), join_type(join_type) {
        hash_table = std::make_shared<JoinHashTable>(this->build_ids, this->probe_ids, this->comparison_types, this->join_type);
    }
};

class PhysicalHashJoin : public PhysicalOperator {
public:
    static constexpr const PhysicalOperatorType TYPE = PhysicalOperatorType::HASH_JOIN;
    JoinTypes join_type = JoinTypes::INNER;
    /* 多线程环境下，必须记录chunk探测其它线程的状态，然后合并所有探测状态
     * 例如，对于LEFT JOIN，可能在Pipeline-1中探测时没有对应结果，但在Pipeline-2中探测到了结果，
     * 若此时直接在Pipeline-1中将没有探测到结果的数据补NULL，会导致产生重复结果。因此，必须要记录每次
     * 探测其它线程的结果，然后根据结果决定是否补NULL
    */
    std::vector<std::shared_ptr<ProbeState>> probe_states;
    std::vector<int> build_ids;
    std::vector<int> probe_ids;
    // TODO: 多比较条件暂时只用于SEMI、ANTI-join，应该扩展到所有类型JOIN，后续优化
    std::vector<ExpressionTypes> comparison_types;
    // 构建表的Schema
    std::shared_ptr<arrow::Schema> build_schema;
    // join结果的Schema
    std::shared_ptr<arrow::Schema> return_schema;
    std::vector<std::shared_ptr<arrow::RecordBatch>> join_result;
    // Semi-join去重
    std::set<int> uniqueNumbers;
    // 支持BUILD表复制
    bool is_copy = false;
    // For debug
    int join_num = 0;
public:
    PhysicalHashJoin(JoinTypes join_type, std::vector<int> build_ids, std::vector<int> probe_ids, std::vector<ExpressionTypes> comparison_types = {}, bool is_copy = false);

    // Sink 接口
    SinkResultType sink(std::shared_ptr<ExecuteContext> &execute_context, LocalSinkState &output) override;
    std::shared_ptr<LocalSinkState> GetLocalSinkState() const override;
    bool is_sink() const override { return true; }
    bool ParallelSink() const override { return true; }
    // Operator 接口
    OperatorResultType execute_operator(std::shared_ptr<ExecuteContext> &execute_context) override;
    bool is_operator() const override { return true; }
    bool ParallelOperator() const override { return true; }

    void build_pipelines(std::shared_ptr<Pipeline> &current, std::shared_ptr<PipelineGroup> &pipelineGroup) override;
    std::shared_ptr<PhysicalOperator> Copy(int arg) override;
};

} // DaseX