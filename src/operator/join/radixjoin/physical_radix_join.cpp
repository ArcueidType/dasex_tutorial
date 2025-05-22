#include "physical_radix_join.hpp"
#include "pipeline.hpp"
#include "common_macro.hpp"

namespace DaseX
{

PhysicalRadixJoin::PhysicalRadixJoin(int build_id, int probe_id, bool is_build, int32_t partition_nums) :
                                        PhysicalOperator(PhysicalOperatorType::RADIX_JOIN),
                                        build_id(build_id),
                                        probe_id(probe_id),
                                        is_build(is_build),
                                        partition_nums(partition_nums) {}
// Source 接口实现
SourceResultType PhysicalRadixJoin::get_data(std::shared_ptr<ExecuteContext> &execute_context, LocalSourceState &input) const
{
    auto &lstate = input.Cast<RadixJoinLocalSourceState>();
    auto weak_pipeline = execute_context->get_pipeline();
    std::vector<std::shared_ptr<arrow::RecordBatch>> buffers(CHUNK_SIZE);
    if (auto pipeline = weak_pipeline.lock())
    {
        // TODO: join结果保存为std::vector<std::shared_ptr<arrow::RecordBatch>> result,RecordBatch都是一行结果
        // 因此，这里想加一个buffer，将RB合并成一个具有CHUNK_SIZE大小的RB，然后将其赋值给temporary_chunk
        int count = 0;
        if (lstate.idx_nums >= lstate.result->size())
        {
            // 这里如果Join结果为空，会出现错误，因为如果一个数据块都没有读到的话temporary_chunk实际上为nullptr，所以要加个判断
            if (pipeline->temporary_chunk == nullptr || pipeline->temporary_chunk->num_rows() == 0)
            {
                return SourceResultType::FINISHED;
            }
            else
            {
                pipeline->temporary_chunk = arrow::RecordBatch::MakeEmpty(pipeline->temporary_chunk->schema()).ValueOrDie();
                return SourceResultType::FINISHED;
            }
        }
        while (lstate.idx_nums < lstate.result->size())
        {
            buffers[count++] = (*lstate.result)[lstate.idx_nums++];
            if (count == CHUNK_SIZE)
            {
                // 将buffer合并成一个RB并赋值给temporary_chunk
                pipeline->temporary_chunk = arrow::ConcatenateRecordBatches(buffers).ValueOrDie();
                // spdlog::info("[{} : {}] Join结果数据: {}", __FILE__, __LINE__, pipeline->temporary_chunk->ToString());
                return SourceResultType::HAVE_MORE_OUTPUT;
            }
        }
        if(count == 0) {
            return SourceResultType::FINISHED;
        }
        buffers.resize(count);
        pipeline->temporary_chunk = arrow::ConcatenateRecordBatches(buffers).ValueOrDie();
        return SourceResultType::FINISHED;
    }
    else
    {
        throw std::runtime_error("Invalid PhysicalTableScan::pipeline");
    }
}

std::shared_ptr<LocalSourceState> PhysicalRadixJoin::GetLocalSourceState() const {
    return std::make_shared<RadixJoinLocalSourceState>(this->lsink_state->Cast<RadixJoinLocalSinkState>());
}

// Sink 接口实现
SinkResultType PhysicalRadixJoin::sink(std::shared_ptr<ExecuteContext> &execute_context, LocalSinkState &output)
{
    auto weak_pipeline = execute_context->get_pipeline();
    is_build = (*(weak_pipeline.lock()->sink)).Cast<PhysicalRadixJoin>().is_build;
    auto &lstate = output.Cast<RadixJoinLocalSinkState>();
    if (is_build)
    {
        int count_rows = 0;
        if (auto pipeline = weak_pipeline.lock())
        {
            // 1.从上游算子获取数据并缓存
            if (!pipeline->is_final_chunk)
            {
                count_rows = pipeline->temporary_chunk->num_rows();
                lstate.build_col_nums = pipeline->temporary_chunk->num_columns();
                if (count_rows != 0)
                {
                    // lstate.buffer_chunk.push_back(std::move(pipeline->temporary_chunk));
                    lstate.buffer_chunk.push_back(pipeline->temporary_chunk);
                    lstate.processed_chunks++;
                    lstate.processed_rows += count_rows;
                }
                return SinkResultType::NEED_MORE_INPUT;
            }
            else
            {
                // auto pipeline_group = pipeline->get_pipeline_group();
                count_rows = pipeline->temporary_chunk->num_rows();
                lstate.build_col_nums = pipeline->temporary_chunk->num_columns();
                if (count_rows != 0)
                {
                    // lstate.buffer_chunk.push_back(std::move(pipeline->temporary_chunk));
                    lstate.buffer_chunk.push_back(pipeline->temporary_chunk);
                    lstate.processed_chunks++;
                    lstate.processed_rows += count_rows;
                }
                radix_partition(lstate, true);
                // pipeline->get_pipeline_group().expired();
                if (auto pipe_group = pipeline->get_pipeline_group().lock())
                {
                    auto barrier = pipe_group->get_barrier();
                    barrier->wait();
                }
                else
                {
                    throw std::runtime_error("Invalid PhysicalRadixJoin::pipeline");
                }
                auto start = std::chrono::steady_clock::now();
                build(lstate);
                auto end = std::chrono::steady_clock::now();
                spdlog::info("[{} : {}] build执行时间: {}", __FILE__, __LINE__, Util::time_difference(start, end));
                lstate.buffer_chunk.clear();
                lstate.processed_chunks = 0;
                lstate.processed_rows = 0;
                is_build = false;
                return SinkResultType::FINISHED;
            }
        }
        else
        {
            throw std::runtime_error("Invalid PhysicalRadixJoin::pipeline");
        }
    }
    else
    {
        int count_rows = 0;
        if (auto pipeline = weak_pipeline.lock())
        {
            // 1.从上游算子获取数据并缓存
            if (!pipeline->is_final_chunk)
            {
                count_rows = pipeline->temporary_chunk->num_rows();
                lstate.probe_col_nums = pipeline->temporary_chunk->num_columns();
                if (count_rows != 0)
                {
                    lstate.buffer_chunk.push_back(std::move(pipeline->temporary_chunk));
                    lstate.processed_chunks++;
                    lstate.processed_rows += count_rows;
                }
            }
            else
            {
                count_rows = pipeline->temporary_chunk->num_rows();
                lstate.probe_col_nums = pipeline->temporary_chunk->num_columns();
                if (count_rows != 0)
                {
                    lstate.buffer_chunk.push_back(std::move(pipeline->temporary_chunk));
                    lstate.processed_chunks++;
                    lstate.processed_rows += count_rows;
                }
                auto pipeline_group = pipeline->get_pipeline_group();
                radix_partition(lstate, false);
                if (auto pipe_group = pipeline_group.lock())
                {
                    auto barrier = pipe_group->get_barrier();
                    barrier->wait();
                    spdlog::info("[{} : {}] 开始probe=======", __FILE__, __LINE__);
                    auto start = std::chrono::steady_clock::now();
                    probe(pipe_group, lstate);
                    auto end = std::chrono::steady_clock::now();
                    spdlog::info("[{} : {}] probe执行时间: {}", __FILE__, __LINE__, Util::time_difference(start, end));
                }
                else
                {
                    throw std::runtime_error("Invalid PhysicalRadixJoin::pipeline");
                }
                lstate.buffer_chunk.clear();
                return SinkResultType::FINISHED;
            }
            return SinkResultType::NEED_MORE_INPUT;
        }
        else
        {
            throw std::runtime_error("Invalid PhysicalRadixJoin::pipeline");
        }
    }
}

void PhysicalRadixJoin::radix_partition(LocalSinkState &input, bool is_build) const
{
    auto &lstate = input.Cast<RadixJoinLocalSinkState>();
    if (lstate.processed_rows == 0)
    {
        return;
    }
    std::shared_ptr<arrow::RecordBatch> rbatch = arrow::ConcatenateRecordBatches(lstate.buffer_chunk).ValueOrDie();
    int32_t partition_nums = lstate.partition_nums;
    int64_t num_rows = lstate.processed_rows;
    int32_t MASK = partition_nums - 1;
    if (is_build)
    {
        std::shared_ptr<arrow::Array> arr = rbatch->column(build_id);
        lstate.output_build.resize(num_rows);
        switch (arr->type_id())
        {
        case arrow::Type::INT32:
        {
            auto int32array = std::static_pointer_cast<arrow::Int32Array>(arr);
            int32_t histogram_tmp[partition_nums] = {0};
            for (int64_t i = 0; i < num_rows; ++i)
            {
                uint32_t bucket = int32array->Value(i) & MASK;
                lstate.prefix_sum_build[bucket]++;
            }
            // 求前缀和
            int32_t sum = 0;
            for (int32_t i = 0; i < partition_nums; i++)
            {
                sum += lstate.prefix_sum_build[i];
                lstate.prefix_sum_build[i] = sum;
            }

            for (int32_t i = 0; i < partition_nums - 1; i++)
            {
                lstate.histogram_build[i + 1] = lstate.prefix_sum_build[i];
                histogram_tmp[i + 1] = lstate.prefix_sum_build[i];
            }
            for (int64_t i = 0; i < num_rows; ++i)
            {
                uint32_t bucket = int32array->Value(i) & MASK;
                lstate.output_build[histogram_tmp[bucket]] = rbatch->Slice(i, 1);
                histogram_tmp[bucket]++;
            }
            spdlog::info("[{} : {}] output_build.size is :======================={}", __FILE__, __LINE__, lstate.output_build.size());
            break;
        } // case arrow::Type::INT32
        // TODO:
        case arrow::Type::FLOAT:
        default:
            throw std::runtime_error("Invalid arrow::Type!!!");
            break;
        } // switch
    }
    else
    {
        std::shared_ptr<arrow::Array> arr = rbatch->column(probe_id);
        lstate.output_probe.resize(num_rows);
        switch (arr->type_id())
        {
        case arrow::Type::INT32:
        {
            auto int32array = std::static_pointer_cast<arrow::Int32Array>(arr);
            int32_t histogram_tmp[partition_nums] = {0};
            for (int64_t i = 0; i < num_rows; ++i)
            {
                uint32_t bucket = int32array->Value(i) & MASK;
                lstate.prefix_sum_probe[bucket]++;
            }
            // 求前缀和
            int32_t sum = 0;
            for (int32_t i = 0; i < partition_nums; i++)
            {
                sum += lstate.prefix_sum_probe[i];
                lstate.prefix_sum_probe[i] = sum;
            }

            for (int32_t i = 0; i < partition_nums - 1; i++)
            {
                lstate.histogram_probe[i + 1] = lstate.prefix_sum_probe[i];
                histogram_tmp[i + 1] = lstate.prefix_sum_probe[i];
            }
            for (int64_t i = 0; i < num_rows; ++i)
            {
                uint32_t bucket = int32array->Value(i) & MASK;
                lstate.output_probe[histogram_tmp[bucket]] = rbatch->Slice(i, 1);
                histogram_tmp[bucket]++;
            }
            spdlog::info("[{} : {}] output_probe.size is :======================={}", __FILE__, __LINE__, lstate.output_probe.size());
            break;
        } // case arrow::Type::INT32
        // TODO:
        case arrow::Type::FLOAT:
        default:
            throw std::runtime_error("Invalid arrow::Type!!!");
            break;
        } // switch
    }

} // radix_partition

void PhysicalRadixJoin::build(LocalSinkState &input) const
{
    auto &lstate = input.Cast<RadixJoinLocalSinkState>();
    if (lstate.processed_rows == 0)
    {
        return;
    }
    int32_t start_pos = 0;
    int32_t end_pos = 0;
    int32_t key = 0;
    for (int i = 0; i < lstate.partition_nums; i++)
    {
        start_pos = lstate.histogram_build[i];
        end_pos = lstate.prefix_sum_build[i];
        std::unordered_map<int32_t, std::shared_ptr<arrow::RecordBatch>> hash_table;
        for (int j = start_pos; j < end_pos; j++)
        {
            std::shared_ptr<arrow::Array> arr = lstate.output_build[j]->column(build_id);
            switch (arr->type_id())
            {
            case arrow::Type::INT32:
            {
                auto int32array = std::static_pointer_cast<arrow::Int32Array>(arr);
                key = int32array->Value(0);
                hash_table.insert(std::pair<int32_t, std::shared_ptr<arrow::RecordBatch>>(key, lstate.output_build[j]));
                break;
            }
            case arrow::Type::FLOAT:
            default:
                throw std::runtime_error("Invalid arrow::Type!!!");
            } // switch
        }     // for
        lstate.hash_tables[i] = hash_table;
    } // for
}     // build

void PhysicalRadixJoin::probe(std::shared_ptr<PipelineGroup> &pipe_group, LocalSinkState &input) const
{
    auto &lstate = input.Cast<RadixJoinLocalSinkState>();
    if (lstate.processed_rows == 0)
    {
        return;
    }
    auto pipeline_arr = pipe_group->get_pipelines();
    int16_t size = pipeline_arr.size();
    int32_t start_pos = 0;
    int32_t end_pos = 0;
    int32_t key = 0;
    for (int i = 0; i < lstate.partition_nums; i++)
    {

        // TODO:
        // 1: 获取到所有的pipeline的HT
        // 2: probe对应分区
        // 3: 产生join结果   lstate.result
        start_pos = lstate.histogram_probe[i];
        end_pos = lstate.prefix_sum_probe[i];
        spdlog::info("[{} : {}] start_pos and end_pos is :======[{} : {}]", __FILE__, __LINE__, start_pos, end_pos);
        for (int j = start_pos; j < end_pos; j++)
        {
            std::shared_ptr<arrow::Array> arr = lstate.output_probe[j]->column(probe_id);
            switch (arr->type_id())
            {
            case arrow::Type::INT32:
            {
                auto int32array = std::static_pointer_cast<arrow::Int32Array>(arr);
                key = int32array->Value(0);
                for (int k = 0; k < size; k++)
                {
                    auto &lstate_k = (pipeline_arr[k]->output)->Cast<RadixJoinLocalSinkState>();
                    auto &hash_tables = lstate_k.hash_tables;
                    auto &hash_table = hash_tables[i];
                    auto it = hash_table.find(key);
                    if (it != hash_table.end())
                    {
                        // 拼接两个RecordBatch
                        auto &left = it->second;
                        auto &right = lstate.output_probe[j];
                        std::shared_ptr<arrow::RecordBatch> join_res;
                        Util::join_two_record_batch(left, lstate.build_col_nums, build_id, right, lstate.probe_col_nums, probe_id, join_res);
                        // 将结果保存至result
                        lstate.result.emplace_back(join_res);
                    }
                }
                break;
            }
            case arrow::Type::FLOAT:
            default:
                throw std::runtime_error("Invalid arrow::Type!!!");
                break;
            } // switch
        }     // for
    }         // for
    spdlog::info("[{} : {}] lstate.result.size is :======================={}", __FILE__, __LINE__, lstate.result.size());
}

std::shared_ptr<LocalSinkState> PhysicalRadixJoin::GetLocalSinkState() const {
    return std::make_shared<RadixJoinLocalSinkState>(partition_nums);
}

RadixJoinLocalSinkState::RadixJoinLocalSinkState(int32_t partition_nums) : partition_nums(partition_nums) {
    NEXT_POW_2(partition_nums);
    histogram_build.resize(partition_nums);
    prefix_sum_build.resize(partition_nums);
    histogram_probe.resize(partition_nums);
    prefix_sum_probe.resize(partition_nums);
    hash_tables.resize(partition_nums);
}

std::shared_ptr<PhysicalOperator> PhysicalRadixJoin::Copy(int arg) {
    //PhysicalRadixJoin(int16_t build_id, int16_t probe_id, bool is_build = true, int32_t partition_nums = 4);
    auto join_op = std::make_shared<PhysicalRadixJoin>(this->build_id, this->probe_id, this->is_build, this->partition_nums);
    for(auto &en : this->children_operator) {
        join_op->children_operator.emplace_back(en->Copy(arg));
    }
    return join_op;
}

void PhysicalRadixJoin::build_pipelines(std::shared_ptr<Pipeline> &current, std::shared_ptr<PipelineGroup> &pipelineGroup) {
    current->source = this->getptr();
    // we create a new pipelineGroup starting from the child
    pipelineGroup->create_child_group(current->source);
} // build_pipelines

} // namespace DaseX