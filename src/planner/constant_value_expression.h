#pragma once

#include <memory>
#include <string>
#include <vector>

#include "abstract_expression.h"
#include "logical_type.hpp"
#include "value_type.hpp"

namespace DaseX {

class ConstantValueExpression : public AbstractExpression {
public:
  explicit ConstantValueExpression(const Value &val)
      : AbstractExpression({}, val.type_,ExpressionClass::CONSTANT), val_(val) {}

    static constexpr const ExpressionClass TYPE = ExpressionClass::CONSTANT;

  EXPR_CLONE_WITH_CHILDREN(ConstantValueExpression);

  Value val_;
  [[nodiscard]] auto ToString() const -> std::string {
    std::string logicalType;
    switch (val_.type_) {
      case LogicalType::INTEGER:
        logicalType = "int";
        break;
      case LogicalType::FLOAT:
        logicalType = "float";
        break;
      case LogicalType::BOOL:
        logicalType = "bool";
        break;
      case LogicalType::DOUBLE:
        logicalType = "double";
        break;
      case LogicalType::STRING:
        logicalType = "str";
        break;
      default:
        break;
    }
    return std::to_string(val_.value_.int_) + "(" + logicalType + ")";
  }
};
} // namespace DaseX
