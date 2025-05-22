#pragma once

#include "expression.hpp"
#include "logical_type.hpp"
#include "expression_type.hpp"

namespace DaseX
{

    class BoundComparisonExpression : public Expression
    {
    public:
        static constexpr const ExpressionClass TYPE = ExpressionClass::BOUND_COMPARISON;
        std::shared_ptr<Expression> left;
        std::shared_ptr<Expression> right;

    public:
        BoundComparisonExpression(ExpressionTypes type)
            : Expression(type, ExpressionClass::BOUND_COMPARISON, LogicalType::BOOL) {}

        BoundComparisonExpression(ExpressionTypes type, std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
            : Expression(type, ExpressionClass::BOUND_COMPARISON, LogicalType::BOOL),
              left(std::move(left)),
              right(std::move(right)) {}
    public:
        void SetLeft(std::shared_ptr<Expression> left) { this->left = std::move(left); }
        void SetRight(std::shared_ptr<Expression> right) { this->right = std::move(right); }

        std::shared_ptr<Expression> Copy() override {
            auto expression = std::make_shared<BoundComparisonExpression>(this->type);
            expression->left = this->left->Copy();
            expression->right = this->right->Copy();
            expression->alias = alias;
            return expression;
        }
    };
} // DaseX
