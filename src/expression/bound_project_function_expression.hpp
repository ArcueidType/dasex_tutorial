#pragma once

#include "expression.hpp"
#include "scalar_function.hpp"
#include <vector>

namespace DaseX {

    class BoundProjectFunctionExpression : public Expression {
    public:
        static constexpr const ExpressionClass TYPE = ExpressionClass::BOUND_PROJECT_FUNCTION;
    public:
        //! The bound function expression
        ScalarProjectFunction function;
        //! List of child-expressions of the function
        std::vector<std::shared_ptr<Expression>> children;
        //! The bound function data (if any)
        std::shared_ptr<FunctionData> bind_info;
    public:
        BoundProjectFunctionExpression(LogicalType return_type,
                                       ScalarProjectFunction bound_function,
                                       std::vector<std::shared_ptr<Expression>> arguments,
                                       std::shared_ptr<FunctionData> bind_info)
                : Expression(ExpressionTypes::BOUND_PROJECT_FUNCTION, ExpressionClass::BOUND_PROJECT_FUNCTION, std::move(return_type)),
                  function(std::move(bound_function)),
                  children(std::move(arguments)),
                  bind_info(std::move(bind_info)){}

        std::shared_ptr<Expression> Copy() override {
            auto new_bind_info = bind_info ? bind_info->Copy() : nullptr;
            std::vector<std::shared_ptr<Expression>> new_children;
            for (auto &child : children) {
                new_children.emplace_back(child->Copy());
            }
            auto expr = std::make_shared<BoundProjectFunctionExpression>(this->return_type, this->function, new_children, new_bind_info);
            expr->alias = alias;
            return expr;
        }
    }; // BoundFunctionExpression

} // DaseX
