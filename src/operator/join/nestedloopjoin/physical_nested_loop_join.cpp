//
// Created by cwl on 7/31/24.
//
#include "physical_nested_loop_join.hpp"
#include "pipeline.hpp"
#include "common_macro.hpp"
#include "arrow_help.hpp"


namespace DaseX {

// 构造函数
PhysicalNestedLoopJoin::PhysicalNestedLoopJoin(JoinTypes join_type,
                                               std::vector<int> build_ids,
                                               std::vector<int> probe_ids,
                                               std::vector<ExpressionTypes> compare_types)
                                        : PhysicalOperator(PhysicalOperatorType::NESTED_LOOP_JOIN),
                                          join_type(join_type),
                                          build_ids(std::move(build_ids)),
                                          probe_ids(std::move(probe_ids)),
                                          compare_types(std::move(compare_types)) {}

// Sink 接口
SinkResultType PhysicalNestedLoopJoin::sink(std::shared_ptr<ExecuteContext> &execute_context, LocalSinkState &output) {
    auto weak_pipeline = execute_context->get_pipeline();
    auto &lstate = output.Cast<NestedLoopJoinLocalSinkState>();
    auto &cache_result = lstate.cache_result;
    if (auto pipeline = weak_pipeline.lock()) {
        auto &chunk = pipeline->temporary_chunk;
        if(build_schema == nullptr) {
            build_schema = chunk->schema();
        }
        if (likely(!pipeline->is_final_chunk)) {
            int num_rows = chunk->num_rows();
            if(num_rows != 0) {
                lstate.total_rows += num_rows;
                cache_result.push_back(chunk);
            }
            return SinkResultType::NEED_MORE_INPUT;
        } else {
            return SinkResultType::FINISHED;
        }
    }
}

// Operator 接口
OperatorResultType PhysicalNestedLoopJoin::execute_operator(std::shared_ptr<ExecuteContext> &execute_context) {
    auto weak_pipeline = execute_context->get_pipeline();
    if (auto pipeline = weak_pipeline.lock()) {
        RecordBatchPtr &data_chunk = pipeline->temporary_chunk;
        if(return_schema == nullptr) {
            switch (join_type) {
                case JoinTypes::INNER:
                case JoinTypes::LEFT:
                case JoinTypes::RIGHT:
                {
                    auto probe_schema = data_chunk->schema();
                    return_schema = Util::MergeTwoSchema(build_schema, probe_schema);
                    break;
                }
                case JoinTypes::SEMI:
                case JoinTypes::ANTI:
                {
                    return_schema = build_schema;
                    break;
                }
                default:
                    throw std::runtime_error("Unsupported JoinType");
            } // switch
        }
        if (data_chunk->num_rows() == 0) {
            data_chunk = arrow::RecordBatch::MakeEmpty(return_schema).ValueOrDie();
            return OperatorResultType::FINISHED;
        }
        int operator_idx = execute_context->operator_idx;
        RecordBatchPtrVector result;
        if(auto pipelines_group = pipeline->get_pipeline_group().lock()) {
            auto pipelines = pipelines_group->pipelines;
            int parallel_p = pipelines_group->parallelism;
            for(int i = 0; i < parallel_p; i++) {
                auto &sink = pipelines[i]->operators[operator_idx]->Cast<PhysicalNestedLoopJoin>();
                auto &lstate = sink.lsink_state->Cast<NestedLoopJoinLocalSinkState>();
                auto &cache_result = lstate.cache_result;
                RecordBatchPtr res_p;
                switch (join_type) {
                    case JoinTypes::INNER:
                    {
                        GetInnerJoinResult(cache_result, data_chunk, res_p);
                        break;
                    }
                    case JoinTypes::LEFT:
                    {
                        break;
                    }
                    case JoinTypes::RIGHT:
                    {
                        break;
                    }
                    case JoinTypes::SEMI:
                    {
                        break;
                    }
                    case JoinTypes::ANTI:
                    {
                        break;
                    }
                    default:
                        throw std::runtime_error("Unsupported JoinType");
                } // switch
                if(res_p != nullptr) {
                    result.push_back(res_p);
                }
            } // for
            if(!result.empty()) {
                data_chunk = arrow::ConcatenateRecordBatches(result).ValueOrDie();
            } else {
                data_chunk = arrow::RecordBatch::MakeEmpty(return_schema).ValueOrDie();
            }
            process_nums += data_chunk->num_rows();
            return OperatorResultType::HAVE_MORE_OUTPUT;
        } // if
    }
}

void PhysicalNestedLoopJoin::GetInnerJoinResult(RecordBatchPtrVector &cache_result,
                                                RecordBatchPtr &data_chunk,
                                                RecordBatchPtr &result) const {
    if(cache_result.empty()) {
        return;
    }
    int chunk_size = cache_result.size();
    int data_chunk_rows = data_chunk->num_rows();
    int data_chunk_columns = data_chunk->num_columns();
    RecordBatchPtrVector result_p;
    for(int i = 0; i < chunk_size; i++) {
        RecordBatchPtr &chunk_p = cache_result[i];
        int num_rows = chunk_p->num_rows();
        int num_columns = chunk_p->num_columns();
        for(int j = 0; j < num_rows; j++) {
            for(int k = 0; k < data_chunk_rows; k++) {
                auto right = data_chunk->Slice(k, 1);
                auto left = chunk_p->Slice(j, 1);
                bool is_match = Util::MatchTwoRecordBatch(left, right, build_ids, probe_ids, compare_types);
                if(is_match) {
                    std::shared_ptr<arrow::RecordBatch> res_p;
                    Util::join_two_record_batch(left, num_columns, 0, right, data_chunk_columns, 0, res_p);
                    result_p.push_back(res_p);
                }
            }
        } // for
    } // for
    if(!result_p.empty()) {
        result = arrow::ConcatenateRecordBatches(result_p).ValueOrDie();
    }
} // GetInnerJoinResult

std::shared_ptr<LocalSinkState> PhysicalNestedLoopJoin::GetLocalSinkState() const {
    auto exp = std::make_shared<NestedLoopJoinLocalSinkState>();
    return exp;
}

// Common 接口
void PhysicalNestedLoopJoin::build_pipelines(std::shared_ptr<Pipeline> &current, std::shared_ptr<PipelineGroup> &pipelineGroup) {
    // Probe Side
    // 'current' is the probe pipeline: add this operator
    auto current_operator = this->getptr();
    current->operators.push_back(current_operator);
    children_operator[1]->build_pipelines(current, pipelineGroup);
    // Build Side
    pipelineGroup->create_child_group(current_operator);
}

std::shared_ptr<PhysicalOperator> PhysicalNestedLoopJoin::Copy(int arg) {
    auto join_op = std::make_shared<PhysicalNestedLoopJoin>(this->join_type, this->build_ids, this->probe_ids, this->compare_types);
    join_op->build_schema = build_schema;
    join_op->return_schema = return_schema;
    join_op->exp_name = this->exp_name;
    for(auto &en : this->children_operator) {
        join_op->children_operator.emplace_back(en->Copy(arg));
    }
    return join_op;
}

} // DaseX