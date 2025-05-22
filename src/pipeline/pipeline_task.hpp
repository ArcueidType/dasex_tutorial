/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2024-03-18 21:32:41
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-03-19 11:45:19
 * @FilePath: /task_sche/src/pipeline/pipeline_task.hpp
 * @Description:
 */
#pragma once

#include "execute_context.hpp"
#include "filter_condition.hpp"
#include "physical_filter.hpp"
#include "physical_operator.hpp"
#include "physical_project.hpp"
#include "physical_table_scan.hpp"
#include "pipeline.hpp"
#include "pipeline_result_type.hpp"
#include "common_macro.hpp"

namespace DaseX
{

class PipelineTask {
public:
    std::shared_ptr<ExecuteContext> execute_context;
    std::shared_ptr<Pipeline> pipeline;

public:
    PipelineTask(std::shared_ptr<Pipeline> pipeline_) : pipeline(pipeline_) {
        execute_context = pipeline->execute_context;
    }

    void operator()() {
        while (true) {
            PipelineResultType result = pipeline->execute_pipeline();
            if (result == PipelineResultType::FINISHED) {
                auto pipeline_group = pipeline->get_pipeline_group();
                if (auto pipe_group = pipeline_group.lock()) {
                    if (unlikely(pipe_group->is_finish())) {
                        if (auto pipe_executor = pipe_group->executor.lock()) {
                            // 通知主线程当前PipelineGroup中所有pipeline执行完成，可以开始调度下一个PipelineGroup
                            pipe_executor->cv_.notify_one();
                        } else {
                            throw std::runtime_error("Invalid PipelineTask::pipe_executor");
                        }
                    }
                    else {
                        // TODO: 这里是否可以窃取其他Pipeline中的数据进行处理？？？
                        // 第一个做完的线程来评估一下整个PipelineGroup的任务执行情况
                        // 如果存在倾斜，即某几个线程只是完成任务的50%或者70%，那么该线程充当领导者，重新调度任务
                        // 1. 通知任务重的线程，我要指派线程帮助你（此时，get_data要上锁）
                        // 2. 计算距离，指派最近的线程去帮助(即，创建一个或几个新的Pipelinetask，并指定从哪里获取数据，提交至线程)
                    }
                } else {
                    throw std::runtime_error("Invalid PipelineTask::pipe_group");
                }
                break;
            }
        }
    }
}; // PipelineTask

} // namespace DaseX