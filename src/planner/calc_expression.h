//
// Created by zyy on 24-10-22.
//

#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>
#include "catalog.hpp"
#include "abstract_expression.h"
#include "value_type.hpp"

namespace DaseX {

    enum class CalcType {
        Plus, Minus, Multiply, Divide
    };

    static std::string CalcType2Str(CalcType type) {
        if (type == CalcType::Plus)
            return "+";
        if (type == CalcType::Minus)
            return "-";
        if (type == CalcType::Multiply)
            return "*";
        if (type == CalcType::Divide)
            return "/";
        return "?";
    }

    class CalcExpression : public AbstractExpression {
    public:
        /** Creates a new comparison expression representing (left comp_type right). */
        CalcExpression(AbstractExpressionRef left, AbstractExpressionRef right, CalcType compute_type)
                : AbstractExpression({std::move(left), std::move(right)}, LogicalType::FLOAT,
                                     ExpressionClass::FUNCTION),
                  compute_type_{compute_type} {

        }

        EXPR_CLONE_WITH_CHILDREN(CalcExpression);

        CalcType compute_type_;

        [[nodiscard]] auto ToString() const -> std::string {
            return "{" + children_[0]->ToString() + CalcType2Str(compute_type_) + children_[1]->ToString() + "}";
        }
    };
}