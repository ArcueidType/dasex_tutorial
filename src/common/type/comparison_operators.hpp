#pragma once

#include "logical_type.hpp"
#include <cstring>
#include <stdexcept>

namespace DaseX
{

    //===--------------------------------------------------------------------===//
    // Comparison Operations
    //===--------------------------------------------------------------------===//
    // TODO:目前只支持强类型比较，不支持弱类型比较，后续优化，要支持弱类型比较需要把这里的Operation重载
    struct Equals
    {
        template <class T>
        static inline bool Operation(const T &left, const T &right)
        {
            return left == right;
        }
    };
    struct NotEquals
    {
        template <class T>
        static inline bool Operation(const T &left, const T &right)
        {
            return !Equals::Operation(left, right);
        }
    };

    struct GreaterThan
    {
        template <class T>
        static inline bool Operation(const T &left, const T &right)
        {
            return left > right;
        }
    };

    struct GreaterThanEquals
    {
        template <class T>
        static inline bool Operation(const T &left, const T &right)
        {
            return !GreaterThan::Operation(right, left);
        }
    };

    struct LessThan
    {
        template <class T>
        static inline bool Operation(const T &left, const T &right)
        {
            return GreaterThan::Operation(right, left);
        }
    };

    struct LessThanEquals
    {
        template <class T>
        static inline bool Operation(const T &left, const T &right)
        {
            return !GreaterThan::Operation(left, right);
        }
    };

    template <class OP>
    static bool TemplatedBooleanOperation(const Value &left, const Value &right)
    {
        switch (left.type_)
        {
        case LogicalType::INTEGER:
            return OP::Operation(left.GetValueUnsafe<int>(), right.GetValueUnsafe<int>());
        case LogicalType::FLOAT:
            return OP::Operation(left.GetValueUnsafe<float>(), right.GetValueUnsafe<float>());
        case LogicalType::DOUBLE:
            return OP::Operation(left.GetValueUnsafe<double>(), right.GetValueUnsafe<double>());
        case LogicalType::STRING:
            return OP::Operation(left.GetValueUnsafe<std::string>(), right.GetValueUnsafe<std::string>());
        default:
            throw std::runtime_error("Unimplemented type for value comparison");
        }
    };

} // namespace DaseX
