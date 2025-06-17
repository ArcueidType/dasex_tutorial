#pragma once

#include "pipeline_group.hpp"
#include "pipeline.hpp"
#include "logical_type.hpp"
#include "arrow_help.hpp"
#include "physical_order.hpp"
#include <memory>
#include <arrow/api.h>
#include <arrow/compute/api.h>

namespace DaseX {

class MergeSortTask {
public:
    std::shared_ptr<PipelineGroup> pipeline_group;
public:
    MergeSortTask(std::shared_ptr<PipelineGroup> pipeline_group) : pipeline_group(pipeline_group) {}
    void operator()() {
        auto &gstate = pipeline_group->gstate;
        auto &global_sink_state = (*gstate).Cast<OrderGlobalSinkState>();
        std::vector<std::shared_ptr<arrow::RecordBatch>> &tar = global_sink_state.local_cache;
        std::vector<std::shared_ptr<arrow::UInt64Array>> &tar_idx = global_sink_state.local_sort_idx;
        std::vector<int> *sort_keys = global_sink_state.sort_keys;
        std::vector<LogicalType> *key_types = global_sink_state.key_types;
        std::vector<SortOrder> *orders = global_sink_state.orders;
        std::vector<arrow::compute::SortKey> arrow_sort_keys;
        int col_nums = sort_keys->size();
        std::shared_ptr<arrow::RecordBatch> tar_p;
        // 这里获取本地排序数据不为空的一个RB
        for(int k = 0; k < tar.size(); k++) {
            if(tar[k] != nullptr) {
                tar_p = tar[k];
                break;
            }
        }
        if(tar_p != nullptr) {
            for(int i = 0; i < col_nums; i++) {
                // std::string col_name = tar_p->column_name((*sort_keys)[i]);
                int column_index = (*sort_keys)[i];
                SortOrder inter_order = (*orders)[i];
                arrow::compute::SortOrder order = inter_order == SortOrder::DESCENDING ? arrow::compute::SortOrder::Descending : arrow::compute::SortOrder::Ascending;
                arrow_sort_keys.emplace_back(arrow::compute::SortKey(column_index, order));
            }
            std::vector<std::shared_ptr<arrow::RecordBatch>> &res = global_sink_state.global_sort_data;
            // spdlog::info("[{} : {}] MergeSortTask结果: {}", __FILE__, __LINE__, res.size());
            // TODO: 将res更换为vector，避免ConcatenateRecordBatches动作
            Util::MergeRecordBatch(tar, tar_idx, res, &arrow_sort_keys, key_types);
//            std::string filePath = "/home/cwl/data/tpch_result/Q11.csv";
//            Util::WriteToCSVFromRecordBatch(res, filePath);
            // 清空缓存数据
            global_sink_state.local_cache.clear();
            pipeline_group->extra_task = true;
            // 唤醒执行器，继续执行下一个PipelineGroup
            if (auto pipe_executor = pipeline_group->executor.lock()) {
                // 通知主线程当前PipelineGroup中所有pipeline执行完成，可以开始调度下一个PipelineGroup
                pipe_executor->cv_.notify_one();
            } else {
                throw std::runtime_error("Invalid PipelineTask::pipe_executor");
            }
        } else {
            pipeline_group->extra_task = true;
            // 唤醒执行器，继续执行下一个PipelineGroup
            if (auto pipe_executor = pipeline_group->executor.lock()) {
                // 通知主线程当前PipelineGroup中所有pipeline执行完成，可以开始调度下一个PipelineGroup
                pipe_executor->cv_.notify_one();
            } else {
                throw std::runtime_error("Invalid PipelineTask::pipe_executor");
            }
        }
    } // operator()
};

} // DaseX
