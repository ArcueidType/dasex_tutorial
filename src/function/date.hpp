//
// Created by cwl on 7/29/24.
//

#pragma once

#include "expression_executor_state.hpp"
#include "bound_project_function_expression.hpp"
#include "arrow_help.hpp"
#include <arrow/api.h>
#include <vector>
#include <ctime>
#include <memory>

namespace DaseX {

static void RegularExtractYearFunction(std::vector<std::shared_ptr<arrow::Array>> &input, ExpressionState &state, std::shared_ptr<arrow::Array> &result) {
    auto &left = input[0];
    auto left_val = std::static_pointer_cast<arrow::Int32Array>(left);
    int length = left_val->length();
    std::vector<int> res(length);
    for(int i = 0; i < length; i++) {
        std::time_t timestamp = left_val->Value(i);
        std::tm *timeinfo = std::localtime(&timestamp);
        int year = 1900 + timeinfo->tm_year;
        res[i] = year;
    }
    Util::AppendIntArray(result, res);
} // RegularNumericAddFunction

inline scalar_function_p GetExtractYearFunction() {
    scalar_function_p function = &RegularExtractYearFunction;
    return function;
}

} // DaseX
