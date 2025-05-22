/*
* @Author: caiwanli 651943559@qq.com
* @Date: 2024-03-18 15:03:22
* @LastEditors: caiwanli 651943559@qq.com
* @LastEditTime: 2024-03-18 16:34:51
* @FilePath: /task_sche/src/pipeline/pipeline_group.cpp
* @Description:
*/
#include "pipeline_group.hpp"
#include "pipeline.hpp"
#include "physical_table_scan.hpp"
#include "physical_order.hpp"

namespace DaseX {

PipelineGroup::PipelineGroup() : parallelism(0), comlete_pipe(0) {}

void PipelineGroup::Initialize() {
	// Step 1：判断是否为根PipelineGroup
	if(dependencies.empty()) {
		is_root = true;
	}
	int parallelism_count = 0;
	// Step 2：根据Source的类型初始化PipelineGroup，如果是TABLE_SCAN类型，这里需要根据表分区配置并行度，
	// 否则，需要从父PipelineGroup中获取相关信息。
	if(pipelines[0]->source->type == PhysicalOperatorType::TABLE_SCAN) {
		// Step 3: 获取表分区，几个分区对应几个并行度
		std::shared_ptr<Table> &table = (*(pipelines[0]->source)).Cast<PhysicalTableScan>().table;
		std::vector<int> partition_info;
		int partition_num = table->partition_num;
		std::vector<int> &partition_bitmap = table->partition_bitmap;
		for(int i = 0; i < partition_num; i++) {
			if(partition_bitmap[i] == 1) {
				partition_info.push_back(i);
				parallelism_count++;
			}
		}
		// Step 4: 根据并行度，Copy出对应数量的Pipeline，同时配置依赖关系
		// TODO: 这里需要补充代码（实现Pipeline::Copy()方法）

		// Step 5: 初始化每条Pipeline，配置输入输出
		for(int i = 0; i < parallelism_count; i++) {
			pipelines[i]->Initialize(i, partition_info[i]);
		}
	}
	// Step 6: 不是TABLE_SCAN类型，根据父PipelineGroup信息初始化PipelineGroup
	else {
		// Step 7: 从父PipelineGroup中获取并行度
		// TODO: 这里需要补充代码

		// Step 8: 根据取并行度Copy出对应数量的Pipeline，同时配置依赖关系
		// TODO: 这里需要补充代码

		// Step 9: 初始化每条Pipeline，配置输入输出
		for(int i = 0; i < parallelism_count; i++) {
			pipelines[i]->Initialize(i);
		}
	}

	//     Step 10: 对于一些必须要维护全局Sink状态的算子（例如，OrderBy），可以在这里配置
	if(pipelines[0]->sink && pipelines[0]->sink->type == PhysicalOperatorType::ORDER_BY) {
		gstate = pipelines[0]->sink->GetGlobalSinkState();
		auto &global_state = (*gstate).Cast<OrderGlobalSinkState>();
		global_state.local_cache.resize(parallelism_count);
		global_state.local_sort_idx.resize(parallelism_count);
		for(int i = 0; i < parallelism_count; i++) {
			pipelines[i]->sink->gsink_state = gstate;
		}
	}
}

std::vector<std::shared_ptr<Pipeline>> PipelineGroup::get_pipelines() {
    return pipelines;
}

void PipelineGroup::add_pipeline(const std::shared_ptr<Pipeline> &pipeline) {
    pipelines.push_back(pipeline);
    pipeline->pipelines = this->getptr();
    pipeline->pipeline_id = parallelism;
    parallelism++;
} // add_pipeline

void PipelineGroup::add_dependence(std::shared_ptr<PipelineGroup> dependence) {
    dependencies.push_back(dependence);
}

void PipelineGroup::create_child_group(std::shared_ptr<PhysicalOperator> &current) {
	// Step 1: 获取算子类型，根据类型判断如何构建子PipelineGroup
	auto operator_type = current->type;
	// Step 2: 只有sink算子会创建子PipelineGroup
	// Step 3: JOIN也属于sink，需要特殊处理
	if(current->children_operator.size() > 1) {
		// Step 4: JOIN有多种类型，RadixJoin为两端阻塞，需要特殊处理，这里只需要完成HashJoin部分即可
		switch (operator_type) {
			case PhysicalOperatorType::RADIX_JOIN:
			{
				// Probe
				auto &child_operator_probe = current->children_operator[1];
				auto child_pipeline_group_probe = std::make_shared<PipelineGroup>();
				auto child_pipeline_probe = std::make_shared<Pipeline>();
				std::shared_ptr<ExecuteContext> child_execute_context_probe = std::make_shared<ExecuteContext>();
				child_execute_context_probe->set_pipeline(child_pipeline_probe);
				child_pipeline_group_probe->add_pipeline(child_pipeline_probe);
				dependencies.push_back(child_pipeline_group_probe);
				child_pipeline_probe->sink = current;
				child_pipeline_probe->child = pipelines[0];
				pipelines[0]->parent = child_pipeline_probe;
				child_operator_probe->build_pipelines(child_pipeline_probe, child_pipeline_group_probe);
				// Build
				auto &child_operator_build = current->children_operator[0];
				auto child_pipeline_group_build = std::make_shared<PipelineGroup>();
				auto child_pipeline_build = std::make_shared<Pipeline>();
				std::shared_ptr<ExecuteContext> child_execute_context_build = std::make_shared<ExecuteContext>();
				child_execute_context_build->set_pipeline(child_pipeline_build);
				child_pipeline_group_build->add_pipeline(child_pipeline_build);
				child_pipeline_group_probe->dependencies.push_back(child_pipeline_group_build);
				child_pipeline_build->sink = current;
				child_pipeline_build->child = child_pipeline_probe;
				child_pipeline_probe->parent = child_pipeline_build;
				child_operator_build->build_pipelines(child_pipeline_build, child_pipeline_group_build);
				break;
			}
			case PhysicalOperatorType::NESTED_LOOP_JOIN:
			case PhysicalOperatorType::HASH_JOIN:
			{
				// Step 5: 处理Build端的子PipelineGroup构建
				// Step 5.1: 获取Build端算子，当前算子的左孩子
				// TODO: 这里需要补充代码
				auto &child_operator_build = current->children_operator[0];
				// Step 5.2: 新建PipelineGroup以及Pipeline，并将Pipeline添加至PipelineGroup
				// TODO: 这里需要补充代码
				auto child_pipeline_group_build = std::make_shared<PipelineGroup>();
				auto child_pipeline_build = std::make_shared<Pipeline>();
				std::shared_ptr<ExecuteContext> child_execute_context_build = std::make_shared<ExecuteContext>();
				child_execute_context_build->set_pipeline(child_pipeline_build);
				child_pipeline_group_build->add_pipeline(child_pipeline_build);
				// Step 5.3: 设置PipelineGroup之间的依赖关系
				// TODO: 这里需要补充代码
				this->dependencies.push_back(child_pipeline_group_build);
				// Step 5.4: 配置Pipeline的sink，同时设置Pipeline之间的依赖关系
				// TODO: 这里需要补充代码
				child_pipeline_build->sink = current;
				child_pipeline_build->child = this->pipelines[0];
				if (this->pipelines[0]->parent == nullptr) {
					pipelines[0]->parent = child_pipeline_build;
				}
				// Step 5.5: 递归构建Pipeline
				// TODO: 这里需要补充代码
				child_operator_build->build_pipelines(child_pipeline_build, child_pipeline_group_build);
				break;
			}
			default:
				break;
		} // switch
	}
	// Step 6: 正常sink处理
	else {
		// Step 6.1: 获取当前算子的孩子
		// TODO: 这里需要补充代码
		auto &child_operator = current->children_operator[0];
		// Step 6.2: 构建子PipelineGroup和Pipeline，并将Pipeline添加至PipelineGroup
		// TODO: 这里需要补充代码
		auto child_pipeline_group = std::make_shared<PipelineGroup>();
		auto child_pipeline = std::make_shared<Pipeline>();
		std::shared_ptr<ExecuteContext> child_execute_context = std::make_shared<ExecuteContext>();
		child_execute_context->set_pipeline(child_pipeline);
		child_pipeline_group->add_pipeline(child_pipeline);
		// Step 6.3: 配置PipelineGroup依赖关系
		// TODO: 这里需要补充代码
		this->dependencies.push_back(child_pipeline_group);
		// Step 6.4: 配置Pipeline的sink，同时设置Pipeline之间的依赖关系
		// TODO: 这里需要补充代码
		child_pipeline->sink = current;
		child_pipeline->child = this->pipelines[0];
		this->pipelines[0]->parent = child_pipeline;
		// Step 6.5: 递归构建Pipeline
		// TODO: 这里需要补充代码
		child_operator->build_pipelines(child_pipeline, child_pipeline_group);
	}
} // create_child_group

std::shared_ptr<Barrier> PipelineGroup::get_barrier() {
    std::lock_guard<std::mutex> lock(barrier_mutex);
    if (barrier == nullptr) {
        barrier = std::make_shared<Barrier>(parallelism);
        return barrier;
    } else {
        return barrier;
    }
} // get_barrier


} // namespace DaseX
