#include "vector_operations.hpp"
#include <spdlog/spdlog.h>
#include <cassert>
#include <cmath>

namespace {

double RoundToDecimalPlaces(double value, int precision) {
    double factor = std::pow(10.0, precision);
    return std::round(value * factor) / factor;
}

float RoundToDecimalPlaces(float value, int precision) {
    float factor = std::pow(10.0F, precision);
    return std::round(value * factor) / factor;
}


bool AreAlmostEqual(double a, double b, int precision) {
    // 计算容忍度
    double epsilon = std::pow(10.0f, -precision);

    // 四舍五入到指定精度
    double roundedA = std::round(a * std::pow(10.0, precision)) / std::pow(10.0, precision);
    double roundedB = std::round(b * std::pow(10.0, precision)) / std::pow(10.0, precision);

    // 比较两数的差值是否在容忍度范围内
    return std::fabs(roundedA - roundedB) < epsilon;
}

}

namespace DaseX
{

    int VectorOperations::Equals(std::shared_ptr<arrow::Array> &left, std::shared_ptr<arrow::Array> &right, int count, std::vector<int> &sel)
    {
        assert(left->length() == count);
        assert(right->length() == count);
        int res = 0;
        std::vector<int> current_sel;
        std::vector<int> res_sel;
        switch (left->type_id())
        {
        case arrow::Type::INT32:
        {
            auto left_array = std::static_pointer_cast<arrow::Int32Array>(left);
            switch (right->type_id()) {
                case arrow::Type::INT32:
                {
                    auto right_array = std::static_pointer_cast<arrow::Int32Array>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            // TODO: 这里只能暂时解决Q2中浮点数比较时的精度问题，后续还是要支持DECIMAL类型
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            int left_p = left_array->Value(i);
                            int right_p = right_array->Value(i);
                            if (left_p == right_p)
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            int left_p = left_array->Value(i);
                            int right_p = right_array->Value(i);
                            if (left_p == right_p)
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                case arrow::Type::FLOAT:
                {
                    auto right_array = std::static_pointer_cast<arrow::FloatArray>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            int left_p = left_array->Value(i);
                            float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (left_p == right_p)
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            int left_p = left_array->Value(i);
                            float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (left_p == right_p)
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                case arrow::Type::DOUBLE:
                {
                    auto right_array = std::static_pointer_cast<arrow::DoubleArray>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            int left_p = left_array->Value(i);
                            double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (left_p == right_p)
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            int left_p = left_array->Value(i);
                            double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (left_p == right_p)
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                default:
                    throw std::runtime_error("Invalid arrow::Type!!!");
            } // switch
            break;
        }
        case arrow::Type::FLOAT:
        {
            auto left_array = std::static_pointer_cast<arrow::FloatArray>(left);
            switch (right->type_id()) {
                case arrow::Type::INT32:
                {
                    auto right_array = std::static_pointer_cast<arrow::Int32Array>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            // TODO: 这里只能暂时解决Q2中浮点数比较时的精度问题，后续还是要支持DECIMAL类型
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            int right_p = right_array->Value(i);
                            if (left_p == right_p)
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            int right_p = right_array->Value(i);
                            if (left_p == right_p)
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                case arrow::Type::FLOAT:
                {
                    auto right_array = std::static_pointer_cast<arrow::FloatArray>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (AreAlmostEqual(left_p ,right_p, 2))
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (AreAlmostEqual(left_p, right_p, 2))
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                case arrow::Type::DOUBLE:
                {
                    auto right_array = std::static_pointer_cast<arrow::DoubleArray>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (AreAlmostEqual(left_p, right_p, 2))
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (AreAlmostEqual(left_p, right_p, 2))
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                default:
                    throw std::runtime_error("Invalid arrow::Type!!!");
            } // switch
            break;
        }
        case arrow::Type::DOUBLE:
        {
            auto left_array = std::static_pointer_cast<arrow::DoubleArray>(left);
            switch (right->type_id()) {
                case arrow::Type::INT32:
                {
                    auto right_array = std::static_pointer_cast<arrow::Int32Array>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            // TODO: 这里只能暂时解决Q2中浮点数比较时的精度问题，后续还是要支持DECIMAL类型
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            int right_p = right_array->Value(i);
                            if (left_p == right_p)
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            int right_p = right_array->Value(i);
                            if (left_p == right_p)
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                case arrow::Type::FLOAT:
                {
                    auto right_array = std::static_pointer_cast<arrow::FloatArray>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (AreAlmostEqual(left_p, right_p, 2))
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (AreAlmostEqual(left_p, right_p, 2))
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                case arrow::Type::DOUBLE:
                {
                    auto right_array = std::static_pointer_cast<arrow::DoubleArray>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (AreAlmostEqual(left_p, right_p, 2))
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (AreAlmostEqual(left_p, right_p, 2))
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                default:
                    throw std::runtime_error("Invalid arrow::Type!!!");
            } // switch
            break;
        }
        case arrow::Type::STRING:
        {
            auto left_array = std::static_pointer_cast<arrow::StringArray>(left);
            auto right_array = std::static_pointer_cast<arrow::StringArray>(right);
            if (!sel.empty())
            {
                for (int i = 0; i < count; i++)
                {
                    if(left_array->IsNull(i) || right_array->IsNull(i)) {
                        continue;
                    }
                    if (left_array->GetString(i) == right_array->GetString(i))
                    {
                        current_sel.push_back(i);
                        res++;
                    }
                }
                if (res == 0) {
                    sel.clear();
                    break;
                }
                for (int j = 0; j < res; j++)
                {
                    res_sel.push_back(sel[current_sel[j]]);
                }
                sel.swap(res_sel);
            }
            else
            {
                for (int i = 0; i < count; i++)
                {
                    if(left_array->IsNull(i) || right_array->IsNull(i)) {
                        continue;
                    }
                    if (left_array->GetString(i) == right_array->GetString(i))
                    {
                        sel.push_back(i);
                        res++;
                    }
                }
            }
            break;
        }
        default:
            throw std::runtime_error("Invalid arrow::Type!!!");
        } // switch

        return res;
    }

    // A != B
    int VectorOperations::NotEquals(std::shared_ptr<arrow::Array> &left, std::shared_ptr<arrow::Array> &right, int count, std::vector<int> &sel)
    {
        assert(left->length() == count);
        assert(right->length() == count);
        int res = 0;
        std::vector<int> current_sel;
        std::vector<int> res_sel;
        switch (left->type_id())
        {
        case arrow::Type::INT32:
        {
            auto left_array = std::static_pointer_cast<arrow::Int32Array>(left);
            switch (right->type_id()) {
                case arrow::Type::INT32:
                {
                    auto right_array = std::static_pointer_cast<arrow::Int32Array>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            // TODO: 这里只能暂时解决Q2中浮点数比较时的精度问题，后续还是要支持DECIMAL类型
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            int left_p = left_array->Value(i);
                            int right_p = right_array->Value(i);
                            if (left_p != right_p)
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            int left_p = left_array->Value(i);
                            int right_p = right_array->Value(i);
                            if (left_p != right_p)
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                case arrow::Type::FLOAT:
                {
                    auto right_array = std::static_pointer_cast<arrow::FloatArray>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            int left_p = left_array->Value(i);
                            float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (left_p != right_p)
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            int left_p = left_array->Value(i);
                            float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (left_p != right_p)
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                case arrow::Type::DOUBLE:
                {
                    auto right_array = std::static_pointer_cast<arrow::DoubleArray>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            int left_p = left_array->Value(i);
                            double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (left_p != right_p)
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            int left_p = left_array->Value(i);
                            double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (left_p != right_p)
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                default:
                    throw std::runtime_error("Invalid arrow::Type!!!");
            } // switch
            break;
        }
        case arrow::Type::FLOAT:
        {
            auto left_array = std::static_pointer_cast<arrow::FloatArray>(left);
            switch (right->type_id()) {
                case arrow::Type::INT32:
                {
                    auto right_array = std::static_pointer_cast<arrow::Int32Array>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            // TODO: 这里只能暂时解决Q2中浮点数比较时的精度问题，后续还是要支持DECIMAL类型
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            int right_p = right_array->Value(i);
                            if (left_p != right_p)
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            int right_p = right_array->Value(i);
                            if (left_p != right_p)
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                case arrow::Type::FLOAT:
                {
                    auto right_array = std::static_pointer_cast<arrow::FloatArray>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (!AreAlmostEqual(left_p, right_p, 2))
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (!AreAlmostEqual(left_p, right_p, 2))
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                case arrow::Type::DOUBLE:
                {
                    auto right_array = std::static_pointer_cast<arrow::DoubleArray>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (!AreAlmostEqual(left_p, right_p, 2))
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (!AreAlmostEqual(left_p, right_p, 2))
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                default:
                    throw std::runtime_error("Invalid arrow::Type!!!");
            } // switch
            break;
        }
        case arrow::Type::DOUBLE:
        {
            auto left_array = std::static_pointer_cast<arrow::DoubleArray>(left);
            switch (right->type_id()) {
                case arrow::Type::INT32:
                {
                    auto right_array = std::static_pointer_cast<arrow::Int32Array>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            // TODO: 这里只能暂时解决Q2中浮点数比较时的精度问题，后续还是要支持DECIMAL类型
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            int right_p = right_array->Value(i);
                            if (left_p != right_p)
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            int right_p = right_array->Value(i);
                            if (left_p != right_p)
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                case arrow::Type::FLOAT:
                {
                    auto right_array = std::static_pointer_cast<arrow::FloatArray>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (!AreAlmostEqual(left_p, right_p, 2))
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (!AreAlmostEqual(left_p, right_p, 2))
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                case arrow::Type::DOUBLE:
                {
                    auto right_array = std::static_pointer_cast<arrow::DoubleArray>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (!AreAlmostEqual(left_p, right_p, 2))
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (!AreAlmostEqual(left_p, right_p, 2))
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                default:
                    throw std::runtime_error("Invalid arrow::Type!!!");
            } // switch
            break;
        }
        case arrow::Type::STRING:
        {
            auto left_array = std::static_pointer_cast<arrow::StringArray>(left);
            auto right_array = std::static_pointer_cast<arrow::StringArray>(right);
            if (!sel.empty())
            {
                for (int i = 0; i < count; i++)
                {
                    if(left_array->IsNull(i) || right_array->IsNull(i)) {
                        continue;
                    }
                    if (left_array->GetString(i) != right_array->GetString(i))
                    {
                        current_sel.push_back(i);
                        res++;
                    }
                }
                if (res == 0) {
                    sel.clear();
                    break;
                }
                for (int j = 0; j < res; j++)
                {
                    res_sel.push_back(sel[current_sel[j]]);
                }
                sel.swap(res_sel);
            }
            else
            {
                for (int i = 0; i < count; i++)
                {
                    if(left_array->IsNull(i) || right_array->IsNull(i)) {
                        continue;
                    }
                    if (left_array->GetString(i) != right_array->GetString(i))
                    {
                        sel.push_back(i);
                        res++;
                    }
                }
            }
            break;
        }
        default:
            throw std::runtime_error("Invalid arrow::Type!!!");
        } // switch
        return res;
    }

    // A > B
    int VectorOperations::GreaterThan(std::shared_ptr<arrow::Array> &left, std::shared_ptr<arrow::Array> &right, int count, std::vector<int> &sel)
    {
        assert(left->length() == count);
        assert(right->length() == count);
        int res = 0;
        std::vector<int> current_sel;
        std::vector<int> res_sel;
        switch (left->type_id())
        {
        case arrow::Type::INT32:
        {
            auto left_array = std::static_pointer_cast<arrow::Int32Array>(left);
            switch (right->type_id()) {
                case arrow::Type::INT32:
                {
                    auto right_array = std::static_pointer_cast<arrow::Int32Array>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            // TODO: 这里只能暂时解决Q2中浮点数比较时的精度问题，后续还是要支持DECIMAL类型
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            int left_p = left_array->Value(i);
                            int right_p = right_array->Value(i);
                            if (left_p > right_p)
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            int left_p = left_array->Value(i);
                            int right_p = right_array->Value(i);
                            if (left_p > right_p)
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                case arrow::Type::FLOAT:
                {
                    auto right_array = std::static_pointer_cast<arrow::FloatArray>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            int left_p = left_array->Value(i);
                            float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (left_p > right_p)
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            int left_p = left_array->Value(i);
                            float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (left_p > right_p)
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                case arrow::Type::DOUBLE:
                {
                    auto right_array = std::static_pointer_cast<arrow::DoubleArray>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            int left_p = left_array->Value(i);
                            double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (left_p > right_p)
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            int left_p = left_array->Value(i);
                            double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (left_p > right_p)
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                default:
                    throw std::runtime_error("Invalid arrow::Type!!!");
            } // switch
            break;
        }
        case arrow::Type::FLOAT:
        {
            auto left_array = std::static_pointer_cast<arrow::FloatArray>(left);
            switch (right->type_id()) {
                case arrow::Type::INT32:
                {
                    auto right_array = std::static_pointer_cast<arrow::Int32Array>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            // TODO: 这里只能暂时解决Q2中浮点数比较时的精度问题，后续还是要支持DECIMAL类型
                            float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            int right_p = right_array->Value(i);
                            if (left_p > right_p)
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            int right_p = right_array->Value(i);
                            if (left_p > right_p)
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                case arrow::Type::FLOAT:
                {
                    auto right_array = std::static_pointer_cast<arrow::FloatArray>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (left_p > right_p)
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (left_p > right_p)
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                case arrow::Type::DOUBLE:
                {
                    auto right_array = std::static_pointer_cast<arrow::DoubleArray>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (left_p > right_p)
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (left_p > right_p)
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                default:
                    throw std::runtime_error("Invalid arrow::Type!!!");
            } // switch
            break;
        }
        case arrow::Type::DOUBLE:
        {
            auto left_array = std::static_pointer_cast<arrow::DoubleArray>(left);
            switch (right->type_id()) {
                case arrow::Type::INT32:
                {
                    auto right_array = std::static_pointer_cast<arrow::Int32Array>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            // TODO: 这里只能暂时解决Q2中浮点数比较时的精度问题，后续还是要支持DECIMAL类型
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            int right_p = right_array->Value(i);
                            if (left_p > right_p)
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            int right_p = right_array->Value(i);
                            if (left_p > right_p)
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                case arrow::Type::FLOAT:
                {
                    auto right_array = std::static_pointer_cast<arrow::FloatArray>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (left_p > right_p)
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (left_p > right_p)
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                case arrow::Type::DOUBLE:
                {
                    auto right_array = std::static_pointer_cast<arrow::DoubleArray>(right);
                    if (!sel.empty())
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (left_p > right_p)
                            {
                                current_sel.push_back(i);
                                res++;
                            }
                        }
                        if (res == 0) {
                            sel.clear();
                            break;
                        }
                        for (int j = 0; j < res; j++)
                        {
                            res_sel.push_back(sel[current_sel[j]]);
                        }
                        sel.swap(res_sel);
                    }
                    else
                    {
                        for (int i = 0; i < count; i++)
                        {
                            if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                continue;
                            }
                            double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                            double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                            if (left_p > right_p)
                            {
                                sel.push_back(i);
                                res++;
                            }
                        }
                    } // else
                    break;
                }
                default:
                    throw std::runtime_error("Invalid arrow::Type!!!");
            } // switch
            break;
        }
        case arrow::Type::STRING:
        {
            auto left_array = std::static_pointer_cast<arrow::StringArray>(left);
            auto right_array = std::static_pointer_cast<arrow::StringArray>(right);
            if (!sel.empty())
            {
                for (int i = 0; i < count; i++)
                {
                    if(left_array->IsNull(i) || right_array->IsNull(i)) {
                        continue;
                    }
                    if (left_array->GetString(i) > right_array->GetString(i))
                    {
                        current_sel.push_back(i);
                        res++;
                    }
                }
                if (res == 0) {
                    sel.clear();
                    break;
                }
                for (int j = 0; j < res; j++)
                {
                    res_sel.push_back(sel[current_sel[j]]);
                }
                sel.swap(res_sel);
            }
            else
            {
                for (int i = 0; i < count; i++)
                {
                    if(left_array->IsNull(i) || right_array->IsNull(i)) {
                        continue;
                    }
                    if (left_array->GetString(i) > right_array->GetString(i))
                    {
                        sel.push_back(i);
                        res++;
                    }
                }
            }
            break;
        }
        default:
            throw std::runtime_error("Invalid arrow::Type!!!");
        } // switch
        return res;
    }

    // A >= B
    int VectorOperations::GreaterThanEquals(std::shared_ptr<arrow::Array> &left, std::shared_ptr<arrow::Array> &right, int count, std::vector<int> &sel)
    {
        assert(left->length() == count);
        assert(right->length() == count);
        int res = 0;
        std::vector<int> current_sel;
        std::vector<int> res_sel;
        switch (left->type_id())
        {
            case arrow::Type::INT32:
            {
                auto left_array = std::static_pointer_cast<arrow::Int32Array>(left);
                switch (right->type_id()) {
                    case arrow::Type::INT32:
                    {
                        auto right_array = std::static_pointer_cast<arrow::Int32Array>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                // TODO: 这里只能暂时解决Q2中浮点数比较时的精度问题，后续还是要支持DECIMAL类型
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                int left_p = left_array->Value(i);
                                int right_p = right_array->Value(i);
                                if (left_p > right_p || left_p == right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                int left_p = left_array->Value(i);
                                int right_p = right_array->Value(i);
                                if (left_p > right_p || left_p == right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    case arrow::Type::FLOAT:
                    {
                        auto right_array = std::static_pointer_cast<arrow::FloatArray>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                int left_p = left_array->Value(i);
                                float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p > right_p || left_p == right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                int left_p = left_array->Value(i);
                                float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p > right_p || left_p == right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    case arrow::Type::DOUBLE:
                    {
                        auto right_array = std::static_pointer_cast<arrow::DoubleArray>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                int left_p = left_array->Value(i);
                                double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p > right_p || left_p == right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                int left_p = left_array->Value(i);
                                double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p > right_p || left_p == right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    default:
                        throw std::runtime_error("Invalid arrow::Type!!!");
                } // switch
                break;
            }
            case arrow::Type::FLOAT:
            {
                auto left_array = std::static_pointer_cast<arrow::FloatArray>(left);
                switch (right->type_id()) {
                    case arrow::Type::INT32:
                    {
                        auto right_array = std::static_pointer_cast<arrow::Int32Array>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                // TODO: 这里只能暂时解决Q2中浮点数比较时的精度问题，后续还是要支持DECIMAL类型
                                float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                int right_p = right_array->Value(i);
                                if (left_p > right_p || left_p == right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                int right_p = right_array->Value(i);
                                if (left_p > right_p || left_p == right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    case arrow::Type::FLOAT:
                    {
                        auto right_array = std::static_pointer_cast<arrow::FloatArray>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p > right_p || left_p == right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p > right_p || left_p == right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    case arrow::Type::DOUBLE:
                    {
                        auto right_array = std::static_pointer_cast<arrow::DoubleArray>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p > right_p || left_p == right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p > right_p || left_p == right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    default:
                        throw std::runtime_error("Invalid arrow::Type!!!");
                } // switch
                break;
            }
            case arrow::Type::DOUBLE:
            {
                auto left_array = std::static_pointer_cast<arrow::DoubleArray>(left);
                switch (right->type_id()) {
                    case arrow::Type::INT32:
                    {
                        auto right_array = std::static_pointer_cast<arrow::Int32Array>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                // TODO: 这里只能暂时解决Q2中浮点数比较时的精度问题，后续还是要支持DECIMAL类型
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                int right_p = right_array->Value(i);
                                if (left_p > right_p || left_p == right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                int right_p = right_array->Value(i);
                                if (left_p > right_p || left_p == right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    case arrow::Type::FLOAT:
                    {
                        auto right_array = std::static_pointer_cast<arrow::FloatArray>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p > right_p || left_p == right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p > right_p || left_p == right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    case arrow::Type::DOUBLE:
                    {
                        auto right_array = std::static_pointer_cast<arrow::DoubleArray>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p > right_p || left_p == right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p > right_p || left_p == right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    default:
                        throw std::runtime_error("Invalid arrow::Type!!!");
                } // switch
                break;
            }
            case arrow::Type::STRING:
            {
                auto left_array = std::static_pointer_cast<arrow::StringArray>(left);
                auto right_array = std::static_pointer_cast<arrow::StringArray>(right);
                if (!sel.empty())
                {
                    for (int i = 0; i < count; i++)
                    {
                        if(left_array->IsNull(i) || right_array->IsNull(i)) {
                            continue;
                        }
                        if (left_array->GetString(i) >= right_array->GetString(i))
                        {
                            current_sel.push_back(i);
                            res++;
                        }
                    }
                    if (res == 0) {
                        sel.clear();
                        break;
                    }
                    for (int j = 0; j < res; j++)
                    {
                        res_sel.push_back(sel[current_sel[j]]);
                    }
                    sel.swap(res_sel);
                }
                else
                {
                    for (int i = 0; i < count; i++)
                    {
                        if(left_array->IsNull(i) || right_array->IsNull(i)) {
                            continue;
                        }
                        if (left_array->GetString(i) >= right_array->GetString(i))
                        {
                            sel.push_back(i);
                            res++;
                        }
                    }
                }
                break;
            }
            default:
                throw std::runtime_error("Invalid arrow::Type!!!");
        } // switch
        return res;
    }

    // A < B
    int VectorOperations::LessThan(std::shared_ptr<arrow::Array> &left, std::shared_ptr<arrow::Array> &right, int count, std::vector<int> &sel)
    {
        assert(left->length() == count);
        assert(right->length() == count);
        int res = 0;
        std::vector<int> current_sel;
        std::vector<int> res_sel;
        switch (left->type_id())
        {
            case arrow::Type::INT32:
            {
                auto left_array = std::static_pointer_cast<arrow::Int32Array>(left);
                switch (right->type_id()) {
                    case arrow::Type::INT32:
                    {
                        auto right_array = std::static_pointer_cast<arrow::Int32Array>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                // TODO: 这里只能暂时解决Q2中浮点数比较时的精度问题，后续还是要支持DECIMAL类型
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                int left_p = left_array->Value(i);
                                int right_p = right_array->Value(i);
                                if (left_p < right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                int left_p = left_array->Value(i);
                                int right_p = right_array->Value(i);
                                if (left_p < right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    case arrow::Type::FLOAT:
                    {
                        auto right_array = std::static_pointer_cast<arrow::FloatArray>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                int left_p = left_array->Value(i);
                                float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p < right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                int left_p = left_array->Value(i);
                                float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p < right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    case arrow::Type::DOUBLE:
                    {
                        auto right_array = std::static_pointer_cast<arrow::DoubleArray>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                int left_p = left_array->Value(i);
                                double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p < right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                int left_p = left_array->Value(i);
                                double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p < right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    default:
                        throw std::runtime_error("Invalid arrow::Type!!!");
                } // switch
                break;
            }
            case arrow::Type::FLOAT:
            {
                auto left_array = std::static_pointer_cast<arrow::FloatArray>(left);
                switch (right->type_id()) {
                    case arrow::Type::INT32:
                    {
                        auto right_array = std::static_pointer_cast<arrow::Int32Array>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                // TODO: 这里只能暂时解决Q2中浮点数比较时的精度问题，后续还是要支持DECIMAL类型
                                float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                int right_p = right_array->Value(i);
                                if (left_p < right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                int right_p = right_array->Value(i);
                                if (left_p < right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    case arrow::Type::FLOAT:
                    {
                        auto right_array = std::static_pointer_cast<arrow::FloatArray>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p < right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p < right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    case arrow::Type::DOUBLE:
                    {
                        auto right_array = std::static_pointer_cast<arrow::DoubleArray>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p < right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p < right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    default:
                        throw std::runtime_error("Invalid arrow::Type!!!");
                } // switch
                break;
            }
            case arrow::Type::DOUBLE:
            {
                auto left_array = std::static_pointer_cast<arrow::DoubleArray>(left);
                switch (right->type_id()) {
                    case arrow::Type::INT32:
                    {
                        auto right_array = std::static_pointer_cast<arrow::Int32Array>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                // TODO: 这里只能暂时解决Q2中浮点数比较时的精度问题，后续还是要支持DECIMAL类型
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                int right_p = right_array->Value(i);
                                if (left_p < right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                int right_p = right_array->Value(i);
                                if (left_p < right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    case arrow::Type::FLOAT:
                    {
                        auto right_array = std::static_pointer_cast<arrow::FloatArray>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p < right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p < right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    case arrow::Type::DOUBLE:
                    {
                        auto right_array = std::static_pointer_cast<arrow::DoubleArray>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p < right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p < right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    default:
                        throw std::runtime_error("Invalid arrow::Type!!!");
                } // switch
                break;
            }
            case arrow::Type::STRING:
            {
                auto left_array = std::static_pointer_cast<arrow::StringArray>(left);
                auto right_array = std::static_pointer_cast<arrow::StringArray>(right);
                if (!sel.empty())
                {
                    for (int i = 0; i < count; i++)
                    {
                        if(left_array->IsNull(i) || right_array->IsNull(i)) {
                            continue;
                        }
                        if (left_array->GetString(i) < right_array->GetString(i))
                        {
                            current_sel.push_back(i);
                            res++;
                        }
                    }
                    if (res == 0) {
                        sel.clear();
                        break;
                    }
                    for (int j = 0; j < res; j++)
                    {
                        res_sel.push_back(sel[current_sel[j]]);
                    }
                    sel.swap(res_sel);
                }
                else
                {
                    for (int i = 0; i < count; i++)
                    {
                        if(left_array->IsNull(i) || right_array->IsNull(i)) {
                            continue;
                        }
                        if (left_array->GetString(i) < right_array->GetString(i))
                        {
                            sel.push_back(i);
                            res++;
                        }
                    }
                }
                break;
            }
            default:
                throw std::runtime_error("Invalid arrow::Type!!!");
        } // switch
        return res;
    }

    // A <= B
    int VectorOperations::LessThanEquals(std::shared_ptr<arrow::Array> &left, std::shared_ptr<arrow::Array> &right, int count, std::vector<int> &sel)
    {
        assert(left->length() == count);
        assert(right->length() == count);
        int res = 0;
        std::vector<int> current_sel;
        std::vector<int> res_sel;
        switch (left->type_id())
        {
            case arrow::Type::INT32:
            {
                auto left_array = std::static_pointer_cast<arrow::Int32Array>(left);
                switch (right->type_id()) {
                    case arrow::Type::INT32:
                    {
                        auto right_array = std::static_pointer_cast<arrow::Int32Array>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                // TODO: 这里只能暂时解决Q2中浮点数比较时的精度问题，后续还是要支持DECIMAL类型
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                int left_p = left_array->Value(i);
                                int right_p = right_array->Value(i);
                                if (left_p <= right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                int left_p = left_array->Value(i);
                                int right_p = right_array->Value(i);
                                if (left_p <= right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    case arrow::Type::FLOAT:
                    {
                        auto right_array = std::static_pointer_cast<arrow::FloatArray>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                int left_p = left_array->Value(i);
                                float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p <= right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                int left_p = left_array->Value(i);
                                float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p <= right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    case arrow::Type::DOUBLE:
                    {
                        auto right_array = std::static_pointer_cast<arrow::DoubleArray>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                int left_p = left_array->Value(i);
                                double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p <= right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                int left_p = left_array->Value(i);
                                double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p <= right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    default:
                        throw std::runtime_error("Invalid arrow::Type!!!");
                } // switch
                break;
            }
            case arrow::Type::FLOAT:
            {
                auto left_array = std::static_pointer_cast<arrow::FloatArray>(left);
                switch (right->type_id()) {
                    case arrow::Type::INT32:
                    {
                        auto right_array = std::static_pointer_cast<arrow::Int32Array>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                // TODO: 这里只能暂时解决Q2中浮点数比较时的精度问题，后续还是要支持DECIMAL类型
                                float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                int right_p = right_array->Value(i);
                                if (left_p <= right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                int right_p = right_array->Value(i);
                                if (left_p <= right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    case arrow::Type::FLOAT:
                    {
                        auto right_array = std::static_pointer_cast<arrow::FloatArray>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p <= right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p <= right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    case arrow::Type::DOUBLE:
                    {
                        auto right_array = std::static_pointer_cast<arrow::DoubleArray>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p <= right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                float left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p <= right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    default:
                        throw std::runtime_error("Invalid arrow::Type!!!");
                } // switch
                break;
            }
            case arrow::Type::DOUBLE:
            {
                auto left_array = std::static_pointer_cast<arrow::DoubleArray>(left);
                switch (right->type_id()) {
                    case arrow::Type::INT32:
                    {
                        auto right_array = std::static_pointer_cast<arrow::Int32Array>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                // TODO: 这里只能暂时解决Q2中浮点数比较时的精度问题，后续还是要支持DECIMAL类型
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                int right_p = right_array->Value(i);
                                if (left_p <= right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                int right_p = right_array->Value(i);
                                if (left_p <= right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    case arrow::Type::FLOAT:
                    {
                        auto right_array = std::static_pointer_cast<arrow::FloatArray>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p <= right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                float right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p <= right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    case arrow::Type::DOUBLE:
                    {
                        auto right_array = std::static_pointer_cast<arrow::DoubleArray>(right);
                        if (!sel.empty())
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p <= right_p)
                                {
                                    current_sel.push_back(i);
                                    res++;
                                }
                            }
                            if (res == 0) {
                                sel.clear();
                                break;
                            }
                            for (int j = 0; j < res; j++)
                            {
                                res_sel.push_back(sel[current_sel[j]]);
                            }
                            sel.swap(res_sel);
                        }
                        else
                        {
                            for (int i = 0; i < count; i++)
                            {
                                if(left_array->IsNull(i) || right_array->IsNull(i)) {
                                    continue;
                                }
                                double left_p = RoundToDecimalPlaces(left_array->Value(i), 2);
                                double right_p = RoundToDecimalPlaces(right_array->Value(i), 2);
                                if (left_p <= right_p)
                                {
                                    sel.push_back(i);
                                    res++;
                                }
                            }
                        } // else
                        break;
                    }
                    default:
                        throw std::runtime_error("Invalid arrow::Type!!!");
                } // switch
                break;
            }
            case arrow::Type::STRING:
            {
                auto left_array = std::static_pointer_cast<arrow::StringArray>(left);
                auto right_array = std::static_pointer_cast<arrow::StringArray>(right);
                if (!sel.empty())
                {
                    for (int i = 0; i < count; i++)
                    {
                        if(left_array->IsNull(i) || right_array->IsNull(i)) {
                            continue;
                        }
                        if (left_array->GetString(i) <= right_array->GetString(i))
                        {
                            current_sel.push_back(i);
                            res++;
                        }
                    }
                    if (res == 0) {
                        sel.clear();
                        break;
                    }
                    for (int j = 0; j < res; j++)
                    {
                        res_sel.push_back(sel[current_sel[j]]);
                    }
                    sel.swap(res_sel);
                }
                else
                {
                    for (int i = 0; i < count; i++)
                    {
                        if(left_array->IsNull(i) || right_array->IsNull(i)) {
                            continue;
                        }
                        if (left_array->GetString(i) <= right_array->GetString(i))
                        {
                            sel.push_back(i);
                            res++;
                        }
                    }
                }
                break;
            }
            default:
                throw std::runtime_error("Invalid arrow::Type!!!");
        } // switch
        return res;
    }

} // DaseX