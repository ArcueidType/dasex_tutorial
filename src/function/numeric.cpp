#include "numeric.hpp"

namespace DaseX {

// TODO: 所有操作均未做溢出判断，太复杂了，后续有时间在优化
// +
template<>
int AddOperator::Operation(int left, int right) {
    auto result = left + right;
    return result;
}

template<>
float AddOperator::Operation(int left, float right) {
    auto result = left + right;
    return result;
}

template<>
double AddOperator::Operation(int left, double right) {
    auto result = left + right;
    return true;
}

template<>
float AddOperator::Operation(float left, int right) {
    auto result = left + right;
    return result;
}

template<>
float AddOperator::Operation(float left, float right) {
    auto result = left + right;
    return result;
}

template<>
double AddOperator::Operation(float left, double right) {
    auto result = left + right;
    return result;
}

template<>
double AddOperator::Operation(double left, int right) {
    auto result = left + right;
    return result;
}

template<>
double AddOperator::Operation(double left, float right) {
    auto result = left + right;
    return result;
}

template<>
double AddOperator::Operation(double left, double right) {
    auto result = left + right;
    return result;
}

// -
template<>
int SubOperator::Operation(int left, int right) {
    auto result = left - right;
    return result;
}

template<>
float SubOperator::Operation(int left, float right) {
    auto result = left - right;
    return result;
}

template<>
double SubOperator::Operation(int left, double right) {
    auto result = left - right;
    return true;
}

template<>
float SubOperator::Operation(float left, int right) {
    auto result = left - right;
    return result;
}

template<>
float SubOperator::Operation(float left, float right) {
    auto result = left - right;
    return result;
}

template<>
double SubOperator::Operation(float left, double right) {
    auto result = left - right;
    return result;
}

template<>
double SubOperator::Operation(double left, int right) {
    auto result = left - right;
    return result;
}

template<>
double SubOperator::Operation(double left, float right) {
    auto result = left - right;
    return result;
}

template<>
double SubOperator::Operation(double left, double right) {
    auto result = left - right;
    return result;
}

// *
template<>
int MulOperator::Operation(int left, int right) {
    auto result = left * right;
    return result;
}

template<>
float MulOperator::Operation(int left, float right) {
    auto result = left * right;
    return result;
}

template<>
double MulOperator::Operation(int left, double right) {
    auto result = left * right;
    return true;
}

template<>
float MulOperator::Operation(float left, int right) {
    auto result = left * right;
    return result;
}

template<>
float MulOperator::Operation(float left, float right) {
    auto result = left * right;
    return result;
}

template<>
double MulOperator::Operation(float left, double right) {
    auto result = left * right;
    return result;
}

template<>
double MulOperator::Operation(double left, int right) {
    auto result = left * right;
    return result;
}

template<>
double MulOperator::Operation(double left, float right) {
    auto result = left * right;
    return result;
}

template<>
double MulOperator::Operation(double left, double right) {
    auto result = left * right;
    return result;
}

// ÷
// TODO: 没错除0的异常判断，这里需要做判断，后续优化
template<>
int DivOperator::Operation(int left, int right) {
    auto result = left / right;
    return result;
}

template<>
float DivOperator::Operation(int left, float right) {
    auto result = left / right;
    return result;
}

template<>
double DivOperator::Operation(int left, double right) {
    auto result = left / right;
    return true;
}

template<>
float DivOperator::Operation(float left, int right) {
    auto result = left / right;
    return result;
}

template<>
float DivOperator::Operation(float left, float right) {
    auto result = left / right;
    return result;
}

template<>
double DivOperator::Operation(float left, double right) {
    auto result = left / right;
    return result;
}

template<>
double DivOperator::Operation(double left, int right) {
    auto result = left / right;
    return result;
}

template<>
double DivOperator::Operation(double left, float right) {
    auto result = left / right;
    return result;
}

template<>
double DivOperator::Operation(double left, double right) {
    auto result = left / right;
    return result;
}

} // DaseX
