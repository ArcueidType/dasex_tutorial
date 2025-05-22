#pragma once

#include "expression.hpp"
#include "logical_type.hpp"
#include "expression_type.hpp"
#include <vector>

namespace DaseX
{

    class BoundConjunctionExpression : public Expression
    {
    public:
        static constexpr const ExpressionClass TYPE = ExpressionClass::BOUND_CONJUNCTION;
        std::vector<std::shared_ptr<Expression>> children;

    public:
        explicit BoundConjunctionExpression(ExpressionTypes expression_type) : Expression(expression_type, ExpressionClass::BOUND_CONJUNCTION, LogicalType::BOOL) {}
        BoundConjunctionExpression(ExpressionTypes expression_type, std::shared_ptr<Expression> left, std::shared_ptr<Expression> right) : BoundConjunctionExpression(expression_type)
        {
            children.push_back(std::move(left));
            children.push_back(std::move(right));
        }

    public:
        void AddExpression(std::shared_ptr<Expression> expr)
        {
            children.push_back(std::move(expr));
        }

        std::shared_ptr<Expression> Copy() override {
            auto expression = std::make_shared<BoundConjunctionExpression>(this->type);
            for(auto &en : this->children) {
                expression->children.emplace_back(en->Copy());
            }
            expression->alias = alias;
            return expression;
        }
    };
} // DaseX
