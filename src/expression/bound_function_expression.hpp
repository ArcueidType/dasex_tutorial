#pragma once

#include "expression.hpp"
#include "scalar_function.hpp"
#include <vector>

namespace DaseX {

class BoundFunctionExpression : public Expression {
public:
    static constexpr const ExpressionClass TYPE = ExpressionClass::BOUND_FUNCTION;
public:
    //! The bound function expression
    ScalarFunction function;
    //! List of child-expressions of the function
    std::vector<std::shared_ptr<Expression>> children;
    //! The bound function data (if any)
    std::shared_ptr<FunctionData> bind_info;
public:
    BoundFunctionExpression(LogicalType return_type,
                            ScalarFunction bound_function,
                            std::vector<std::shared_ptr<Expression>> arguments,
                            std::shared_ptr<FunctionData> bind_info)
              : Expression(ExpressionTypes::BOUND_FUNCTION, ExpressionClass::BOUND_FUNCTION, std::move(return_type)),
                function(std::move(bound_function)),
                children(std::move(arguments)),
                bind_info(std::move(bind_info)){}

    std::shared_ptr<Expression> Copy() override {
        auto new_bind_info = bind_info ? bind_info->Copy() : nullptr;
        std::vector<std::shared_ptr<Expression>> new_children;
        for (auto &child : children) {
            new_children.emplace_back(child->Copy());
        }
        auto expr = std::make_shared<BoundFunctionExpression>(this->return_type, this->function, new_children, new_bind_info);
        expr->alias = alias;
        return expr;
    }
}; // BoundFunctionExpression

} // DaseX
