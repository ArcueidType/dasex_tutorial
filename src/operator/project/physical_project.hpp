/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2024-04-01 19:33:51
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-04-01 19:33:52
 * @FilePath: /task_sche/src/operator/project/physical_project.hpp
 * @Description: 
 */
#pragma once

#include "execute_context.hpp"
#include "physical_operator.hpp"
#include "physical_operator_states.hpp"
#include "physical_operator_type.hpp"
#include "expression.hpp"
#include "expression_executor_state.hpp"
#include "expression_executor.hpp"
#include <arrow/api.h>
#include <spdlog/spdlog.h>

namespace DaseX
{

class PhysicalProject : public PhysicalOperator {
public:
    static constexpr const PhysicalOperatorType TYPE = PhysicalOperatorType::PROJECTION;
    // TODO: 兼容老版本，方便测试，后续删除
    std::vector<int> col_id;
    std::vector<std::shared_ptr<Expression>> expressions;
    std::vector<std::shared_ptr<ExpressionExecutorState>> states;
    std::shared_ptr<ExpressionExecutor> expressionExecutor;
    // For debug
    int process_nums = 0;
    std::vector<std::shared_ptr<arrow::RecordBatch>> project_result;
public:
    //! Table scan that immediately projects out filter columns that are unused in
    //! the remainder of the query plan
    PhysicalProject(std::vector<std::shared_ptr<Expression>> expressions);
    // TODO: 兼容老版本，方便测试，后续删除
    PhysicalProject(std::vector<int> col_id) : PhysicalOperator(PhysicalOperatorType::PROJECTION), col_id(std::move(col_id)) {}
    // Operator interface
    OperatorResultType execute_operator(std::shared_ptr<ExecuteContext> &execute_context) override;
    bool is_operator() const override { return true; }
    bool ParallelOperator() const override { return true; }
    std::shared_ptr<PhysicalOperator> Copy(int arg) override;
};

} // namespace DaseX
