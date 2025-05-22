#pragma once

#include "expression_executor_state.hpp"
#include "bound_project_function_expression.hpp"
#include "binary_executor.hpp"
#include "numeric_operator_type.hpp"
#include <vector>
#include <memory>

namespace DaseX {

// TODO: 这里的执行器是逐元素操作，应该实现一个向量化的版本，即直接对数组进行操作，否则会大量重复调用Operation方法，后续需要优化
// +
class AddOperator {
public:
    AddOperator() {}
public:
    template<class LEFT_TYPE, class RIGHT_TYPE, class RESULT_TYPE>
    static inline RESULT_TYPE Operation(LEFT_TYPE left, RIGHT_TYPE right) {
        return left + right;
    }
}; // AddOperator

static void RegularAddFunction(std::vector<std::shared_ptr<arrow::Array>> &input, ExpressionState &state, std::shared_ptr<arrow::Array> &result) {
    auto &left = input[0];
    auto &right = input[1];
    auto scala_left = left->GetScalar(0).ValueOrDie();
    std::shared_ptr<arrow::DataType> type_left = scala_left->type;
    auto scala_right = right->GetScalar(0).ValueOrDie();
    auto type_right = scala_right->type;
    if(type_left->id() == arrow::Type::type::INT32 && type_right->id() == arrow::Type::type::FLOAT) {
        BinaryExecutor::Execute<int, float, float, AddOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::INT32 && type_right->id() == arrow::Type::type::DOUBLE) {
        BinaryExecutor::Execute<int, double, double, AddOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::FLOAT && type_right->id() == arrow::Type::type::INT32) {
        BinaryExecutor::Execute<float, int, float, AddOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::FLOAT && type_right->id() == arrow::Type::type::FLOAT) {
        BinaryExecutor::Execute<float, float, float, AddOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::FLOAT && type_right->id() == arrow::Type::type::DOUBLE) {
        BinaryExecutor::Execute<float, double, double, AddOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::DOUBLE && type_right->id() == arrow::Type::type::INT32) {
        BinaryExecutor::Execute<double, int, double, AddOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::DOUBLE && type_right->id() == arrow::Type::type::FLOAT) {
        BinaryExecutor::Execute<double, float, double, AddOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::DOUBLE && type_right->id() == arrow::Type::type::DOUBLE) {
        BinaryExecutor::Execute<double, double, double, AddOperator>(left, right, result, left->length());
    }
    else {
        BinaryExecutor::Execute<int, int, int, AddOperator>(left, right, result, left->length());
    }
} // RegularNumericAddFunction



    inline scalar_function_p GetAddFunction() {

    scalar_function_p function = &RegularAddFunction;
    return function;
}

// -
class SubOperator {
public:
    SubOperator() {}
public:
    template<class LEFT_TYPE, class RIGHT_TYPE, class RESULT_TYPE>
    static inline RESULT_TYPE Operation(LEFT_TYPE left, RIGHT_TYPE right) {
        return left - right;
    }
}; // SubOperator

static void RegularSubFunction(std::vector<std::shared_ptr<arrow::Array>> &input, ExpressionState &state, std::shared_ptr<arrow::Array> &result) {
    auto &left = input[0];
    auto &right = input[1];
    auto scala_left = left->GetScalar(0).ValueOrDie();
    std::shared_ptr<arrow::DataType> type_left = scala_left->type;
    auto scala_right = right->GetScalar(0).ValueOrDie();
    auto type_right = scala_right->type;
    if(type_left->id() == arrow::Type::type::INT32 && type_right->id() == arrow::Type::type::FLOAT) {
        BinaryExecutor::Execute<int, float, float, SubOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::INT32 && type_right->id() == arrow::Type::type::DOUBLE) {
        BinaryExecutor::Execute<int, double, double, SubOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::FLOAT && type_right->id() == arrow::Type::type::INT32) {
        BinaryExecutor::Execute<float, int, float, SubOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::FLOAT && type_right->id() == arrow::Type::type::FLOAT) {
        BinaryExecutor::Execute<float, float, float, SubOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::FLOAT && type_right->id() == arrow::Type::type::DOUBLE) {
        BinaryExecutor::Execute<float, double, double, SubOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::DOUBLE && type_right->id() == arrow::Type::type::INT32) {
        BinaryExecutor::Execute<double, int, double, SubOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::DOUBLE && type_right->id() == arrow::Type::type::FLOAT) {
        BinaryExecutor::Execute<double, float, double, SubOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::DOUBLE && type_right->id() == arrow::Type::type::DOUBLE) {
        BinaryExecutor::Execute<double, double, double, SubOperator>(left, right, result, left->length());
    }
    else {
        BinaryExecutor::Execute<int, int, int, SubOperator>(left, right, result, left->length());
    }
} // RegularNumericAddFunction

    inline scalar_function_p GetSubFunction() {

    scalar_function_p function = &RegularSubFunction;
    return function;
}

// *
class MulOperator {
public:
    MulOperator() {}
public:
    template<class LEFT_TYPE, class RIGHT_TYPE, class RESULT_TYPE>
    static inline RESULT_TYPE Operation(LEFT_TYPE left, RIGHT_TYPE right) {
        return left * right;
    }
}; // MulOperator

static void RegularMulFunction(std::vector<std::shared_ptr<arrow::Array>> &input, ExpressionState &state, std::shared_ptr<arrow::Array> &result) {
    auto &left = input[0];
    auto &right = input[1];
    auto scala_left = left->GetScalar(0).ValueOrDie();
    std::shared_ptr<arrow::DataType> type_left = scala_left->type;
    auto scala_right = right->GetScalar(0).ValueOrDie();
    auto type_right = scala_right->type;
	// printf("letf type is %s , right type is %s\n",type_left->ToString().c_str(),type_right->ToString().c_str());
    if(type_left->id() == arrow::Type::type::INT32 && type_right->id() == arrow::Type::type::FLOAT) {
        BinaryExecutor::Execute<int, float, float, MulOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::INT32 && type_right->id() == arrow::Type::type::DOUBLE) {
        BinaryExecutor::Execute<int, double, double, MulOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::FLOAT && type_right->id() == arrow::Type::type::INT32) {
        BinaryExecutor::Execute<float, int, float, MulOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::FLOAT && type_right->id() == arrow::Type::type::FLOAT) {
        BinaryExecutor::Execute<float, float, float, MulOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::FLOAT && type_right->id() == arrow::Type::type::DOUBLE) {
        BinaryExecutor::Execute<float, double, double, MulOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::DOUBLE && type_right->id() == arrow::Type::type::INT32) {
        BinaryExecutor::Execute<double, int, double, MulOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::DOUBLE && type_right->id() == arrow::Type::type::FLOAT) {
        BinaryExecutor::Execute<double, float, double, MulOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::DOUBLE && type_right->id() == arrow::Type::type::DOUBLE) {
        BinaryExecutor::Execute<double, double, double, MulOperator>(left, right, result, left->length());
    }
    else {
        BinaryExecutor::Execute<int, int, int, MulOperator>(left, right, result, left->length());
    }
} // RegularNumericAddFunction


    inline scalar_function_p GetMulFunction() {

    scalar_function_p function = &RegularMulFunction;
    return function;
}

// ÷
class DivOperator {
public:
    DivOperator() {}
public:
    template<class LEFT_TYPE, class RIGHT_TYPE, class RESULT_TYPE>
    static inline RESULT_TYPE Operation(LEFT_TYPE left, RIGHT_TYPE right) {
        return left / right;
    }
}; // MulOperator

static void RegularDivFunction(std::vector<std::shared_ptr<arrow::Array>> &input, ExpressionState &state, std::shared_ptr<arrow::Array> &result) {
    auto &left = input[0];
    auto &right = input[1];
    auto scala_left = left->GetScalar(0).ValueOrDie();
    std::shared_ptr<arrow::DataType> type_left = scala_left->type;
    auto scala_right = right->GetScalar(0).ValueOrDie();
    auto type_right = scala_right->type;
    if(type_left->id() == arrow::Type::type::INT32 && type_right->id() == arrow::Type::type::FLOAT) {
        BinaryExecutor::Execute<int, float, float, DivOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::INT32 && type_right->id() == arrow::Type::type::DOUBLE) {
        BinaryExecutor::Execute<int, double, double, DivOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::FLOAT && type_right->id() == arrow::Type::type::INT32) {
        BinaryExecutor::Execute<float, int, float, DivOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::FLOAT && type_right->id() == arrow::Type::type::FLOAT) {
        BinaryExecutor::Execute<float, float, float, DivOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::FLOAT && type_right->id() == arrow::Type::type::DOUBLE) {
        BinaryExecutor::Execute<float, double, double, DivOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::DOUBLE && type_right->id() == arrow::Type::type::INT32) {
        BinaryExecutor::Execute<double, int, double, DivOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::DOUBLE && type_right->id() == arrow::Type::type::FLOAT) {
        BinaryExecutor::Execute<double, float, double, DivOperator>(left, right, result, left->length());
    }
    else if(type_left->id() == arrow::Type::type::DOUBLE && type_right->id() == arrow::Type::type::DOUBLE) {
        BinaryExecutor::Execute<double, double, double, DivOperator>(left, right, result, left->length());
    }
    else {
        BinaryExecutor::Execute<int, int, int, DivOperator>(left, right, result, left->length());
    }
} // RegularNumericAddFunction


    inline scalar_function_p GetDivFunction() {

    scalar_function_p function = &RegularDivFunction;
    return function;
}






} // DaseX
