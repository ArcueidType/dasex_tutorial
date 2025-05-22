#pragma once

#include "expression.hpp"
#include "logical_type.hpp"
#include "expression_type.hpp"

#include <vector>

namespace DaseX
{

    class BoundCaseExpression : public Expression
    {
    public:
        static constexpr const ExpressionClass TYPE = ExpressionClass::BOUND_CASE;
        std::vector<std::shared_ptr<Expression>> when_exprs;
        std::vector<std::shared_ptr<Expression>> then_exprs;
        std::shared_ptr<Expression> else_expr;

    public:
        BoundCaseExpression()
                : Expression(ExpressionTypes::CASE_EXPR, ExpressionClass::BOUND_CASE, LogicalType::BOOL) {}

        BoundCaseExpression(std::vector<std::shared_ptr<Expression>> when_exprs,
                            std::vector<std::shared_ptr<Expression>> then_exprs,
                            std::shared_ptr<Expression> else_expr)
                : Expression(ExpressionTypes::CASE_EXPR, ExpressionClass::BOUND_CASE, LogicalType::BOOL),
                  when_exprs(std::move(when_exprs)),
                  then_exprs(std::move(then_exprs)),
                  else_expr(else_expr) {}
    public:
        void SetWhenExpr(std::shared_ptr<Expression> when_expr) { when_exprs.push_back(when_expr); }
        void SetThenExpr(std::shared_ptr<Expression> then_expr) { then_exprs.push_back(then_expr); }
        void SetElseExpr(std::shared_ptr<Expression> else_expr) { this->else_expr = else_expr; }

        std::shared_ptr<Expression> Copy() override {
            auto expression = std::make_shared<BoundCaseExpression>();
            int size = this->when_exprs.size();
            for(int i = 0; i < size; i++) {
                expression->SetWhenExpr(this->when_exprs[i]->Copy());
                expression->SetThenExpr(this->then_exprs[i]->Copy());
            }
            expression->SetElseExpr(this->else_expr->Copy());
            expression->alias = alias;
            return expression;
        }
    };
} // DaseX

