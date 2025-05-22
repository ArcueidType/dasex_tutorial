#pragma once


#include "abstract_expression.h"
#include "expression.hpp"
#include "numeric.hpp"


namespace DaseX {

class ExpressionTransformer {
public:
    static auto Transform(AbstractExpressionRef expr) -> ExpressionRef;
};

} // namespace DaseX