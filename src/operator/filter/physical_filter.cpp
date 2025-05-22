#include "physical_filter.hpp"
#include "execute_context.hpp"
#include "pipeline.hpp"
#include "arrow_help.hpp"
#include <arrow/api.h>

namespace DaseX {

PhysicalFilter::PhysicalFilter(std::shared_ptr<Expression> expression)
        : PhysicalOperator(PhysicalOperatorType::FILTER),
          expression(expression){
    expressionExecutor = std::make_shared<ExpressionExecutor>(expression);
    executorState = std::make_shared<ExpressionExecutorState>();
    expressionExecutor->SetExecutorState(executorState);
    expressionExecutor->Initialize(*expression, *executorState);
}

OperatorResultType PhysicalFilter::execute_operator(std::shared_ptr<ExecuteContext> &execute_context) {
    auto weak_pipeline = execute_context->get_pipeline();
    if (auto pipeline = weak_pipeline.lock()) {
        std::shared_ptr<arrow::RecordBatch> &data_chunk = pipeline->temporary_chunk;
        if (pipeline->temporary_chunk == nullptr || data_chunk->num_rows() == 0) {
//            if(exp_name == "2") {
//                std::string filePath = "/home/cwl/data/filter2.csv";
//                Util::WriteToCSVFromRecordBatch(filter_result, filePath);
//                spdlog::info("[{} : {}] exp_name is : {} Filter size is :----------------------{}", __FILE__, __LINE__, exp_name, process_nums);
//            }
            return OperatorResultType::FINISHED;
        }
        std::vector<int> sel;
        int true_nums = expressionExecutor->SelectExpression(data_chunk, sel);
        if (sel.empty() || true_nums == 0) {
            data_chunk = arrow::RecordBatch::MakeEmpty(data_chunk->schema()).ValueOrDie();
            return OperatorResultType::NO_MORE_OUTPUT;
        }
        auto ok = DaseX::Util::RecordBatchSlice3(data_chunk, sel);
        process_nums += data_chunk->num_rows();
//        if(exp_name == "2") {
//            filter_result.push_back(data_chunk);
//            spdlog::info("[{} : {}] exp_name is : {} Filter result is :----------------------{}", __FILE__, __LINE__, exp_name, data_chunk->num_rows());
//            spdlog::info("[{} : {}] Filter result is :----------------------{}", __FILE__, __LINE__, data_chunk->ToString());
//        }
//        printf("%s\n",data_chunk->ToString().c_str());
        return OperatorResultType::HAVE_MORE_OUTPUT;
    } // if
    else {
        throw std::runtime_error("Invalid PhysicalFilter::pipeline");
    }
} // execute_operator

std::shared_ptr<PhysicalOperator> PhysicalFilter::Copy(int arg) {

    auto expression = std::make_shared<PhysicalFilter>(this->expression->Copy());
    expression->exp_name = this->exp_name;
    for(auto &en : this->children_operator) {
        expression->children_operator.emplace_back(en->Copy(arg));
    }
    return expression;
}

} // namespace DaseX
