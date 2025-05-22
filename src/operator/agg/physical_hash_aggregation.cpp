#include "physical_hash_aggregation.hpp"
#include "pipeline.hpp"
#include <cmath>
#include <functional>
#include <unordered_map>
#include <memory>

namespace {

double RoundToDecimalPlaces(double value, int precision) {
    double factor = std::pow(10.0, precision);
    return std::trunc(value * factor) / factor;
}

float RoundToDecimalPlaces(float value, int precision) {
    float factor = std::pow(10.0f, precision);
    return std::round(value * factor) / factor;
}

}

namespace DaseX {

// Common interface
std::shared_ptr<PhysicalOperator> PhysicalHashAgg::Copy(int arg) {
    auto groupByOperator = std::make_shared<PhysicalHashAgg>(group_by_id, aggregate_function_types, group_item_idxs, is_star);
    return groupByOperator;
}
// Sink 接口
SinkResultType PhysicalHashAgg::sink(std::shared_ptr<ExecuteContext> &execute_context, LocalSinkState &output)
{
    auto weak_pipeline = execute_context->get_pipeline();
    auto &lstate = output.Cast<AggLocalSinkState>();
    if (auto pipeline = weak_pipeline.lock())
    {
        if(group_by_id != -1) {
            auto &chunk = pipeline->temporary_chunk;
            if (!pipeline->is_final_chunk) {
                if(chunk->num_rows() != 0) {
                    if(unlikely(lstate.result_schema == nullptr)) {
                        lstate.result_schema = chunk->schema();
                    }
                    lstate.processed_rows += chunk->num_rows();
                    local_agg(chunk, lstate); // 第一阶段: build本地HashTable，中间结果做分区
                }
                return SinkResultType::NEED_MORE_INPUT;
            } else {
                spdlog::info("[{} : {}] -----------agg lstate.processed_rows is {}", __FILE__, __LINE__, lstate.processed_rows);
                if(unlikely(lstate.result_schema == nullptr)) {
                    lstate.result_schema = chunk->schema();
                }
                auto pipeline_group = pipeline->get_pipeline_group();
                if (auto pipe_group = pipeline_group.lock())
                {
                    auto barrier = pipe_group->get_barrier(); // 等待所有线程处理完，准备进入第二阶段
                    barrier->wait();
                    auto status = global_agg(lstate, pipe_group, pipeline, pipeline->pipeline_id); // 第二阶段: Global Hash Agg, 每个线程并发的去做全局聚合
                    assert(status == arrow::Status::OK());
//                    std::string fileName = "Q19_" + std::to_string(pipeline->pipeline_id) + ".csv";
//                    std::string filePath = "/home/cwl/data/tpch_result" + fileName;
//                    if(lstate.result->num_rows() != 0) {
//                        std::vector<std::shared_ptr<arrow::RecordBatch>> tmp = {lstate.result};
//                        Util::WriteToCSVFromRecordBatch(tmp, filePath);
//                    }
                    return SinkResultType::FINISHED;
                } else {
                    throw std::runtime_error("pipeline group error");
                }
            }
        } else {
            auto &chunk = pipeline->temporary_chunk;
            if (!pipeline->is_final_chunk) {
                if (chunk->num_rows() != 0) {
                    if (unlikely(lstate.result_schema == nullptr)) {
                        lstate.result_schema = chunk->schema();
                    }
                    lstate.cache_data.push_back(chunk); // 第一阶段: build本地HashTable，中间结果做分区
                }
                return SinkResultType::NEED_MORE_INPUT;
            } else {
                if(unlikely(lstate.result_schema == nullptr)) {
                    lstate.result_schema = chunk->schema();
                }
                // TODO: 计算本地聚合结果
                local_agg_without_groupby(lstate);
                // TODO: 将结果汇集到第一个Pipeline中
                auto pipeline_group = pipeline->get_pipeline_group();
                if (auto pipe_group = pipeline_group.lock()) {
                    auto barrier = pipe_group->get_barrier(); // 等待所有线程处理完，准备进入第二阶段
                    barrier->wait();
                    auto status = global_agg_without_groupby(lstate, pipe_group, pipeline, pipeline->pipeline_id); // 第二阶段: Global Hash Agg, 每个线程并发的去做全局聚合
                    assert(status == arrow::Status::OK());
//                    std::string fileName = "Q19_" + std::to_string(pipeline->pipeline_id) + ".csv";
//                    std::string filePath = "/home/cwl/data/tpch_result/" + fileName;
//                    if(lstate.result->num_rows() != 0) {
//                        std::vector<std::shared_ptr<arrow::RecordBatch>> tmp = {lstate.result};
//                        Util::WriteToCSVFromRecordBatch(tmp, filePath);
//                    }
                    return SinkResultType::FINISHED;
                } else {
                    throw std::runtime_error("pipeline group error");
                }
            } // else
        }
    } else {
        throw std::runtime_error("Invalid PhysicalHashAgg::pipeline");
    }
}

void PhysicalHashAgg::local_agg(std::shared_ptr<arrow::RecordBatch> &chunk, AggLocalSinkState &state) const {
    auto &partitioned_hashtables = state.partitioned_hash_tables;
    auto partition_num = state.partition_nums;
    auto column = chunk->column(group_by_id);
    // spdlog::info("[{} : {}] -----------chunk name  is {}, group_by_id is {} ", __FILE__, __LINE__, chunk->ToString(), group_by_id);
    auto num_rows = chunk->num_rows();
    auto agg_size = group_item_idxs.size();
    assert(aggregate_function_types.size() == agg_size);
    switch (column->type_id())
    {
        case arrow::Type::INT32:
        {
            std::hash<int> intHasher;
            auto int32array = std::static_pointer_cast<arrow::Int32Array>(column);
            for (size_t i = 0; i < num_rows; ++i)
            {
                auto v = int32array->Value(i);
                auto &hash_table = partitioned_hashtables[intHasher(v) % partition_num];
                auto it = hash_table.find(v);
                if (it == hash_table.end()) {
                    std::vector<std::unique_ptr<Aggstate>> vec;
                    for (auto idx = 0u; idx < agg_size; ++idx) {
                        switch (aggregate_function_types[idx])
                        {
                            case AggFunctionType::COUNT:
                            {
                                // 修改：初始化赋值，否则丢失第一个数据
                                auto aggState = std::make_unique<CountState>();
                                auto group_by_item_array = std::static_pointer_cast<arrow::Int32Array>(chunk->column(group_item_idxs[idx]));
                                if(!group_by_item_array->IsNull(i) || is_star) {
                                    aggState->add(1);
                                }
                                vec.emplace_back(std::move(aggState));
                                break;
                            }
                            case AggFunctionType::MIN:
                            {
                                // 修改：初始化赋值，否则丢失第一个数据
                                auto aggState = std::make_unique<MinState>();
                                // TODO:这里需要根据类型判断进行强转，目前先默认为Int32，后续需要优化
                                auto group_by_item_array = std::static_pointer_cast<arrow::Int32Array>(chunk->column(group_item_idxs[idx]));
                                aggState->add(group_by_item_array->Value(i));
                                vec.emplace_back(std::move(aggState));
                                break;
                            }
                            case AggFunctionType::MAX:
                            {
                                // 修改：初始化赋值，否则丢失第一个数据
                                auto aggState = std::make_unique<MaxState>();
                                auto group_by_item_array = std::static_pointer_cast<arrow::Int32Array>(chunk->column(group_item_idxs[idx]));
                                aggState->add(group_by_item_array->Value(i));
                                vec.emplace_back(std::move(aggState));
                                break;
                            }
                            case AggFunctionType::AVG:
                            {
                                // 修改：初始化赋值，否则丢失第一个数据
                                auto aggState = std::make_unique<AvgState>();
                                auto group_by_item_array = std::static_pointer_cast<arrow::Int32Array>(chunk->column(group_item_idxs[idx]));
                                aggState->add(group_by_item_array->Value(i));
                                vec.emplace_back(std::move(aggState));
                                break;
                            }
                            case AggFunctionType::SUM:
                            {
                                // 修改：初始化赋值，否则丢失第一个数据
                                auto aggState = std::make_unique<SumState>();
                                auto group_by_item_array = std::static_pointer_cast<arrow::Int32Array>(chunk->column(group_item_idxs[idx]));
                                aggState->add(group_by_item_array->Value(i));
                                vec.emplace_back(std::move(aggState));
                                break;
                            }
                            default:
                                throw std::runtime_error("Not Supported For Unknown AggType");
                        }
                    }
                    hash_table.emplace(v, std::move(vec));
                } else {
                    auto& agg_states = it->second;
                    for (auto idx = 0u; idx < agg_size; ++idx)
                    {
                        auto group_by_item_array = std::static_pointer_cast<arrow::Int32Array>(chunk->column(group_item_idxs[idx]));
                        agg_states[idx]->add(group_by_item_array->Value(i));
                    }
                }
            }
            break;
        } // case arrow::Type::INT32
        case arrow::Type::FLOAT:
        default:
            throw std::runtime_error("Not Supported For Now");
            break;
    } // switch
}

void PhysicalHashAgg::local_agg_without_groupby(AggLocalSinkState &lstate) {
    std::vector<std::unique_ptr<Aggstate>> &vec = lstate.local_state;
    auto agg_size = group_item_idxs.size();
    int chunk_nums = lstate.cache_data.size();
    for (auto idx = 0u; idx < agg_size; ++idx) {
        switch (aggregate_function_types[idx])
        {
            case AggFunctionType::COUNT:
            {
                auto aggState = std::make_unique<CountState>();
                vec.emplace_back(std::move(aggState));
                break;
            }
            case AggFunctionType::MIN:
            {
                auto aggState = std::make_unique<MinState>();
                vec.emplace_back(std::move(aggState));
                break;
            }
            case AggFunctionType::MAX:
            {
                auto aggState = std::make_unique<MaxState>();
                vec.emplace_back(std::move(aggState));
                break;
            }
            case AggFunctionType::AVG:
            {
                auto aggState = std::make_unique<AvgState>();
                vec.emplace_back(std::move(aggState));
                break;
            }
            case AggFunctionType::SUM:
            {
                // 修改：初始化赋值，否则丢失第一个数据
                auto aggState = std::make_unique<SumState>();
                vec.emplace_back(std::move(aggState));
                break;
            }
            default:
                throw std::runtime_error("Not Supported For Unknown AggType");
        } // switch
    } // for
    for (auto idx = 0u; idx < agg_size; ++idx) {
        for(int i = 0; i < chunk_nums; i++) {
            auto &chunk = lstate.cache_data[i];
			int row_numsp = chunk->num_rows();
            for(int j = 0; j < row_numsp; j++) {
                auto data_type = chunk->column(group_item_idxs[idx])->type_id();
                switch (data_type) {
                    case arrow::Type::INT32:
                    {
                        auto group_by_item_array = std::static_pointer_cast<arrow::Int32Array>(chunk->column(group_item_idxs[idx]));
                        vec[idx]->add(group_by_item_array->Value(j));
                        break;
                    }
                    case arrow::Type::FLOAT:
                    {
                        auto group_by_item_array = std::static_pointer_cast<arrow::FloatArray>(chunk->column(group_item_idxs[idx]));
                        float val = RoundToDecimalPlaces(group_by_item_array->Value(j), precision);
                        vec[idx]->add(val);
                        break;
                    }
                    case arrow::Type::DOUBLE:
                    {
                        auto group_by_item_array = std::static_pointer_cast<arrow::DoubleArray>(chunk->column(group_item_idxs[idx]));
                        double val = RoundToDecimalPlaces(group_by_item_array->Value(j), precision);
                        vec[idx]->add(val);
                        break;
                    }
                    default:
                        throw std::runtime_error("Invalid arrow::Type!!!");
                } // switch
            }
        }
    }
}


arrow::Status PhysicalHashAgg::global_agg(AggLocalSinkState &lstate,
                                          std::shared_ptr<PipelineGroup> &pipeline_group,
                                          std::shared_ptr<Pipeline> &pipeline,
                                          size_t pipeline_id) const {
    const auto &pipelines = pipeline_group->get_pipelines();
    assert(pipelines.size() > 0);
    auto agg_size = group_item_idxs.size();
    // 合并对应分区
    auto &local_hashtables = lstate.partitioned_hash_tables;
    auto &local_hashtable = local_hashtables[pipeline_id];
    for(int i = 0;  i < pipelines.size(); i++) {
        auto &other_state = pipelines[i]->output->Cast<AggLocalSinkState>();
        if(i == pipeline_id) {
            continue;
        }
        auto &other_hashtable = other_state.partitioned_hash_tables[pipeline_id];
        for (auto &[k, state_vec] : other_hashtable) {
            auto it = local_hashtable.find(k);
            if (it == local_hashtable.end()) {
                local_hashtable.emplace(k, std::move(state_vec));
            } else {
                for (auto idx = 0u; idx < agg_size; ++idx) {
                    local_hashtable[k][idx]->merge(*state_vec[idx]);
                }
            }
        }
    } // for
    // std::unordered_map<int32_t, std::vector<std::unique_ptr<Aggstate>>>
    // 将聚合结果转换成Arrow RecordBatch
    std::vector<std::shared_ptr<arrow::Field>> new_fields;
    auto &old_schema = lstate.result_schema;
    std::vector<std::string> old_names = old_schema->field_names();
    std::string group_by_name = old_names[group_by_id];
    std::shared_ptr<arrow::Field> group_by_field = std::make_shared<arrow::Field>(group_by_name, arrow::int32());
    new_fields.emplace_back(group_by_field);
    for(int i = 0; i < group_item_idxs.size(); i++) {
        switch (aggregate_function_types[i]) {
            case AggFunctionType::COUNT:
            {
                std::string old_name = old_names[group_item_idxs[i]];
                std::string new_name = "Count(" + old_name + ")";
                std::shared_ptr<arrow::Field> new_field = std::make_shared<arrow::Field>(new_name, arrow::int32());
                new_fields.emplace_back(new_field);
                break;
            }
            case AggFunctionType::AVG:
            {
                std::string old_name = old_names[group_item_idxs[i]];
                std::string new_name = "Avg(" + old_name + ")";
                std::shared_ptr<arrow::Field> new_field = std::make_shared<arrow::Field>(new_name, arrow::float64());
                new_fields.emplace_back(new_field);
                break;
            }
            case AggFunctionType::MIN:
            {
                std::string old_name = old_names[group_item_idxs[i]];
                std::string new_name = "Min(" + old_name + ")";
                std::shared_ptr<arrow::Field> new_field = std::make_shared<arrow::Field>(new_name, arrow::float64());
                new_fields.emplace_back(new_field);
                break;
            }
            case AggFunctionType::MAX:
            {
                std::string old_name = old_names[group_item_idxs[i]];
                std::string new_name = "Max(" + old_name + ")";
                std::shared_ptr<arrow::Field> new_field = std::make_shared<arrow::Field>(new_name, arrow::float64());
                new_fields.emplace_back(new_field);
                break;
            }
            case AggFunctionType::SUM:
            {
                std::string old_name = old_names[group_item_idxs[i]];
                std::string new_name = "Sum(" + old_name + ")";
                std::shared_ptr<arrow::Field> new_field = std::make_shared<arrow::Field>(new_name, arrow::float64());
                new_fields.emplace_back(new_field);
                break;
            }
            default:
                throw std::runtime_error("Unsupported aggregation types!!!");
        }
    } // for
    auto new_schema = arrow::schema(new_fields);
    std::vector<std::shared_ptr<arrow::Array>> new_columns;
    std::unordered_map<int, arrow::Int32Builder> intMap;
    std::unordered_map<int, arrow::DoubleBuilder> doubleMap;
    for (auto &[key, state_vec] : local_hashtable) {
        ARROW_RETURN_NOT_OK(intMap[0].Append(key));
        for(auto idx = 0; idx < agg_size; idx++) {
            switch (aggregate_function_types[idx])
            {
                case AggFunctionType::AVG:
                {
                    auto result = static_cast<AvgState*>(state_vec[idx].get())->finalize_avg();
                    ARROW_RETURN_NOT_OK(doubleMap[idx + 1].Append(result));
                    break;
                }
                case AggFunctionType::SUM:
                case AggFunctionType::MAX:
                case AggFunctionType::MIN:
                {
                    ARROW_RETURN_NOT_OK(doubleMap[idx + 1].Append(state_vec[idx]->finalize()));
                    break;
                }
                default:
                {
                    ARROW_RETURN_NOT_OK(intMap[idx + 1].Append(state_vec[idx]->finalize()));
                    break;
                }
            }
        }
    } // for
    ARROW_ASSIGN_OR_RAISE(std::shared_ptr<arrow::Array> key_arr, intMap[0].Finish())
    new_columns.push_back(key_arr);
    for (auto idx = 0; idx < agg_size; idx++) {
        std::shared_ptr<arrow::Array> agg_item;
        switch (aggregate_function_types[idx])
        {
            case AggFunctionType::COUNT:
            {
                ARROW_ASSIGN_OR_RAISE(agg_item, intMap[idx + 1].Finish())
                break;
            }
            default:
            {
                ARROW_ASSIGN_OR_RAISE(agg_item, doubleMap[idx + 1].Finish())
                break;
            }
        }
        new_columns.push_back(agg_item);
    } // for
    lstate.result = arrow::RecordBatch::Make(new_schema, key_arr->length(), new_columns);
//    spdlog::info("[{} : {}] lstate.result.size is :======================={}", __FILE__, __LINE__, lstate.result->num_rows());
//    spdlog::info("[{} : {}] lstate.result is :----------------------{}", __FILE__, __LINE__, lstate.result->ToString());
    return arrow::Status::OK();
}

arrow::Status PhysicalHashAgg::global_agg_without_groupby(AggLocalSinkState &lstate,
                                                          std::shared_ptr<PipelineGroup> &pipeline_group,
                                                          std::shared_ptr<Pipeline> &pipeline,
                                                          int pipeline_id) {
    const auto &pipelines = pipeline_group->get_pipelines();
    assert(pipelines.size() > 0);
    auto agg_size = group_item_idxs.size();
    auto &local_state = lstate.local_state;
    // 将其它Pipeline的结果汇集到pipeline-0
    if(pipeline_id == 0) {
        for(int i = 1;  i < pipelines.size(); i++) {
            auto &other_state = pipelines[i]->output->Cast<AggLocalSinkState>();
            auto &other_local_state = other_state.local_state;
            for (int idx = 0; idx < agg_size; ++idx) {
                local_state[idx]->merge(*other_local_state[idx]);
            }
        } // for
    }
    // 将聚合结果转换成Arrow RecordBatch
    std::vector<std::shared_ptr<arrow::Field>> new_fields;
    auto &old_schema = lstate.result_schema;
    std::vector<std::string> old_names = old_schema->field_names();
    for(int i = 0; i < group_item_idxs.size(); i++) {
        switch (aggregate_function_types[i]) {
            case AggFunctionType::COUNT:
            {
                std::string old_name = old_names[group_item_idxs[i]];
                std::string new_name = "Count(" + old_name + ")";
                std::shared_ptr<arrow::Field> new_field = std::make_shared<arrow::Field>(new_name, arrow::int32());
                new_fields.emplace_back(new_field);
                break;
            }
            case AggFunctionType::AVG:
            {
                std::string old_name = old_names[group_item_idxs[i]];
                std::string new_name = "Avg(" + old_name + ")";
                std::shared_ptr<arrow::Field> new_field = std::make_shared<arrow::Field>(new_name, arrow::float32());
                new_fields.emplace_back(new_field);
                break;
            }
            case AggFunctionType::MIN:
            {
                std::string old_name = old_names[group_item_idxs[i]];
                std::string new_name = "Min(" + old_name + ")";
                std::shared_ptr<arrow::Field> new_field = std::make_shared<arrow::Field>(new_name, arrow::int32());
                new_fields.emplace_back(new_field);
                break;
            }
            case AggFunctionType::MAX:
            {
                std::string old_name = old_names[group_item_idxs[i]];
                std::string new_name = "Max(" + old_name + ")";
                std::shared_ptr<arrow::Field> new_field = std::make_shared<arrow::Field>(new_name, arrow::int32());
                new_fields.emplace_back(new_field);
                break;
            }
            case AggFunctionType::SUM:
            {
                std::string old_name = old_names[group_item_idxs[i]];
                std::string new_name = "Sum(" + old_name + ")";
                std::shared_ptr<arrow::Field> new_field = std::make_shared<arrow::Field>(new_name, arrow::int32());
                new_fields.emplace_back(new_field);
                break;
            }
            default:
            {
                std::string old_name = old_names[group_item_idxs[i]];
                std::shared_ptr<arrow::Field> new_field = std::make_shared<arrow::Field>(old_name, arrow::int32());
                new_fields.emplace_back(new_field);
                break;
            }
        }
    } // for
    auto new_schema = arrow::schema(new_fields);
    if(pipeline_id == 0) {
        std::vector<std::shared_ptr<arrow::Array>> new_columns;
        arrow::Status ok;
        std::unordered_map<int, arrow::Int32Builder> intMap;
        std::unordered_map<int, arrow::FloatBuilder> floatMap;
        for(auto idx = 0; idx < agg_size; idx++) {
            switch (aggregate_function_types[idx])
            {
                case AggFunctionType::AVG: {
                    auto result = static_cast<AvgState*>(local_state[idx].get())->finalize_avg();
                    ARROW_RETURN_NOT_OK(floatMap[idx].Append(result));
                    break;
                }
                case AggFunctionType::MIN: {
					float result = static_cast<MinState*>(local_state[idx].get())->finalize();
                    result = RoundToDecimalPlaces(result, precision);
                    ARROW_RETURN_NOT_OK(floatMap[idx].Append(result));
                    break;
                }
                case AggFunctionType::MAX: {
					float result = static_cast<MaxState*>(local_state[idx].get())->finalize();
                    result = RoundToDecimalPlaces(result, precision);
                    ARROW_RETURN_NOT_OK(floatMap[idx].Append(result));
                    break;
                }
                case AggFunctionType::SUM: {
					float result = static_cast<SumState*>(local_state[idx].get())->finalize();
                    result = RoundToDecimalPlaces(result, precision);
                    ARROW_RETURN_NOT_OK(floatMap[idx].Append(result));
                    break;
                }
                default: {
                    ARROW_RETURN_NOT_OK(intMap[idx].Append(local_state[idx]->finalize()));
                    break;
                }
            }
        } // for
        for (auto idx = 0; idx < agg_size; idx++) {
            std::shared_ptr<arrow::Array> agg_item;
            switch (aggregate_function_types[idx])
            {
                case AggFunctionType::AVG:
                {
                    ARROW_ASSIGN_OR_RAISE(agg_item, floatMap[idx].Finish())
                    break;
                }
                case AggFunctionType::MAX:
                case AggFunctionType::MIN:
                {
                    ARROW_ASSIGN_OR_RAISE(agg_item, floatMap[idx].Finish())
                    break;
                }
                case AggFunctionType::SUM:
                {
                    ARROW_ASSIGN_OR_RAISE(agg_item, floatMap[idx].Finish())
                    break;
                }
                default:
                {
                    ARROW_ASSIGN_OR_RAISE(agg_item, intMap[idx].Finish())
                    break;
                }
            }
            new_columns.push_back(agg_item);
        } // for
        lstate.result = arrow::RecordBatch::Make(new_schema, new_columns[0]->length(), new_columns);
        // spdlog::info("[{} : {}] lstate.result.size is :======================={}", __FILE__, __LINE__, lstate.result->num_rows());
        // spdlog::info("[{} : {}] lstate.result is :----------------------{}", __FILE__, __LINE__, lstate.result->ToString());
    } else {
        lstate.result = arrow::RecordBatch::MakeEmpty(new_schema).ValueOrDie();
        // spdlog::info("[{} : {}] lstate.result.size is :======================={}", __FILE__, __LINE__, lstate.result->num_rows());
        // spdlog::info("[{} : {}] lstate.result is :----------------------{}", __FILE__, __LINE__, lstate.result->ToString());
    }
    return arrow::Status::OK();
}

std::shared_ptr<LocalSinkState> PhysicalHashAgg::GetLocalSinkState() const {
    return std::make_shared<AggLocalSinkState>();
}

// Source 接口
SourceResultType PhysicalHashAgg::get_data(std::shared_ptr<ExecuteContext> &execute_context, LocalSourceState &input) const {
    auto &lstate = input.Cast<AggLocalSourceState>();
    auto weak_pipeline = execute_context->get_pipeline();
    if (auto pipeline = weak_pipeline.lock()) {
        if((lstate.result_size - lstate.idx_nums) >= (CHUNK_SIZE - 1)) {
            pipeline->temporary_chunk = lstate.result->Slice(lstate.idx_nums, CHUNK_SIZE);
            lstate.idx_nums += CHUNK_SIZE;
            return SourceResultType::HAVE_MORE_OUTPUT;
        } else {
            if(unlikely(lstate.idx_nums >= lstate.result_size)) {
                pipeline->temporary_chunk = arrow::RecordBatch::MakeEmpty(lstate.result->schema()).ValueOrDie();
                return SourceResultType::FINISHED;
            } else {
                int length = lstate.result_size - lstate.idx_nums;
                pipeline->temporary_chunk = lstate.result->Slice(lstate.idx_nums);
                lstate.idx_nums += length;
                return SourceResultType::HAVE_MORE_OUTPUT;
            }
        }
    }
    else
    {
        throw std::runtime_error("Invalid PhysicalHashAgg::pipeline");
    }
}

std::shared_ptr<LocalSourceState> PhysicalHashAgg::GetLocalSourceState() const {
    auto &lstate = (*lsink_state).Cast<AggLocalSinkState>();
    return std::make_shared<AggLocalSourceState>(lstate);
}

} // DaseX