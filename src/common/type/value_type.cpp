#include "value_type.hpp"
#include "value_operations.hpp"

namespace DaseX
{
    class ExtraValueInfo
    {
    public:
        ExtraValueInfo() {}
        virtual ~ExtraValueInfo() {}

    public:
        template <class T>
        T &Get()
        {
            return (T &)*this;
        }
    };

    class StringValueInfo : public ExtraValueInfo
    {
    public:
        std::string str;
        StringValueInfo(std::string str_p) : str(std::move(str_p)) {}
        const std::string &GetString() { return str; }
    };

    //===--------------------------------------------------------------------===//
    // Value
    //===--------------------------------------------------------------------===//

    Value::Value(std::string val_) : type_(LogicalType::STRING), is_null(false)
    {
        // TODO: 这里需要验证下字符串的合理性，后续优化
        value_info_ = std::make_shared<StringValueInfo>(std::move(val_));
    }

    Value::Value(const Value &other)
        : type_(other.type_), is_null(other.is_null), value_(other.value_), value_info_(other.value_info_) {}

    Value::Value(Value &&other) noexcept
        : type_(std::move(other.type_)), is_null(other.is_null), value_(other.value_),
          value_info_(std::move(other.value_info_)) {}

    Value &Value::operator=(const Value &other)
    {
        if (this == &other)
        {
            return *this;
        }
        type_ = other.type_;
        is_null = other.is_null;
        value_ = other.value_;
        value_info_ = other.value_info_;
        return *this;
    }

    Value &Value::operator=(Value &&other) noexcept
    {
        type_ = std::move(other.type_);
        is_null = other.is_null;
        value_ = other.value_;
        value_info_ = std::move(other.value_info_);
        return *this;
    }

    //===--------------------------------------------------------------------===//
    // Comparison Operators
    //===--------------------------------------------------------------------===//
    bool Value::operator==(const Value &rhs) const
    {
        return ValueOperations::Equals(*this, rhs);
    }

    bool Value::operator!=(const Value &rhs) const
    {
        return ValueOperations::NotEquals(*this, rhs);
    }

    bool Value::operator<(const Value &rhs) const
    {
        return ValueOperations::LessThan(*this, rhs);
    }

    bool Value::operator>(const Value &rhs) const
    {
        return ValueOperations::GreaterThan(*this, rhs);
    }

    bool Value::operator<=(const Value &rhs) const
    {
        return ValueOperations::LessThanEquals(*this, rhs);
    }

    bool Value::operator>=(const Value &rhs) const
    {
        return ValueOperations::GreaterThanEquals(*this, rhs);
    }

    template <>
    int Value::GetValueUnsafe<int>() const
    {
        return value_.int_;
    }

    template <>
    float Value::GetValueUnsafe<float>() const
    {
        return value_.float_;
    }

    template <>
    double Value::GetValueUnsafe<double>() const
    {
        return value_.double_;
    }

    template <>
    std::string Value::GetValueUnsafe<std::string>() const
    {
        return (*value_info_).Get<StringValueInfo>().GetString();
    }

} // DaseX