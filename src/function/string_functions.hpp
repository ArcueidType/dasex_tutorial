#pragma once

#include "scalar_function.hpp"
#include "typedefs.hpp"
#include <string>
#include "bound_constant_expression.hpp"
#include "bound_project_function_expression.hpp"

namespace DaseX {

struct LikeFun {
    static ScalarFunction GetLikeFunction();
}; // LikeFun

struct NotLikeFun {
    static ScalarFunction GetNotLikeFunction();
}; // LikeFun

struct ContainsFun {
    static idx_t Find(const unsigned char *haystack, idx_t haystack_size, const unsigned char *needle, idx_t needle_size);
};



//===================================================sub_string===================================================
class SubStringFilter : public FunctionData {
public:
    int start_pos;
    int len;
public:
    SubStringFilter(int start_pos, int len) : start_pos(start_pos), len(len) {}
public:
    std::shared_ptr<arrow::Array> SubStr(std::shared_ptr<arrow::Array> &input);
    static std::shared_ptr<SubStringFilter> CreateSubStringFilter(int start_pos, int len);
    std::shared_ptr<FunctionData> Copy() const override;
}; // SubStringFilter

static void RegularSubStringFunction(std::vector<std::shared_ptr<arrow::Array>> &input, ExpressionState &state, std::shared_ptr<arrow::Array> &result) {
    auto &func_expr = (*(state.expr)).Cast<BoundProjectFunctionExpression>();
    auto &operator_substr = (*(func_expr.bind_info)).Cast<SubStringFilter>();
    result = operator_substr.SubStr(input[0]);
} // RegularSubStringFunction

struct SubStringFun {
    static ScalarProjectFunction GetSubStringFunction();
}; // SubStringFun

} // DaseX
