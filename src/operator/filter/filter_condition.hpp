#pragma once

#include "value_type.hpp"
#include <string>

namespace DaseX {

enum class FilterType {
  // 常数比较（e.g. =C, >C, >=C, <C, <=C）
  CONSTANT_COMPARISON,
  IS_NULL,
  IS_NOT_NULL,
  CONJUNCTION_OR,
  CONJUNCTION_AND,
  INVALID
};

enum class ConstantCompareType {
  COMPARE_EQUAL,               //  =
  COMPARE_NOTEQUAL,            //  !=
  COMPARE_LESSTHAN,            //  <
  COMPARE_GREATERTHAN,         //  >
  COMPARE_LESSTHANOREQUALTO,   //  <=
  COMPARE_GREATERTHANOREQUALTO //  >=
};

class Condition {
public:
  static constexpr const FilterType TYPE = FilterType::INVALID;
  virtual std::string to_string() const { return "INVALID"; }
};

class ConstantCompareCondition : public Condition {
public:
  static constexpr const FilterType TYPE = FilterType::CONSTANT_COMPARISON;
  ConstantCompareType compare_type;
  Value constant;

public:
  explicit ConstantCompareCondition(ConstantCompareType compare_type_, int val)
      : compare_type(compare_type_), constant(val) {}
  explicit ConstantCompareCondition(ConstantCompareType compare_type_,
                                    float val)
      : compare_type(compare_type_), constant(val) {}
  explicit ConstantCompareCondition(ConstantCompareType compare_type_,
                                    double val)
      : compare_type(compare_type_), constant(val) {}
  std::string to_string() const override { return "CONSTANT_COMPARISON"; }
};

} // namespace DaseX
