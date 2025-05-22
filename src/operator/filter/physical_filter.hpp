#pragma once

#include "physical_operator.hpp"
#include "physical_operator_type.hpp"
#include "expression.hpp"
#include "expression_executor.hpp"
#include "filter_condition.hpp"
#include <spdlog/spdlog.h>
#include <vector>

namespace DaseX {

class ExecuteContext;

class PhysicalFilter : public PhysicalOperator {
public:
    static constexpr const PhysicalOperatorType TYPE = PhysicalOperatorType::FILTER;
    //! The filter expression
    std::shared_ptr<Expression> expression;
    std::shared_ptr<ExpressionExecutor> expressionExecutor;
    std::shared_ptr<ExpressionExecutorState> executorState;
    // For Debug
    int process_nums = 0;
    std::vector<std::shared_ptr<arrow::RecordBatch>> filter_result;
public:
    PhysicalFilter(std::shared_ptr<Expression> expression);
    // PhysicalFilter(COLUMN_ID idx_, ConstantCompareCondition condition_) : PhysicalOperator(PhysicalOperatorType::FILTER), idx(idx_), condition(condition_) {}

    OperatorResultType execute_operator(std::shared_ptr<ExecuteContext> &execute_context) override;

    bool is_operator() const { return true; }
    bool ParallelOperator() const { return true; }
    std::shared_ptr<PhysicalOperator> Copy(int arg) override;
};

} // namespace DaseX
