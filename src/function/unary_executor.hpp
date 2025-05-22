#pragma once

namespace DaseX {

// 执行任意的一元函数
struct UnaryLambdaWrapper {
    template <class FUNC, class INPUT_TYPE, class RESULT_TYPE>
    static inline RESULT_TYPE Operation(INPUT_TYPE input, void *funptr) {
        auto fun = (FUNC *)funptr;
        return (*fun)(input);
    }
}; // UnaryLambdaWrapper

class UnaryExecutor {
public:
    template <class INPUT_TYPE, class RESULT_TYPE, class FUNC = std::function<RESULT_TYPE(INPUT_TYPE)>>
    static void Execute(std::shared_ptr<arrow::Array> &input, std::vector<int> &result, int count, FUNC fun) {
        auto string_arr = std::static_pointer_cast<arrow::StringArray>(input);
        std::vector<int> current_sel;
        std::vector<int> res_sel;
        int res = 0;
        if(!result.empty()) {
            for (int i = 0; i < count; i++)
            {
                if (UnaryLambdaWrapper::template Operation<FUNC, INPUT_TYPE, RESULT_TYPE>(string_arr->GetString(i), (void *)&fun))
                {
                    current_sel.push_back(i);
                    res++;
                }
            }
            for (int j = 0; j < res; j++)
            {
                res_sel.push_back(result[current_sel[j]]);
            }
            result.swap(res_sel);
        } else {
            for (int i = 0; i < count; i++) {
                if(UnaryLambdaWrapper::template Operation<FUNC, INPUT_TYPE, RESULT_TYPE>(string_arr->GetString(i), (void *)&fun)) {
                    result.push_back(i);
                }
            }
        }
    }
}; // UnaryExecutor

} // DaseX
