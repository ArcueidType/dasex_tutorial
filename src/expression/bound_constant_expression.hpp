#pragma once

#include "expression.hpp"
#include "value_type.hpp"
#include "expression_type.hpp"

namespace DaseX
{

    class BoundConstantExpression : public Expression
    {
    public:
        static constexpr const ExpressionClass TYPE = ExpressionClass::BOUND_CONSTANT;
        Value value;

    public:
        explicit BoundConstantExpression(Value value_p) :
                            Expression(ExpressionTypes::VALUE_CONSTANT,
                                       ExpressionClass::BOUND_CONSTANT,
                                       value_p.type_),
                            value(std::move(value_p)) {}

        std::shared_ptr<Expression> Copy() override {
            auto expression = std::make_shared<BoundConstantExpression>(this->value.Copy());
            expression->alias = alias;
            return expression;
        }
    };
} // DaseX
