#include "string_functions.hpp"
#include "logical_type.hpp"
#include "like.hpp"
#include "arrow_help.hpp"


namespace DaseX {

//===================================================sub_string===================================================
std::shared_ptr<arrow::Array> SubStringFilter::SubStr(std::shared_ptr<arrow::Array> &input) {
        auto string_arr = std::static_pointer_cast<arrow::StringArray>(input);
        int length = string_arr->length();
        std::vector<std::string> res(length);
        for(int i = 0; i < length; i++) {
            std::string old_val = string_arr->GetString(i);
            std::string new_val = old_val.substr(start_pos, len);
            res[i] = new_val;
        }
        std::shared_ptr<arrow::Array> result;
        Util::AppendStringArray(result, res);
        return result;
}

std::shared_ptr<SubStringFilter> SubStringFilter::CreateSubStringFilter(int start_pos, int len) {
    return std::make_shared<SubStringFilter>(start_pos, len);
}

std::shared_ptr<FunctionData> SubStringFilter::Copy() const {
    return std::make_shared<SubStringFilter>(start_pos, len);
}

// 绑定函数参数
static std::shared_ptr<FunctionData> SubStringBindFunction(ScalarProjectFunction &bound_function, std::vector<std::shared_ptr<Expression>> &arguments) {
    // TODO:这里需要判断表达式类型，以及参数，不然容易出错，后续优化
    auto &expression_1 = (*(arguments[0])).Cast<BoundConstantExpression>();
    auto &expression_2 = (*(arguments[1])).Cast<BoundConstantExpression>();
    Value start_pos = expression_1.value;
    Value len = expression_2.value;
    return SubStringFilter::CreateSubStringFilter(start_pos.GetValueUnsafe<int>(), len.GetValueUnsafe<int>());
}

ScalarProjectFunction SubStringFun::GetSubStringFunction() {
    return ScalarProjectFunction("sub_string",
                                 {LogicalType::STRING, LogicalType::STRING},
                                 LogicalType::STRING,
                                 RegularSubStringFunction,
                                 SubStringBindFunction);
}

} // DaseX
