#pragma once

#include <string>
#include <utility>
#include <vector>

#include "abstract_expression.h"
#include "catalog.hpp"
#include "logical_type.hpp"

namespace DaseX {

    class LikeExpression : public AbstractExpression {
    public:
        LikeExpression(AbstractExpressionRef left, AbstractExpressionRef right,
                             bool not_like)
                : AbstractExpression({std::move(left), std::move(right)},
                                     LogicalType::BOOL,ExpressionClass::LIKE),
                  not_like_{not_like} {}

        static constexpr const ExpressionClass TYPE = ExpressionClass::LIKE;

        EXPR_CLONE_WITH_CHILDREN(LikeExpression);

        bool not_like_;
    };
} // namespace DaseX
