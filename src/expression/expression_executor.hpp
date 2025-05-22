#pragma once

#include "execute_context.hpp"
#include "expression.hpp"
#include "expression_executor_state.hpp"
#include "bound_constant_expression.hpp"
#include "bound_conjunction_expression.hpp"
#include "bound_comparison_expression.hpp"
#include "bound_columnref_expression.hpp"
#include "bound_function_expression.hpp"
#include "bound_project_function_expression.hpp"
#include "bound_in_expression.hpp"
#include "bound_case_expression.hpp"
#include <arrow/api.h>
#include <vector>
#include <memory>

namespace DaseX
{
    class ExpressionExecutorState;
    class ExpressionState;
    // TODO: 将表达式执行器完善
    //! ExpressionExecutor is responsible for executing a set of expressions and storing the result in a data chunk
    class ExpressionExecutor : public std::enable_shared_from_this<ExpressionExecutor>
    {
    public:
        //! The expressions of the executor
        std::vector<std::shared_ptr<Expression>> expressions;
        // std::shared_ptr<ExecuteContext> execute_context;
        //! The states of the expression executor; this holds any intermediates and temporary states of expressions
        std::vector<std::weak_ptr<ExpressionExecutorState>> states;
        std::shared_ptr<arrow::RecordBatch> chunk;
        // Project再处理NULL数据时，为了获取Scheam，会传递一行带有默认值的数据，这行数据不用处理
        bool is_default_data = false;
    public:
        ExpressionExecutor()
        {
        }
        ExpressionExecutor(std::shared_ptr<Expression> expression) {
            expressions.push_back(expression);
        }
        ExpressionExecutor(std::vector<std::shared_ptr<Expression>> &exprs) {
            for(auto &expr : exprs) {
                expressions.push_back(expr);
            }
        }

    public:
        std::shared_ptr<ExpressionExecutor> GetPtr()
        {
            return shared_from_this();
        }

        inline void SetChunk(std::shared_ptr<arrow::RecordBatch> &chunk)
        {
            this->chunk = chunk;
        }

        void SetExecutorState(std::shared_ptr<ExpressionExecutorState> &state_);

        void SetExecutorStates(std::vector<std::shared_ptr<ExpressionExecutorState>> &states);

        void Initialize(Expression &expr, ExpressionExecutorState &state);

        static std::shared_ptr<ExpressionState> InitializeState(Expression &expr, ExpressionExecutorState &state);

        static std::shared_ptr<ExpressionState> InitializeState(BoundComparisonExpression &expr, ExpressionExecutorState &state);

        static std::shared_ptr<ExpressionState> InitializeState(BoundConjunctionExpression &expr, ExpressionExecutorState &state);

        static std::shared_ptr<ExpressionState> InitializeState(BoundConstantExpression &expr, ExpressionExecutorState &state);

        static std::shared_ptr<ExpressionState> InitializeState(BoundColumnRefExpression &expr, ExpressionExecutorState &state);

        static std::shared_ptr<ExpressionState> InitializeState(BoundFunctionExpression &expr, ExpressionExecutorState &state);

        static std::shared_ptr<ExpressionState> InitializeState(BoundProjectFunctionExpression &expr, ExpressionExecutorState &state);

        static std::shared_ptr<ExpressionState> InitializeState(BoundInExpression &expr, ExpressionExecutorState &state);

        static std::shared_ptr<ExpressionState> InitializeState(BoundCaseExpression &expr, ExpressionExecutorState &state);

        // ================================================Filter表达式-start================================================
        int SelectExpression(std::shared_ptr<arrow::RecordBatch> &input, std::vector<int> &sel);

        int Select(Expression &expr, std::shared_ptr<ExpressionState> state, int count, std::vector<int> &sel);

        int Select(BoundComparisonExpression &expr, std::shared_ptr<ExpressionState> state, int count, std::vector<int> &sel);

        int Select(BoundFunctionExpression &expr, std::shared_ptr<ExpressionState> state, int count, std::vector<int> &sel);

        int Select(BoundInExpression &expr, std::shared_ptr<ExpressionState> state, int count, std::vector<int> &sel);

        // TODO: ConjunctionExpression表达式目前只实现了and逻辑，后续补充or的实现
        int Select(BoundConjunctionExpression &expr, std::shared_ptr<ExpressionState> state, int count, std::vector<int> &sel);

        // 执行表达式，这里会根据表达式类型去调用对应表达式的Execute方法
        void Execute(Expression &expr, std::shared_ptr<ExpressionState> state, int count, std::vector<int> &sel, std::shared_ptr<arrow::Array> &result);

        // 列表达式执行
        void Execute(BoundColumnRefExpression &expr, std::shared_ptr<ExpressionState> state, int count, std::vector<int> &sel, std::shared_ptr<arrow::Array> &result);

        // 常量表达式执行
        void Execute(BoundConstantExpression &expr, std::shared_ptr<ExpressionState> state, int count, std::vector<int> &sel, std::shared_ptr<arrow::Array> &result);

        // 函数表达式执行
        void Execute(BoundFunctionExpression &expr, std::shared_ptr<ExpressionState> state, int count, std::vector<int> &sel, std::shared_ptr<arrow::Array> &result);
        // ================================================Filter表达式-end================================================

        // ================================================Project表达式-start================================================
        void Execute(std::shared_ptr<arrow::RecordBatch> &input, std::shared_ptr<arrow::RecordBatch> &result);

        void ExecuteExpression(Expression &expr, std::shared_ptr<ExpressionState> state, int idx, std::shared_ptr<arrow::RecordBatch> &result, std::shared_ptr<arrow::Array> &res, bool recursion = false, std::vector<int> sel = {});

        void ExecuteExpression(BoundColumnRefExpression &expr, std::shared_ptr<ExpressionState> state, int idx, std::shared_ptr<arrow::RecordBatch> &result, std::shared_ptr<arrow::Array> &res, bool recursion = false, std::vector<int> sel = {});

        void ExecuteExpression(BoundConstantExpression &expr, std::shared_ptr<ExpressionState> state, int idx, std::shared_ptr<arrow::RecordBatch> &result, std::shared_ptr<arrow::Array> &res, bool recursion = false, std::vector<int> sel = {});

        void ExecuteExpression(BoundProjectFunctionExpression &expr, std::shared_ptr<ExpressionState> state, int idx, std::shared_ptr<arrow::RecordBatch> &result, std::shared_ptr<arrow::Array> &res, bool recursion = false, std::vector<int> sel = {});

        void ExecuteExpression(BoundCaseExpression &expr, std::shared_ptr<ExpressionState> state, int idx, std::shared_ptr<arrow::RecordBatch> &result, std::shared_ptr<arrow::Array> &res, bool recursion = false, std::vector<int> sel = {});
        // ================================================Project表达式-end================================================

        // ================================================Then表达式-start================================================
        void ExecuteThenExpression(Expression &expr, std::shared_ptr<ExpressionState> state, int count, std::vector<int> &sel, std::vector<std::shared_ptr<Value>> &result, std::shared_ptr<arrow::Array> &array_res);

        void ExecuteThenExpression(BoundColumnRefExpression &expr, std::shared_ptr<ExpressionState> state, int count, std::vector<int> &sel, std::vector<std::shared_ptr<Value>> &result, std::shared_ptr<arrow::Array> &array_res);

        void ExecuteThenExpression(BoundConstantExpression &expr, std::shared_ptr<ExpressionState> state, int count, std::vector<int> &sel, std::vector<std::shared_ptr<Value>> &result, std::shared_ptr<arrow::Array> &array_res);

        void ExecuteThenExpression(BoundProjectFunctionExpression &expr, std::shared_ptr<ExpressionState> state, int count, std::vector<int> &sel, std::vector<std::shared_ptr<Value>> &result, std::shared_ptr<arrow::Array> &array_res);
        // ================================================Then表达式-end================================================
    }; // ExpressionExecutor

} // DaseX