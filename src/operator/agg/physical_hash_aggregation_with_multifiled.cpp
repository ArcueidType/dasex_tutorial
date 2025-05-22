#include "physical_hash_aggregation_with_multifield.hpp"
#include "pipeline.hpp"
#include "arrow_help.hpp"
#include "common_macro.hpp"
#include <spdlog/spdlog.h>

namespace DaseX {

PhysicalMultiFieldHashAgg::PhysicalMultiFieldHashAgg(std::vector<int> group_set,
                                                     std::vector<int> agg_set,
                                                     std::vector<int> star_bitmap,
                                                     std::vector<AggFunctionType> aggregate_function_types)
        : PhysicalOperator(PhysicalOperatorType::MULTI_FIELD_HASH_GROUP_BY),
          group_set(std::move(group_set)),
          agg_set(std::move(agg_set)),
          star_bitmap(std::move(star_bitmap)),
          aggregate_function_types(std::move(aggregate_function_types)) {}

// Common interface
std::shared_ptr<PhysicalOperator> PhysicalMultiFieldHashAgg::Copy(int arg) {
    auto physicalOperator = std::make_shared<PhysicalMultiFieldHashAgg>(group_set, agg_set, star_bitmap, aggregate_function_types);
    physicalOperator->exp_name = this->exp_name;
    return physicalOperator;
}

// Source 接口实现
SourceResultType PhysicalMultiFieldHashAgg::get_data(std::shared_ptr<ExecuteContext> &execute_context, LocalSourceState &input) const {
    auto &lstate = input.Cast<MultiFieldAggLocalSourceState>();
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

std::shared_ptr<LocalSourceState> PhysicalMultiFieldHashAgg::GetLocalSourceState() const {
    auto &lstate = (*lsink_state).Cast<MultiFieldAggLocalSinkState>();
    return std::make_shared<MultiFieldAggLocalSourceState>(lstate);
}

// Sink 接口实现
SinkResultType PhysicalMultiFieldHashAgg::sink(std::shared_ptr<ExecuteContext> &execute_context, LocalSinkState &output) {
    auto weak_pipeline = execute_context->get_pipeline();
    auto &lstate = output.Cast<MultiFieldAggLocalSinkState>();
    if (auto pipeline = weak_pipeline.lock()) {
        auto &chunk = pipeline->temporary_chunk;
        if (!pipeline->is_final_chunk) {
            if(unlikely(lstate.result_schema == nullptr)) {
                lstate.result_schema = chunk->schema();
            }
            lstate.processed_rows += chunk->num_rows();
            if(chunk->num_rows() != 0) {
                local_agg(chunk, lstate); // 第一阶段: 构建本地聚合表，中间结果做分区
            }
            return SinkResultType::NEED_MORE_INPUT;
        } else {
            if(unlikely(lstate.result_schema == nullptr)) {
                lstate.result_schema = chunk->schema();
            }
            auto pipeline_group = pipeline->get_pipeline_group();
            if (auto pipe_group = pipeline_group.lock())
            {
                auto barrier = pipe_group->get_barrier(); // 等待所有线程处理完，准备进入第二阶段
                barrier->wait();
                auto status = global_agg(lstate, pipe_group, pipeline, pipeline->pipeline_id); // 第二阶段: Global Hash Agg, 每个线程并发的去做全局聚合
                // assert(status == arrow::Status::OK());
                // spdlog::info("[{} : {}] -----------agg result size is {}", __FILE__, __LINE__, lstate.result->num_rows());
//                std::string fileName = "Q1_" + std::to_string(pipeline->pipeline_id) + ".csv";
//                std::string filePath = "/home/cwl/data/tpch_result/" + fileName;
//                if(lstate.result->num_rows() != 0) {
//                    std::vector<std::shared_ptr<arrow::RecordBatch>> tmp = {lstate.result};
//                    Util::WriteToCSVFromRecordBatch(tmp, filePath);
//                }
                return SinkResultType::FINISHED;
            } else {
                throw std::runtime_error("pipeline group error");
            }
        } // else
    } else {
        throw std::runtime_error("Invalid PhysicalHashAgg::pipeline");
    }
}

void PhysicalMultiFieldHashAgg::local_agg(std::shared_ptr<arrow::RecordBatch> &chunk, MultiFieldAggLocalSinkState &state) const {
    auto &partition_table = state.partition_table;
	printf("chunk is %s",chunk->ToString().c_str());
    partition_table->InsertData(chunk);
}

arrow::Status PhysicalMultiFieldHashAgg::global_agg(MultiFieldAggLocalSinkState &lstate,
                                                    std::shared_ptr<PipelineGroup> &pipeline_group,
                                                    std::shared_ptr<Pipeline> &pipeline, size_t pipeline_id) const {
    const auto &pipelines = pipeline_group->get_pipelines();
    assert(pipelines.size() > 0);
    int agg_size = agg_set.size();
    int group_size = group_set.size();
    // 合并对应分区
    auto &local_hashtables = lstate.partition_table;
    auto local_hashtable = local_hashtables->GetAggHashTable(pipeline_id);
    auto &local_agg_buckets = local_hashtable->buckets;
    for(int i = 0;  i < pipelines.size(); i++) {
        auto &other_state = pipelines[i]->output->Cast<MultiFieldAggLocalSinkState>();
        if(i == pipeline_id) {
            continue;
        }
        auto other_hashtable = other_state.partition_table->GetAggHashTable(pipeline_id);
        // 合并其它pipeline中的聚合状态
        local_hashtable->MergeState(other_hashtable);
    } // for
    // 将聚合结果转换成Arrow RecordBatch
    //    Step1: 转换成RB前，先构建好schema
    std::vector<std::shared_ptr<arrow::Field>> new_fields;
    auto &old_schema = lstate.result_schema;
    std::vector<std::string> old_names = old_schema->field_names();
    //    Step1.1: 构建分组字段
    for(int i = 0; i < group_size; i++) {
        new_fields.emplace_back(old_schema->field(group_set[i]));
    }
    //    Step1.2: 构建聚合字段
    for(int i = 0; i < agg_size; i++) {
        switch (aggregate_function_types[i]) {
            case AggFunctionType::COUNT:
            {
                std::string old_name = old_names[agg_set[i]];
                std::string new_name = "Count(" + old_name + ")";
                std::shared_ptr<arrow::Field> new_field = std::make_shared<arrow::Field>(new_name, arrow::int32());
                new_fields.emplace_back(new_field);
                break;
            }
            case AggFunctionType::AVG:
            {
                std::string old_name = old_names[agg_set[i]];
                std::string new_name = "Avg(" + old_name + ")";
                std::shared_ptr<arrow::Field> new_field = std::make_shared<arrow::Field>(new_name, arrow::float64());
                new_fields.emplace_back(new_field);
                break;
            }
            case AggFunctionType::MIN:
            {
                std::string old_name = old_names[agg_set[i]];
                std::string new_name = "Min(" + old_name + ")";
                std::shared_ptr<arrow::Field> new_field = std::make_shared<arrow::Field>(new_name, arrow::float64());
                new_fields.emplace_back(new_field);
                break;
            }
            case AggFunctionType::MAX:
            {
                std::string old_name = old_names[agg_set[i]];
                std::string new_name = "Max(" + old_name + ")";
                std::shared_ptr<arrow::Field> new_field = std::make_shared<arrow::Field>(new_name, arrow::float64());
                new_fields.emplace_back(new_field);
                break;
            }
            case AggFunctionType::SUM:
            {
                std::string old_name = old_names[agg_set[i]];
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
    //    Step2: 构建好schema后，就可以遍历AggHashTable中的所有数据，然后转换为RB
    int column_nums = agg_size + group_size;
    std::vector<std::shared_ptr<arrow::Array>> new_columns(column_nums);
    std::vector<std::vector<std::shared_ptr<Value>>> key_values(column_nums);
    auto &bucket_map = local_hashtable->bucket_map;
    int count_row = 0;
    for(int i = 0; i < bucket_map.size(); i++) {
        if(bucket_map[i] == 0) {
            continue;
        }
        auto &bucket_p = local_agg_buckets[i];
        int num_rows_p = bucket_p->num_rows;
        for(int j = 0; j < num_rows_p; j++) {
            auto &entrySet_p = bucket_p->bucket[j];
            if(entrySet_p != nullptr) {
                AddOneRowToValueArray(key_values, entrySet_p, count_row++, column_nums);
            }
        }
    }
    arrow::Status ok = FillArrayVector(new_columns, key_values, column_nums);
    if(count_row != 0) {
        lstate.result = arrow::RecordBatch::Make(new_schema, count_row, new_columns);
    } else {
        lstate.result = arrow::RecordBatch::MakeEmpty(new_schema).ValueOrDie();
    }
    return ok;
}

void PhysicalMultiFieldHashAgg::AddOneRowToValueArray(std::vector<std::vector<std::shared_ptr<Value>>> &key_values,
                                                      std::shared_ptr<EntrySet> &entrySet, int row_idx, int column_nums) const {
    std::vector<std::shared_ptr<Value>> value_row = entrySet->GetFormatRow();
    int key_nums = group_set.size();
    for(int i = 0; i < column_nums; i++) {
        auto &value_p = key_values[i];
        value_p.emplace_back(value_row[i]);
        // value_p[row_idx] = value_row[i];
    }
}

arrow::Status PhysicalMultiFieldHashAgg::FillArrayVector(std::vector<std::shared_ptr<arrow::Array>> &columns,
                                                std::vector<std::vector<std::shared_ptr<Value>>> &values,
                                                int column_nums) const {
    arrow::Status ok;
    for (int i = 0; i < column_nums; i++) {
        auto &value_p = values[i];
        auto &array_p = columns[i];
        ok = Util::AppendValueArray(array_p, value_p);
    }
    return ok;
}

std::shared_ptr<LocalSinkState> PhysicalMultiFieldHashAgg::PhysicalMultiFieldHashAgg::GetLocalSinkState() const {
    return std::make_shared<MultiFieldAggLocalSinkState>(group_set, agg_set, star_bitmap, aggregate_function_types);
}

} // DaseX