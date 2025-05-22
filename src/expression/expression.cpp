#include "expression.hpp"

namespace DaseX {

std::shared_ptr<Expression> Expression::Copy() {
    auto expr = std::make_shared<Expression>(this->type, this->expression_class, this->return_type);
    expr->alias = alias;
    return expr;
}


} // DaseX
