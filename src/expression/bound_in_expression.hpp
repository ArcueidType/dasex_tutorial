#pragma once

#include "expression.hpp"
#include "expression_type.hpp"
#include "logical_type.hpp"
#include "value_type.hpp"

#include <vector>

namespace DaseX
{
    class BoundInExpression : public Expression
    {
    public:
        static constexpr const ExpressionClass TYPE = ExpressionClass::BOUND_IN;
        std::shared_ptr<Expression> left;
        std::vector<Value> value_set;
        LogicalType value_type; // left值类型
    public:
        BoundInExpression(std::shared_ptr<Expression> left,
                          std::vector<Value> value_set,
                          LogicalType value_type) : BoundInExpression(std::string(),
                                                                      left,
                                                                      value_set,
                                                                      value_type) {}
        BoundInExpression(std::string alias,
                          std::shared_ptr<Expression> left,
                          std::vector<Value> value_set,
                          LogicalType value_type) : Expression(ExpressionTypes::BOUND_IN, ExpressionClass::BOUND_IN, LogicalType::BOOL),
                                                    left(left),
                                                    value_set(value_set),
                                                    value_type(value_type)
        {
            this->alias = std::move(alias);
        }

        std::shared_ptr<Expression> Copy() override {
            auto expr = std::make_shared<BoundInExpression>(this->left, this->value_set,this->value_type);
            expr->alias = alias;
            return expr;
        }
    };



} // DaseX
