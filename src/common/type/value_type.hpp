/*
 * @Author: caiwanli 651943559@qq.com
 * @Date: 2024-04-03 13:15:46
 * @LastEditors: caiwanli 651943559@qq.com
 * @LastEditTime: 2024-04-11 13:15:29
 * @FilePath: /task_sche/src/common/type/value_type.hpp
 * @Description:
 */
#pragma once

#include "logical_type.hpp"
#include <string>
#include <memory>

namespace DaseX
{

class ExtraValueInfo;

class Value {
public:
    union Val {
        int int_;
        float float_;
        double double_;
    } value_;
    LogicalType type_;
    bool is_null;
    // string特殊类型可以将值放在此处
    std::shared_ptr<ExtraValueInfo> value_info_;

public:
    explicit Value(int val_) : type_(LogicalType::INTEGER), is_null(false) {
        value_.int_ = val_;
    }

    explicit Value(float val_) : type_(LogicalType::FLOAT), is_null(false) {
        value_.float_ = val_;
    }

    explicit Value(double val_) : type_(LogicalType::DOUBLE), is_null(false) {
        value_.double_ = val_;
    }

    explicit Value(std::string val_);

    //! Copy constructor
    Value(const Value &other);

    //! Move constructor
    Value(Value &&other) noexcept;

    // copy assignment
    Value &operator=(const Value &other);

    // move assignment
    Value &operator=(Value &&other) noexcept;

    ~Value() {}

public:
    //===--------------------------------------------------------------------===//
    // Comparison Operators
    //===--------------------------------------------------------------------===//
    bool operator==(const Value &rhs) const;

    bool operator!=(const Value &rhs) const;

    bool operator<(const Value &rhs) const;

    bool operator>(const Value &rhs) const;

    bool operator<=(const Value &rhs) const;

    bool operator>=(const Value &rhs) const;

    template<class T>
    T GetValueUnsafe() const;

    Value Copy() const {
        return Value(*this);
    }
}; // Value

} // namespace DaseX
