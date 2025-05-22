#pragma once

#include "arrow_help.hpp"
#include <vector>
#include <memory>
#include <arrow/api.h>

namespace DaseX {

struct BinaryExecutor {
    template <class LEFT_TYPE, class RIGHT_TYPE, class RESULT_TYPE, class OP>
    static typename std::enable_if<std::is_same<RESULT_TYPE, int>::value, void>::type
    Execute(std::shared_ptr<arrow::Array> &left, std::shared_ptr<arrow::Array> &right, std::shared_ptr<arrow::Array> &result, int count) {
        auto left_arr = std::static_pointer_cast<arrow::Int32Array>(left);
        auto right_arr = std::static_pointer_cast<arrow::Int32Array>(right);
        std::vector<int> res;
        for(int i = 0; i < count; i++) {
            int res_p = OP::template Operation<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE>(left_arr->Value(i), right_arr->Value(i));
            res.push_back(res_p);
        }
        Util::AppendIntArray(result, res);
    } // Execute

    template <class LEFT_TYPE, class RIGHT_TYPE, class RESULT_TYPE, class OP>
    static typename std::enable_if<std::is_same<RESULT_TYPE, float>::value, void>::type
    Execute(std::shared_ptr<arrow::Array> &left, std::shared_ptr<arrow::Array> &right, std::shared_ptr<arrow::Array> &result, int count) {
        auto left_arr = std::static_pointer_cast<arrow::FloatArray>(left);
        switch (right->type_id()) {
            case arrow::Type::INT32:
            {
                auto right_arr = std::static_pointer_cast<arrow::Int32Array>(right);
                std::vector<float> res;
                for(int i = 0; i < count; i++) {
                    float res_p = OP::template Operation<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE>(left_arr->Value(i), right_arr->Value(i));
                    res.push_back(res_p);
                }
                Util::AppendFloatArray(result, res);
                break;
            }
            case arrow::Type::FLOAT:
            {
                auto right_arr = std::static_pointer_cast<arrow::FloatArray>(right);
                std::vector<float> res;
                for(int i = 0; i < count; i++) {
                    float res_p = OP::template Operation<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE>(left_arr->Value(i), right_arr->Value(i));
                    res.push_back(res_p);
                }
                Util::AppendFloatArray(result, res);
                break;
            }
            default:
                throw std::runtime_error("Invalid arrow::Type!!!");
        } // switch
    } // Execute

    template <class LEFT_TYPE, class RIGHT_TYPE, class RESULT_TYPE, class OP>
    static typename std::enable_if<std::is_same<RESULT_TYPE, double>::value, void>::type
    Execute(std::shared_ptr<arrow::Array> &left, std::shared_ptr<arrow::Array> &right, std::shared_ptr<arrow::Array> &result, int count) {
        auto left_arr = std::static_pointer_cast<arrow::DoubleArray>(left);
        auto right_arr = std::static_pointer_cast<arrow::DoubleArray>(right);
        std::vector<double> res;
        for(int i = 0; i < count; i++) {
            double res_p = OP::template Operation<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE>(left_arr->Value(i), right_arr->Value(i));
            res.push_back(res_p);
        }
        Util::AppendDoubleArray(result, res);
    } // Execute


}; // BinaryExecutor

} // DaseX
