#pragma once

#include "execute_context.hpp"
#include "physical_operator.hpp"
#include "physical_operator_states.hpp"
#include "physical_operator_type.hpp"
#include "pipeline_group.hpp"
#include "pipeline_result_type.hpp"
#include <arrow/api.h>
#include <memory>
#include <spdlog/spdlog.h>
#include <vector>

namespace DaseX
{

class PhysicalOperator;

class ExecuteContext;

class Pipeline : public std::enable_shared_from_this<Pipeline> {
public:
    constexpr static uint64_t max_chunks = 10000000000000;
    std::weak_ptr<PipelineGroup> pipelines;     // pipeline所在的组
    std::shared_ptr<Pipeline> parent = nullptr; // 依赖的pipiline
    std::shared_ptr<Pipeline> child = nullptr;  // 被依赖的pipiline
    std::shared_ptr<PhysicalOperator> source;
    std::vector<std::shared_ptr<PhysicalOperator>> operators;
    std::shared_ptr<PhysicalOperator> sink;
    std::shared_ptr<arrow::RecordBatch> temporary_chunk;
    std::shared_ptr<ExecuteContext> execute_context;
    std::shared_ptr<LocalSourceState> input;
    std::shared_ptr<LocalSinkState> output;
    bool is_root = false;       // 标记是否是根pipeline
    size_t pipeline_id = 0;
    int work_id = -1; // 执行pipeline的Worker编号，提交任务时需要，-1表示无效的id，必须要设置
    bool is_final_chunk;
    bool is_finish;
public:
    Pipeline();
    Pipeline(std::shared_ptr<LocalSourceState> input, std::shared_ptr<LocalSinkState> output);
    std::shared_ptr<Pipeline> getptr() { return shared_from_this(); }
    // 执行Pipeline之前需要先做初始化动作，这个方法的主要目的就是设置pipeline的input、output以及work_id
    void Initialize(int pipe_idx, int work_id = -1);
    void add_dependency(std::shared_ptr<Pipeline> parent);
    void set_child(std::shared_ptr<Pipeline> child);
    // pipeline必须要有一个source，但不一定要有sink和operator
    PipelineResultType execute_pipeline();
    std::weak_ptr<PipelineGroup> get_pipeline_group() { return pipelines; }
    void set_work_id(int work_id) { this->work_id = work_id; }
    std::shared_ptr<Pipeline> Copy(int arg);
}; // Pipeline

} // DaseX
