/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2024-03-18 15:56:51
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-03-18 21:24:13
 * @FilePath: /task_sche/src/pipeline/pipeline_group.hpp
 * @Description:
 */
#pragma once
#include "pipeline_group_execute.hpp"
#include "physical_operator.hpp"
#include "barrier.hpp"
#include "common_macro.hpp"
#include "pipeline_group_execute.hpp"
#include "physical_operator_states.hpp"
#include "catalog.hpp"
#include "task_type.hpp"
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

namespace DaseX {

class Pipeline;
class PipelineGroupExecute;
class PhysicalOperator;
/**
 * @description: 一组具有相同功能的Pipeline的集合
 */
class PipelineGroup : public std::enable_shared_from_this<PipelineGroup> {
public:
    std::vector<std::shared_ptr<Pipeline>> pipelines;
    std::vector<std::shared_ptr<PipelineGroup>> dependencies;   // 一般来说这里只会有一个依赖，但是Union all的话就会存在多个依赖
    std::weak_ptr<PipelineGroupExecute> executor;
    std::shared_ptr<GlobalSinkState> gstate;
    bool is_root = false;        // 是否是根pipeline_group
    int16_t parallelism = 0;
    std::shared_ptr<Barrier> barrier;
    std::mutex barrier_mutex;
    std::atomic<int> comlete_pipe;  // 用于统计pipeline完成数量
    int group_id = 0;               // 标识当前PipelineGroup，方便调试时跟踪pipeline_group的执行情况
    TaskType event = TaskType::Invalid;
    bool extra_task = false;        // 标识额外任务是否完成
public:
    PipelineGroup();

    // 循环调用pipeline的Initialize()
    void Initialize();

    std::vector<std::shared_ptr<Pipeline>> get_pipelines();

    void add_pipeline(const std::shared_ptr<Pipeline> &pipeline);

    void create_child_group(std::shared_ptr<PhysicalOperator> &current);

    void add_dependence(std::shared_ptr<PipelineGroup> dependence);

    std::shared_ptr<Barrier> get_barrier();

    void add_count() {
        if (unlikely(comlete_pipe >= parallelism)) {
            return;
        }
        comlete_pipe++;
    }

    bool is_finish() { return comlete_pipe.load() == parallelism; }

    std::shared_ptr<PipelineGroup> getptr() { return shared_from_this(); }
};

} // namespace DaseX
