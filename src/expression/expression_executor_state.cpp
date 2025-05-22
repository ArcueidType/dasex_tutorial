#include "expression_executor_state.hpp"
#include "expression_executor.hpp"

namespace DaseX {

    void ExpressionState::AddChild(Expression &expr, ExpressionExecutorState &state)
    {
        child_states.push_back(ExpressionExecutor::InitializeState(expr, state));
    }


} // DaseX