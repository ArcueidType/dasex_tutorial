
//
// Created by root on 24-3-21.
//
#pragma once

#include "pipeline_group.hpp"
#include "scheduler.hpp"
#include "catalog.hpp"
#include <memory>
#include <vector>
#include <stack>
#include <spdlog/spdlog.h>
#include <mutex>
#include <condition_variable>
#include <chrono>

namespace DaseX
{

class Pipeline;
class PipelineGroup;

class PipelineGroupExecute : public std::enable_shared_from_this<PipelineGroupExecute> {
public:
    std::shared_ptr<PipelineGroup> plan;
    std::stack<std::shared_ptr<PipelineGroup> > pipe_stack;
    std::shared_ptr<Scheduler> &scheduler;
    std::mutex lock_;
    std::condition_variable cv_;
    int flag;
    std::chrono::duration<double, std::milli> duration;
public:
    PipelineGroupExecute(std::shared_ptr<Scheduler> &scheduler_, std::shared_ptr<PipelineGroup> plan_) : scheduler(
            scheduler_), plan(plan_) {}

    void execute();

    void plan_dfs(std::shared_ptr<PipelineGroup> root, int group_id);

    // 使用 DFS 遍历整个计划并将结果保存在 pipe_stack 中
    void traverse_plan();

    std::shared_ptr<PipelineGroupExecute> getptr() {
        return shared_from_this();
    }

}; // PipelineGroupExecute


}