#pragma once

#include "filter_condition.hpp"
#include "logical_type.hpp"
#include <string>
#include <vector>
#include <memory>

namespace DaseX {

class ScalarFunction;
class ScalarProjectFunction;

class FunctionData {
public:
    virtual std::shared_ptr<FunctionData> Copy() const = 0;
public:
    template <class TARGET>
    TARGET &Cast() {
        return reinterpret_cast<TARGET &>(*this);
    }
    template <class TARGET>
    const TARGET &Cast() const {
        return reinterpret_cast<const TARGET &>(*this);
    }
};

class Function {
public:
  //! The name of the function
  std::string name;
  //! Additional Information to specify function from it's name
  std::string extra_info;
public:
  explicit Function(std::string name_p);
};

class SimpleFunction : public Function {
public:
    //! 函数参数类型
    std::vector<LogicalType> arguments_type;
    //! 要支持的变长参数的类型，如果函数不接受变长参数，则为 LogicalType::INVALID
    LogicalType varargs;
    SimpleFunction(std::string name, std::vector<LogicalType> arguments, LogicalType varargs = LogicalType::INVALID);
};

class BaseScalarFunction : public SimpleFunction {
public:
    //! Return type of the function
    LogicalType return_type;
    BaseScalarFunction(std::string name, std::vector<LogicalType> arguments, LogicalType return_type, LogicalType varargs = LogicalType::INVALID);
};




/// ========================================比较函数开始========================================
template <typename T> bool compare_equal(T &left, T &right) {
  return left == right;
}

template <typename T> bool compare_no_equal(T &left, T &right) {
  return left != right;
}

template <typename T> bool compare_less_than(T &left, T &right) {
  return left < right;
}

template <typename T> bool compare_greater_than(T &left, T &right) {
  return left > right;
}

template <typename T> bool compare_less_than_or_equalto(T &left, T &right) {
  return left < right || left == right;
}

template <typename T> bool compare_greater_than_or_equalto(T &left, T &right) {
  return left > right || left == right;
}

template <typename T> class CompareFunction : public Function {
public:
  using compare_function_ = bool (*)(T &, T &);
  compare_function_ function;
  ConstantCompareType type;

public:
  CompareFunction(ConstantCompareType type_)
      : Function("CompareFunction"), type(type_) {
    switch (type) {
    case ConstantCompareType::COMPARE_EQUAL:
      function = compare_equal<T>;
      break;
    case ConstantCompareType::COMPARE_NOTEQUAL:
      function = compare_no_equal<T>;
      break;
    case ConstantCompareType::COMPARE_LESSTHAN:
      function = compare_less_than<T>;
      break;
    case ConstantCompareType::COMPARE_GREATERTHAN:
      function = compare_greater_than<T>;
      break;
    case ConstantCompareType::COMPARE_LESSTHANOREQUALTO:
      function = compare_less_than_or_equalto<T>;
      break;
    case ConstantCompareType::COMPARE_GREATERTHANOREQUALTO:
      function = compare_greater_than_or_equalto<T>;
      break;
    default:
      break;
    }
  }
};

/// ========================================比较函数结束========================================

} // namespace DaseX
