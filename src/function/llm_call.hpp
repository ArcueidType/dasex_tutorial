//
// Created by zyy on 25-3-5.
//
#pragma once

#include "expression_executor_state.hpp"
#include "bound_function_expression.hpp"
#include "unary_executor.hpp"
#include "scalar_function.hpp"
#include "typedefs.hpp"
#include <string>
#include <sstream>
#include "bound_constant_expression.hpp"
#include "bound_project_function_expression.hpp"
#include "binary_executor.hpp"
#include "value_operations.hpp"
#include "value_type.hpp"

namespace DaseX {

class llmfunc : public FunctionData {
   public:
	std::string prompt_;
   public:
	llmfunc(std::string prompt) : prompt_(prompt) {}
   public:
	std::shared_ptr<arrow::Array> call(std::shared_ptr<arrow::Array> &input);
	static std::shared_ptr<llmfunc> Createllmfunc(std::string prompt);
	std::shared_ptr<FunctionData> Copy() const override;
};

static void RegularLLMFunction(std::vector<std::shared_ptr<arrow::Array>> &input,
							   ExpressionState &state,
							   std::shared_ptr<arrow::Array> &result) {
	auto &func_expr = (*(state.expr)).Cast<BoundProjectFunctionExpression>();
	auto &operator_llmfunc = (*(func_expr.bind_info)).Cast<llmfunc>();
	result = operator_llmfunc.call(input[0]);

}


struct LLMCallFun {
	static ScalarProjectFunction GetLLMCallFunction();
};


}