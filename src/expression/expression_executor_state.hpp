#pragma once

#include "execute_context.hpp"
#include "expression.hpp"
#include <arrow/api.h>
#include <vector>
#include <memory>

namespace DaseX
{

    class ExpressionExecutorState;
    class ExpressionExecutor;

    class ExpressionState : public std::enable_shared_from_this<ExpressionState>
    {
    public:
        ExpressionState(std::shared_ptr<Expression> expr, std::shared_ptr<ExpressionExecutorState> root) : expr(expr), root(root)
        {
        }

        virtual ~ExpressionState()
        {
        }

        std::shared_ptr<Expression> expr;
        std::weak_ptr<ExpressionExecutorState> root;
        std::vector<std::shared_ptr<ExpressionState>> child_states;
        std::shared_ptr<arrow::Array> left_column; // 记录临时修改后的列值
        std::shared_ptr<arrow::Array> right_column;

    public:
        void AddChild(Expression &expr, ExpressionExecutorState &state);
        // std::shared_ptr<ExecuteContext> &GetContext();

    public:
        template <class TARGET>
        TARGET &Cast()
        {
            return reinterpret_cast<TARGET &>(*this);
        }
        template <class TARGET>
        const TARGET &Cast() const
        {
            return reinterpret_cast<const TARGET &>(*this);
        }
    };

    class CaseExpressionState : public ExpressionState, std::enable_shared_from_this<CaseExpressionState> {
    public:
        std::vector<int> true_sel;
        std::vector<int> false_sel;
    public:
        CaseExpressionState(std::shared_ptr<Expression> expr, std::shared_ptr<ExpressionExecutorState> root)
                : ExpressionState(expr, root) {}
    };

    //! ExpressionExecutor is responsible for executing a set of expressions and storing the result in a data chunk
    class ExpressionExecutorState : public std::enable_shared_from_this<ExpressionExecutorState>
    {
    public:
        std::shared_ptr<ExpressionState> root_state;
        std::shared_ptr<ExpressionExecutor> executor;

    public:
        ExpressionExecutorState() {}
    };

} // DaseX