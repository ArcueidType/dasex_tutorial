#include "physical_hash_join.hpp"
#include "pipeline.hpp"
#include "arrow_help.hpp"

namespace DaseX {

// 构造函数
PhysicalHashJoin::PhysicalHashJoin(JoinTypes join_type,
                                   std::vector<int> build_ids,
                                   std::vector<int> probe_ids,
                                   std::vector<ExpressionTypes> comparison_types,
                                   bool is_copy)
                                   : PhysicalOperator(PhysicalOperatorType::HASH_JOIN),
                                     join_type(join_type),
                                     build_ids(std::move(build_ids)),
                                     probe_ids(std::move(probe_ids)),
                                     comparison_types(comparison_types),
                                     is_copy(is_copy) {}

// Sink 接口实现
SinkResultType PhysicalHashJoin::sink(std::shared_ptr<ExecuteContext> &execute_context, LocalSinkState &output) {
    auto weak_pipeline = execute_context->get_pipeline();
    auto &lstate = output.Cast<HashJoinLocalSinkState>();
    auto &hash_table = lstate.hash_table;
    if (auto pipeline = weak_pipeline.lock()) {
        if(build_schema == nullptr) {
            build_schema = pipeline->temporary_chunk->schema();
        }
        if (likely(!pipeline->is_final_chunk)) {
            auto &chunk = pipeline->temporary_chunk;
            // spdlog::info("[{} : {}] INNER JOIN Build====", __FILE__, __LINE__);
            hash_table->Build(chunk);
            return SinkResultType::NEED_MORE_INPUT;
        } else {
            return SinkResultType::FINISHED;
        }
    } else {
        throw std::runtime_error("Invalid PhysicalHashJoin::pipeline");
    }
}

std::shared_ptr<LocalSinkState> PhysicalHashJoin::GetLocalSinkState() const {
    std::shared_ptr<HashJoinLocalSinkState> state = std::make_shared<HashJoinLocalSinkState>(build_ids, probe_ids, comparison_types, join_type);
    return state;
}

// Operator 接口
OperatorResultType PhysicalHashJoin::execute_operator(std::shared_ptr<ExecuteContext> &execute_context) {
    auto weak_pipeline = execute_context->get_pipeline();
    if (auto pipeline = weak_pipeline.lock()) {
        std::shared_ptr<arrow::RecordBatch> &data_chunk = pipeline->temporary_chunk;
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
                {
                    return_schema = data_chunk->schema();
                    break;
                }
                case JoinTypes::ANTI:
                {
                    return_schema = data_chunk->schema();
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
        std::vector<std::shared_ptr<arrow::RecordBatch>> result;
        std::chrono::high_resolution_clock::time_point start;
        if(is_copy) {
            auto &lstate = this->lsink_state->Cast<HashJoinLocalSinkState>();
            auto &hash_table = lstate.hash_table;
            std::shared_ptr<ProbeState> probe_state = hash_table->Probe(data_chunk);
            probe_states.push_back(probe_state);
            std::shared_ptr<arrow::RecordBatch> res_p;
            switch (join_type) {
                case JoinTypes::INNER:
                {
                    hash_table->GetJoinResult<JoinTypes::INNER>(data_chunk, res_p, probe_state, probe_state);
                    break;
                }
                case JoinTypes::LEFT:
                {
                    bool is_final_probe = true;
                    std::shared_ptr<ProbeState> last_probe_state = nullptr;
                    hash_table->GetJoinResult<JoinTypes::LEFT>(data_chunk, res_p, probe_state, last_probe_state, is_final_probe);
                    break;
                }
                case JoinTypes::RIGHT:
                {
                    bool is_final_probe = true;
                    std::shared_ptr<ProbeState> last_probe_state = nullptr;
                    hash_table->GetJoinResult<JoinTypes::RIGHT>(data_chunk, res_p, probe_state, last_probe_state, is_final_probe);
                    break;
                }
                case JoinTypes::OUTER:
                {
                    bool is_final_probe = true;
                    std::shared_ptr<ProbeState> last_probe_state = nullptr;
                    hash_table->GetJoinResult<JoinTypes::OUTER>(data_chunk, res_p, probe_state, last_probe_state, is_final_probe);
                    break;
                }
                case JoinTypes::SEMI:
                {
                    auto &match_idxs = probe_state->match_row_idx;
                    std::vector<int> new_match_idxs;
                    for(int i = 0; i < match_idxs.size(); i++) {
                        int val = match_idxs[i];
                        if(uniqueNumbers.find(val) == uniqueNumbers.end()) {
                            uniqueNumbers.insert(val);
                            new_match_idxs.push_back(val);
                        }
                    }
                    match_idxs = new_match_idxs;
                    hash_table->GetJoinResult<JoinTypes::SEMI>(data_chunk, res_p, probe_state, probe_state);
                    break;
                }
                case JoinTypes::ANTI:
                {
                    bool is_final_probe = true;
                    std::shared_ptr<ProbeState> last_probe_state = nullptr;
                    hash_table->GetJoinResult<JoinTypes::ANTI>(data_chunk, res_p, probe_state, last_probe_state, is_final_probe);
                    break;
                }
                default:
                    throw std::runtime_error("Unsupported JoinType");
            } // switch
            if(res_p != nullptr) {
                // spdlog::info("[{} : {}] ANTI size结果: {}", __FILE__, __LINE__, res_p->num_rows());
                result.emplace_back(res_p);
            }
            uniqueNumbers.clear();
            probe_states.clear();
        }
        else {
            if(auto pipelines_group = pipeline->get_pipeline_group().lock()) {
                auto pipelines = pipelines_group->pipelines;
                int parallel_p = pipelines_group->parallelism;
                for(int i = 0; i < parallel_p; i++) {
                    auto &sink = pipelines[i]->operators[operator_idx]->Cast<PhysicalHashJoin>();
                    auto &lstate = sink.lsink_state->Cast<HashJoinLocalSinkState>();
                    auto &hash_table = lstate.hash_table;
                    std::shared_ptr<ProbeState> probe_state = hash_table->Probe(data_chunk);
                    probe_states.emplace_back(probe_state);
                    std::shared_ptr<arrow::RecordBatch> res_p;
                    switch (join_type) {
                        case JoinTypes::INNER:
                        {
                            hash_table->GetJoinResult<JoinTypes::INNER>(data_chunk, res_p, probe_state, probe_state);
                            break;
                        }
                        case JoinTypes::LEFT:
                        {
                            bool is_final_probe = false;
                            if(i == (parallel_p - 1)) {
                                is_final_probe = true;
                            }
                            std::shared_ptr<ProbeState> last_probe_state = (i==0 ? nullptr : probe_states[i - 1]);
                            hash_table->GetJoinResult<JoinTypes::LEFT>(data_chunk, res_p, probe_state, last_probe_state, is_final_probe);
                            break;
                        }
                        case JoinTypes::RIGHT:
                        {
                            bool is_final_probe = false;
                            if(i == (parallel_p - 1)) {
                                is_final_probe = true;
                            }
                            std::shared_ptr<ProbeState> last_probe_state = (i==0 ? nullptr : probe_states[i - 1]);
                            hash_table->GetJoinResult<JoinTypes::RIGHT>(data_chunk, res_p, probe_state, last_probe_state, is_final_probe);
                            break;
                        }
                        case JoinTypes::OUTER:
                        {
                            bool is_final_probe = false;
                            if(i == (parallel_p - 1)) {
                                is_final_probe = true;
                            }
                            std::shared_ptr<ProbeState> last_probe_state = (i==0 ? nullptr : probe_states[i - 1]);
                            hash_table->GetJoinResult<JoinTypes::OUTER>(data_chunk, res_p, probe_state, last_probe_state, is_final_probe);
                            break;
                        }
                        case JoinTypes::SEMI:
                        {
                            auto &match_idxs = probe_state->match_row_idx;
                            std::vector<int> new_match_idxs;
                            for(int i = 0; i < match_idxs.size(); i++) {
                                int val = match_idxs[i];
                                if(uniqueNumbers.find(val) == uniqueNumbers.end()) {
                                    uniqueNumbers.insert(val);
                                    new_match_idxs.emplace_back(val);
                                }
                            }
                            match_idxs = new_match_idxs;
                            hash_table->GetJoinResult<JoinTypes::SEMI>(data_chunk, res_p, probe_state, probe_state);
                            break;
                        }
                        case JoinTypes::ANTI:
                        {
                            bool is_final_probe = false;
                            if(i == (parallel_p - 1)) {
                                is_final_probe = true;
                            }
                            std::shared_ptr<ProbeState> last_probe_state = (i==0 ? nullptr : probe_states[i - 1]);
                            hash_table->GetJoinResult<JoinTypes::ANTI>(data_chunk, res_p, probe_state, last_probe_state, is_final_probe);
                            break;
                        }
                        default:
                            throw std::runtime_error("Unsupported JoinType");
                    } // switch
                    if(res_p != nullptr) {
                        // spdlog::info("[{} : {}] ANTI size结果: {}", __FILE__, __LINE__, res_p->num_rows());
                        result.emplace_back(res_p);
                    }
                } // for
                uniqueNumbers.clear();
                probe_states.clear();
            } // if
            else {
                throw std::runtime_error("Invalid PhysicalHashJoin::pipeline");
            }
        }
        if(!result.empty()) {
            data_chunk = arrow::ConcatenateRecordBatches(result).ValueOrDie();
        } else {
            data_chunk = arrow::RecordBatch::MakeEmpty(return_schema).ValueOrDie();
        }
        join_num += data_chunk->num_rows();
        return OperatorResultType::HAVE_MORE_OUTPUT;
    } // if
    else {
        throw std::runtime_error("Invalid PhysicalHashJoin::pipeline");
    }
}

void PhysicalHashJoin::build_pipelines(std::shared_ptr<Pipeline> &current, std::shared_ptr<PipelineGroup> &pipelineGroup) {
	// Step 1: 先执行探测端Pipeline的构建
	//     Step 1.1: 将当前算子添加至当前Pipeline中
	//     TODO: 这里需要补充代码

	//     Step 1.2: 递归执行右侧孩子（右侧为探测端）
	//     TODO: 这里需要补充代码

	// Step 2: 执行构建端Pipeline的构建，从当前算子开始，递归构建子PipelineGroup
	//     TODO: 这里需要补充代码

}

std::shared_ptr<PhysicalOperator> PhysicalHashJoin::Copy(int arg) {
    auto join_op = std::make_shared<PhysicalHashJoin>(this->join_type, this->build_ids, this->probe_ids, this->comparison_types, this->is_copy);
    join_op->build_schema = build_schema;
    join_op->return_schema = return_schema;
    join_op->exp_name = this->exp_name;
    for(auto &en : this->children_operator) {
        join_op->children_operator.emplace_back(en->Copy(arg));
    }
    return join_op;
}

} // DaseX