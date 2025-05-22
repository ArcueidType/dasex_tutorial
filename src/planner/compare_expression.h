#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "abstract_expression.h"
#include "catalog.hpp"
#include "exception.h"
#include "logical_type.hpp"

namespace DaseX {

enum class ComparisonType : uint8_t {
  Equal,
  NotEqual,
  LessThan,
  LessThanOrEqual,
  GreaterThan,
  GreaterThanOrEqual,
  INVALID
};

static std::map<ComparisonType, std::string> comparisonType2Str = {
  {ComparisonType::Equal, "="},
  {ComparisonType::NotEqual, "!="},
  {ComparisonType::LessThan, "<"},
  {ComparisonType::LessThanOrEqual, "<="},
  {ComparisonType::GreaterThan, ">"},
  {ComparisonType::GreaterThanOrEqual, ">="}
};

static ComparisonType OperatorToComparisonType(const std::string &op) {
	if (op == "=" || op == "==") {
		return ComparisonType::Equal;
	} else if (op == "!=" || op == "<>") {
		return ComparisonType::NotEqual;
	} else if (op == "<") {
		return ComparisonType::LessThan;
	} else if (op == ">") {
		return ComparisonType::GreaterThan;
	} else if (op == "<=") {
		return ComparisonType::LessThanOrEqual;
	} else if (op == ">=") {
		return ComparisonType::GreaterThanOrEqual;
	}
	return ComparisonType::INVALID;
}


static ComparisonType NegateComparisonType(ComparisonType type) {
	ComparisonType negated_type = ComparisonType::INVALID;
	switch (type) {
	case ComparisonType::Equal:
		negated_type = ComparisonType::NotEqual;
		break;
	case ComparisonType::NotEqual:
		negated_type = ComparisonType::Equal;
		break;
	case ComparisonType::LessThan:
		negated_type = ComparisonType::GreaterThanOrEqual;
		break;
	case ComparisonType::GreaterThan:
		negated_type = ComparisonType::LessThanOrEqual;
		break;
	case ComparisonType::LessThanOrEqual:
		negated_type = ComparisonType::GreaterThan;
		break;
	case ComparisonType::GreaterThanOrEqual:
		negated_type = ComparisonType::LessThan;
		break;
	default:
		throw NotImplementedException("Unsupported comparison type in negation");
	}
	return negated_type;
}

class ComparisonExpression : public AbstractExpression {
public:
  ComparisonExpression(AbstractExpressionRef left, AbstractExpressionRef right,
                       ComparisonType comp_type)
      : AbstractExpression({std::move(left), std::move(right)},
                           LogicalType::BOOL,ExpressionClass::COMPARISON),
        comp_type_{comp_type} {}

    static constexpr const ExpressionClass TYPE = ExpressionClass::COMPARISON;

  EXPR_CLONE_WITH_CHILDREN(ComparisonExpression);

  ComparisonType comp_type_;

  [[nodiscard]] auto ToString() const -> std::string {
    return children_[0]->ToString() + " " + comparisonType2Str[comp_type_] + " " + children_[1]->ToString();
  }
};
} // namespace DaseX
