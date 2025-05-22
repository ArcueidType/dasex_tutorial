#include "physical_order.hpp"
#include "pipeline.hpp"
#include "common_macro.hpp"
#include "arrow_help.hpp"

namespace DaseX {

PhysicalOrder::PhysicalOrder(std::vector<int> *sort_keys, std::vector<LogicalType> *key_types, std::vector<SortOrder> *orders) : PhysicalOperator(PhysicalOperatorType::ORDER_BY), sort_keys(sort_keys), key_types(key_types), orders(orders) {}

// Common interface
std::shared_ptr<PhysicalOperator> PhysicalOrder::Copy(int arg) {
    auto order_by = std::make_shared<PhysicalOrder>(sort_keys, key_types, orders);
    order_by->gsink_state = gsink_state;
    for(auto &en : this->children_operator) {
        order_by->children_operator.emplace_back(en->Copy(arg));
    }
    order_by->exp_name = this->exp_name;
    return order_by;
}

// Source interface
SourceResultType PhysicalOrder::get_data(std::shared_ptr<ExecuteContext> &execute_context, LocalSourceState &input) const {
    auto &lstate = input.Cast<OrderLocalSourceState>();
    auto weak_pipeline = execute_context->get_pipeline();
    std::vector<std::shared_ptr<arrow::RecordBatch>> buffers(CHUNK_SIZE);
    if (auto pipeline = weak_pipeline.lock())
    {
        // 通过Pipeline_id来计算需要处理的数据的位置
        int pipeline_id = pipeline->pipeline_id;
        int parallelism = 0;
        if(auto pipe_group = pipeline->pipelines.lock()) {
            parallelism = pipe_group->parallelism;
        }
        size_t row_nums = (lstate.result)->size();
        // spdlog::info("[{} : {}] orderby.result.size is :======================={}", __FILE__, __LINE__, row_nums);
        if(row_nums == 0) {
            return SourceResultType::FINISHED;
        }
        if(unlikely(row_nums <= parallelism)) {
            if(pipeline_id < row_nums) {
                pipeline->temporary_chunk = (*lstate.result)[pipeline_id];
            } else {
                auto schema_tmp = ((*lstate.result)[0])->schema();
                pipeline->temporary_chunk = arrow::RecordBatch::MakeEmpty(schema_tmp).ValueOrDie();
            }
            // spdlog::info("[{} : {}] 第{}个pipeline排序后的数据: {}", __FILE__, __LINE__, pipeline_id, pipeline->temporary_chunk->ToString());
            return SourceResultType::FINISHED;
        } else {
            int64_t row_per_pipeline = row_nums / parallelism;
            int64_t start = pipeline_id * row_per_pipeline;
            int count = 0;
            if(pipeline_id != (parallelism - 1)) {
                while(lstate.idx_nums < row_per_pipeline) {
                    uint64_t idx = start + lstate.idx_nums++;
                    buffers[count++] = (*lstate.result)[idx];
                    if(count == CHUNK_SIZE) {
                        // 将buffer合并成一个RB并赋值给temporary_chunk
                        pipeline->temporary_chunk = arrow::ConcatenateRecordBatches(buffers).ValueOrDie();
                        // spdlog::info("[{} : {}] 第{}个pipeline排序后的数据: {}", __FILE__, __LINE__, pipeline_id, pipeline->temporary_chunk->ToString());
                        return SourceResultType::HAVE_MORE_OUTPUT;
                    }
                } // while
                if(count == 0) {
                    return SourceResultType::FINISHED;
                }
                buffers.resize(count);
                pipeline->temporary_chunk = arrow::ConcatenateRecordBatches(buffers).ValueOrDie();
                // spdlog::info("[{} : {}] 第{}个pipeline排序后的数据: {}", __FILE__, __LINE__, pipeline_id, pipeline->temporary_chunk->ToString());
                return SourceResultType::FINISHED;
            } else {
                row_per_pipeline = row_nums - start;
                while(lstate.idx_nums < row_per_pipeline) {
                    uint64_t idx = start + lstate.idx_nums++;
                    buffers[count++] = (*lstate.result)[idx];
                    if(count == CHUNK_SIZE) {
                        // 将buffer合并成一个RB并赋值给temporary_chunk
                        pipeline->temporary_chunk = arrow::ConcatenateRecordBatches(buffers).ValueOrDie();
                        // spdlog::info("[{} : {}] 第{}个pipeline排序后的数据: {}", __FILE__, __LINE__, pipeline_id, pipeline->temporary_chunk->ToString());
                        return SourceResultType::HAVE_MORE_OUTPUT;
                    }
                } // while
                if(count == 0) {
                    return SourceResultType::FINISHED;
                }
                buffers.resize(count);
                pipeline->temporary_chunk = arrow::ConcatenateRecordBatches(buffers).ValueOrDie();
                // spdlog::info("[{} : {}] 第{}个pipeline排序后的数据: {}", __FILE__, __LINE__, pipeline_id, pipeline->temporary_chunk->ToString());
                return SourceResultType::FINISHED;
            }
        }
    } // if
    else
    {
        throw std::runtime_error("Invalid PhysicalOrder::pipeline");
    }
} // get_data

std::shared_ptr<LocalSourceState> PhysicalOrder::GetLocalSourceState() const {
    auto &gstate = (*gsink_state).Cast<OrderGlobalSinkState>();
    return std::make_shared<OrderLocalSourceState>(gstate);
}

// Sink interface
SinkResultType PhysicalOrder::sink(std::shared_ptr<ExecuteContext> &execute_context, LocalSinkState &output) {
    auto weak_pipeline = execute_context->get_pipeline();
    auto &lstate = output.Cast<OrderLocalSinkState>();
    int count_rows = 0;
    size_t pipeline_id = 0;
    if (auto pipeline = weak_pipeline.lock())
    {
        pipeline_id = pipeline->pipeline_id;
        // 1.从上游算子获取数据并缓存
        if (!pipeline->is_final_chunk)
        {
            count_rows = pipeline->temporary_chunk->num_rows();
            if (count_rows != 0)
            {
                // spdlog::info("[{} : {}] Order结果: {}", __FILE__, __LINE__, pipeline->temporary_chunk->ToString());
                lstate.order_cache.push_back(pipeline->temporary_chunk);
            }
            return SinkResultType::NEED_MORE_INPUT;
        }
        else
        {
            count_rows = pipeline->temporary_chunk->num_rows();
            if (count_rows != 0) {
                lstate.order_cache.push_back(pipeline->temporary_chunk);
            }
            auto ok = SortInLocal(lstate);
            // pipeline->get_pipeline_group().expired();
            if (auto pipe_group = pipeline->get_pipeline_group().lock())
            {
                auto barrier = pipe_group->get_barrier();
                // 向PipelineGroup注册一个合并任务
                pipe_group->event = TaskType::MergeSortTask;
                barrier->wait();
            }
            else
            {
                throw std::runtime_error("Invalid PhysicalOrder::pipeline");
            }
            // TODO: 清空缓存数据
            lstate.order_cache.clear();
            // 将排好序的局部数据插入到全局数据中
            auto &gstate = (*gsink_state).Cast<OrderGlobalSinkState>();
            gstate.local_cache[pipeline_id] = lstate.sort_data;
//            std::string fileName = "Q18" + std::to_string(pipeline->pipeline_id) + ".csv";
//            std::string filePath = "/home/cwl/data/tpch_result/" + fileName;
//            if(lstate.sort_data != nullptr && lstate.sort_data->num_rows() != 0) {
//                std::vector<std::shared_ptr<arrow::RecordBatch>> tmp = {lstate.sort_data};
//                Util::WriteToCSVFromRecordBatch(tmp, filePath);
//            }
            gstate.local_sort_idx[pipeline_id] = lstate.sort_idx;
            return SinkResultType::FINISHED;
        }
    }
    else
    {
        throw std::runtime_error("Invalid PhysicalOrder::pipeline");
    }
} // sink

arrow::Status PhysicalOrder::SortInLocal(OrderLocalSinkState &input) {
    if(input.order_cache.empty()) {
        return arrow::Status::OK();
    }
    input.sort_data = arrow::ConcatenateRecordBatches(input.order_cache).ValueOrDie();
//    spdlog::info("[{} : {}] sortinmem 模式: {}", __FILE__, __LINE__, input.sort_data->num_rows());
//    spdlog::info("[{} : {}] sortinmem 模式: {}", __FILE__, __LINE__, input.sort_data->ToString());
    arrow::Datum res;
    std::vector<arrow::compute::SortKey> arrow_sort_keys;
    int col_nums = sort_keys->size();
    for(int i = 0; i < col_nums; i++) {
        std::string col_name = input.sort_data->column_name((*sort_keys)[i]);
        SortOrder inter_order = (*orders)[i];
        arrow::compute::SortOrder order = inter_order == SortOrder::DESCENDING ? arrow::compute::SortOrder::Descending : arrow::compute::SortOrder::Ascending;
        arrow_sort_keys.emplace_back(arrow::compute::SortKey(col_name, order));
    }
    arrow::compute::SortOptions sort_options(arrow_sort_keys);
    ARROW_ASSIGN_OR_RAISE(res, arrow::compute::CallFunction("sort_indices", {input.sort_data}, &sort_options));
    input.sort_idx = std::static_pointer_cast<arrow::UInt64Array>(res.make_array());
    // spdlog::info("[{} : {}] sortinmem 结果: {}", __FILE__, __LINE__, input.sort_idx->ToString());
    return arrow::Status::OK();
} // SortInLocal

std::shared_ptr<LocalSinkState> PhysicalOrder::GetLocalSinkState() const {
    return std::make_shared<OrderLocalSinkState>();
}

std::shared_ptr<GlobalSinkState> PhysicalOrder::GetGlobalSinkState() const {
    return std::make_shared<OrderGlobalSinkState>(sort_keys, key_types, orders);
}

} // Dasex
