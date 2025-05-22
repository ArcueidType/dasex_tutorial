#include "physical_project.hpp"
#include "pipeline.hpp"
#include "arrow_help.hpp"

namespace DaseX {

PhysicalProject::PhysicalProject(std::vector<std::shared_ptr<Expression>> expressions) : PhysicalOperator(PhysicalOperatorType::PROJECTION), expressions(std::move(expressions)) {
    expressionExecutor = std::make_shared<ExpressionExecutor>(this->expressions);
    for(int i = 0; i < this->expressions.size(); i++) {
        auto state = std::make_shared<ExpressionExecutorState>();
        states.emplace_back(state);
    }
    expressionExecutor->SetExecutorStates(states);
}

OperatorResultType PhysicalProject::execute_operator(std::shared_ptr<ExecuteContext> &execute_context) {
    auto weak_pipeline = execute_context->get_pipeline();
    if (auto pipeline = weak_pipeline.lock()) {
        std::shared_ptr<arrow::RecordBatch> &data_chunk = pipeline->temporary_chunk;
        if (data_chunk->num_rows() == 0) {
            auto schema = data_chunk->schema();
            Util::GetSinlgeRow(data_chunk, schema);
            std::shared_ptr<arrow::RecordBatch> result;
            expressionExecutor->is_default_data = true;
            expressionExecutor->Execute(data_chunk, result);
            data_chunk = arrow::RecordBatch::MakeEmpty(result->schema()).ValueOrDie();
//            if(exp_name == "1") {
//                std::string filePath = "/home/cwl/data/project2.csv";
//                Util::WriteToCSVFromRecordBatch(project_result, filePath);
//                spdlog::info("[{} : {}] exp_name is : {} Project size is :----------------------{}", __FILE__, __LINE__, exp_name, process_nums);
//            }
            return OperatorResultType::FINISHED;
        }
        std::shared_ptr<arrow::RecordBatch> result;
        expressionExecutor->Execute(data_chunk, result);
        data_chunk = result;
        process_nums += data_chunk->num_rows();
//        if(exp_name == "4") {
//            project_result.push_back(data_chunk);
//            spdlog::info("[{} : {}] project {} result size is :----------------------{}", __FILE__, __LINE__, exp_name, data_chunk->num_rows());
//            spdlog::info("[{} : {}] project {} result is :----------------------{}", __FILE__, __LINE__, exp_name, data_chunk->ToString());
//        }
        return OperatorResultType::HAVE_MORE_OUTPUT;
    } // if
    else {
        throw std::runtime_error("Invalid PhysicalFilter::pipeline");
    }
} // execute_operator

std::shared_ptr<PhysicalOperator> PhysicalProject::Copy(int partition_id) {
    std::vector<std::shared_ptr<Expression>> expressions_p;
    for(auto &expr : expressions) {
        expressions_p.push_back(expr->Copy());
    }
    auto physicalProject = std::make_shared<PhysicalProject>(expressions_p);
    physicalProject->exp_name = this->exp_name;
    for(auto &en : this->children_operator) {
        physicalProject->children_operator.emplace_back(en->Copy(partition_id));
    }
    return physicalProject;
}

} // DaseX
