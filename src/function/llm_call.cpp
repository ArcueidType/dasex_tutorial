//
// Created by zyy on 25-3-5.
//

#include "value_type.hpp"
#include "llm_call.hpp"
#include <string>
#include "bound_constant_expression.hpp"

namespace DaseX {
class StringValueInfo;


// TODO: 向 llm 服务发送请求，并且接受其返回值
std::shared_ptr<arrow::Array> llmfunc::call(std::shared_ptr<arrow::Array> &input) {
	auto string_arr = std::static_pointer_cast<arrow::StringArray>(input);
	int length = string_arr->length();
	std::vector<std::string> res(length);
	for(int i = 0; i < length; i++) {
		std::string old_val = string_arr->GetString(i);
		std::string new_val = prompt_ + old_val;
		res[i] = new_val;
	}
	std::shared_ptr<arrow::Array> result;
	Util::AppendStringArray(result, res);
	return result;
}

	std::shared_ptr<llmfunc> llmfunc::Createllmfunc(std::string prompt) {
	return std::make_shared<llmfunc>(prompt);
}

std::shared_ptr<FunctionData> llmfunc::Copy() const {
	return std::make_shared<llmfunc>(prompt_);
}

static std::shared_ptr<FunctionData> llmfuncBindFunction(ScalarProjectFunction &bound_function, std::vector<std::shared_ptr<Expression>> &arguments) {

	auto &expression_1 = (*(arguments[0])).Cast<BoundConstantExpression>();
	auto string_value_info = expression_1.value.GetValueUnsafe<std::string>();
	std::string prompt;
	if (!string_value_info.empty()) {
		prompt = string_value_info;
	}
	else {
		printf("llmfuncBindFunction string valueinfo trasform fail\n");
	}
	return llmfunc::Createllmfunc(prompt);
}

ScalarProjectFunction LLMCallFun::GetLLMCallFunction() {
	return ScalarProjectFunction("llm call",
								 {LogicalType::STRING, LogicalType::STRING},
								 LogicalType::STRING,
								 RegularLLMFunction,
								 llmfuncBindFunction);
}

} // namespace DaseX