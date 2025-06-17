// Created by root on 24-3-21.
//
/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2024-03-18 21:21:20
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-03-26 10:06:47
 * @FilePath: /task_sche/src/pipeline/execute/pipeline_group_execute.cpp
 * @Description:
 */
#include "pipeline_group_execute.hpp"
#include "execute_context.hpp"
#include "physical_table_scan.hpp"
#include "join/radixjoin/physical_radix_join.hpp"
#include "pipeline.hpp"
#include "pipeline_task.hpp"
#include "merge_sort_task.hpp"
#include <thread>
#include <chrono>

namespace DaseX {

void PipelineGroupExecute::traverse_plan() {
	// Step 1: 使用栈来保存遍历PipelineGroup树的结果，在每次遍历前清空上一次的结果
	// TODO: 这里需要补充代码
    this->pipe_stack = std::stack<std::shared_ptr<PipelineGroup>>();
    
	// Step 2： 使用 DFS 遍历整个计划并将结果保存在 pipe_stack 中
	// TODO: 这里需要补充代码
    plan_dfs(this->plan, 0);
}

void PipelineGroupExecute::plan_dfs(std::shared_ptr<PipelineGroup> root, int group_id) {
	// Step 1: 设置PipelineGroup的执行器以及group_id
	root->executor = this->getptr();
	root->group_id = group_id;
	// Step 2: 将当前节点保存至 pipe_stack 中，同时递增group_id
	// TODO: 这里需要补充代码
    this->pipe_stack.push(root);
    group_id++;
	// Step 3: 递归访问子节点
	// TODO: 这里需要补充代码
    for (auto &dependency : root->dependencies) {
        plan_dfs(dependency, group_id);
    }
}

void PipelineGroupExecute::execute() {
    while (!pipe_stack.empty()) {
        auto start_execute = std::chrono::steady_clock::now();
		// Step 1: 从栈中依次取出一个PipelineGroup节点执行
        std::shared_ptr<PipelineGroup> node = pipe_stack.top();
        pipe_stack.pop();
		// Step 2: 执行之前需要初始化PipelineGroup，所谓初始化其实就是生成并行Pipeline，完成并行执行，
		// 同时在生成Pipeline的过程中配置依赖关系以及设置输入输出关系，即要将当前Pipeline的Sink设置为子Pipeline的Source
        node->Initialize();
        int size = node->pipelines.size();
        std::vector<PipelineTask> pipeline_tasks;
		// Step 3: 执行PipelineGroup内部的Pipeline，即构造pipeline_task并提交到对应的工作线程中
        for (int i = 0; i < size; i++) {
            int work_id = node->pipelines[i]->work_id;
            PipelineTask pipeline_task(node->pipelines[i]);
            pipeline_tasks.push_back(pipeline_task);
            scheduler->submit_task(pipeline_task, work_id, true);
        }

        // TODO:加锁同步，完成一个pipeline_group后，再执行下一个pipeline_group
        {
            std::unique_lock<std::mutex> lock(lock_);
            cv_.wait(lock, [this, &node] { return node->is_finish(); });
        }
        // 查看注册事件，例如MergeSort，MergeJoin等
        if(node->event != TaskType::Invalid) {
            switch (node->event) {
                case TaskType::MergeSortTask :
                {
                    // TODO: 提交合并排序任务给线程池，并且需要等待排序任务完成
                    MergeSortTask merge_sort_task(node);
                    scheduler->submit_task(merge_sort_task, node->pipelines[0]->work_id, true);
                    // 等待合并排序任务完成
                    {
                        std::unique_lock<std::mutex> lock_sort(lock_);
                        cv_.wait(lock_sort, [this, &node] { return node->extra_task; });
                    }
                    break;
                }
                case TaskType::MergeJoinTask :
                {
                    // TODO: 后续可能会有MergeJoinTask
                    break;
                }
                default:
                    break;
            }
        } // if
        auto end_execute = std::chrono::steady_clock::now();
        spdlog::info("[{} : {}] pipeline执行时间: {}", __FILE__, __LINE__, Util::time_difference(start_execute, end_execute));
    }
}


} // DaseX
