#include "physical_operator.hpp"
#include "pipeline.hpp"

namespace DaseX {

// common
void PhysicalOperator::build_pipelines(std::shared_ptr<Pipeline> &current,
                                       std::shared_ptr<PipelineGroup> &pipelineGroup) {
	// Step 1: 判断当前算子是否为sink？
	if (is_sink()) {
		// Step 2: 将当前算子设置为当前pipeline的source
		// TODO: 这里需要补充代码
		current->source = this->getptr();
		// Step 3: 判断PipelineGroup中是否已经存在Pipeline
		if(pipelineGroup->pipelines.empty()) {
			// Step 4: 初始化Pipeline执行上下文，同时将当前Pipeline添加至PipelineGroup中
			// TODO: 这里需要补充代码
			std::shared_ptr<ExecuteContext> execute_ctx = std::make_shared<ExecuteContext>();
			execute_ctx->set_pipeline(current);
			pipelineGroup->add_pipeline(current);
		}
		// Step 5: 从当前算子开始，递归的构建子PipelineGroup
		// TODO: 这里需要补充代码
		pipelineGroup->create_child_group(current->source);
	}
	// Step 6: 如果不是sink
	else {
		// Step 7: 判断是否为Source
		if (children_operator.empty()) {
			// Step 8: 将当前算子设置为当前pipeline的Source
			// TODO: 这里需要补充代码
			current->source = this->getptr();
		}
		// Step 9: 否则为Operator
		else {
			// Step 10: 判断子孩子是否为1，如果大于1说明为JOIN算子，需要特殊处理，即JOIN需要重载该方法
			if (children_operator.size() != 1) {
				throw std::runtime_error("Operator not supported in BuildPipelines");
			}
			// Step 11: 正常Operator，添加至当前Pipeline中
			// TODO: 这里需要补充代码
			current->operators.push_back(this->getptr());
			// Step 12: 递归执行子Operator
			// TODO: 这里需要补充代码
			children_operator[0]->build_pipelines(current, pipelineGroup);
		}
	}
}

void PhysicalOperator::AddChild(std::shared_ptr<PhysicalOperator> child) {
    children_operator.emplace_back(child);
}
// Source interface
std::shared_ptr<LocalSourceState> PhysicalOperator::GetLocalSourceState() const {
    return std::make_shared<LocalSourceState>();
}

std::shared_ptr<LocalSinkState> PhysicalOperator::GetLocalSinkState() const {
    return std::make_shared<LocalSinkState>();
}

std::shared_ptr<GlobalSinkState> PhysicalOperator::GetGlobalSinkState() const {
    return std::make_shared<GlobalSinkState>();
}



} // DaseX