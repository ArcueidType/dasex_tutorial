
#pragma once

#include <string>
#include <utility>
#include <vector>

#include "catalog.hpp"
#include "abstract_expression.h"

namespace DaseX {

enum class LogicType { And, Or };

/**
 * LogicExpression represents two expressions being computed.
 */
class LogicExpression : public AbstractExpression {
public:

  LogicExpression(AbstractExpressionRef left, AbstractExpressionRef right, LogicType logic_type)
      : AbstractExpression({std::move(left), std::move(right)},LogicalType::BOOL,ExpressionClass::CONJUNCTION), logic_type_{logic_type} {
    if (GetChildAt(0)->GetReturnType() != LogicalType::BOOL || GetChildAt(1)->GetReturnType() != LogicalType::BOOL) {
      printf("expect boolean from either side");
    }
  }
  static constexpr const ExpressionClass TYPE = ExpressionClass::CONJUNCTION;

  EXPR_CLONE_WITH_CHILDREN(LogicExpression);
  LogicType logic_type_;

  [[nodiscard]] auto ToString() const -> std::string {
    std::string conj = logic_type_ == LogicType::And ? "AND" : "OR";
    return children_[0]->ToString() + " " + conj + " " + children_[1]->ToString();
  }
};

}
