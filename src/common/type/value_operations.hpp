#pragma once

#include "value_type.hpp"
#include "comparison_operators.hpp"

namespace DaseX
{
    class ValueOperations
    {
    public:
        ValueOperations() {}

    public:
        //===--------------------------------------------------------------------===//
        // Comparison Operations
        //===--------------------------------------------------------------------===//
        // A == B
        static bool Equals(const Value &left, const Value &right)
        {
            return TemplatedBooleanOperation<DaseX::Equals>(left, right);
        }
        // A != B
        static bool NotEquals(const Value &left, const Value &right)
        {
            return !ValueOperations::Equals(left, right);
        }
        // A > B
        static bool GreaterThan(const Value &left, const Value &right)
        {
            return TemplatedBooleanOperation<DaseX::GreaterThan>(left, right);
        }
        // A >= B
        static bool GreaterThanEquals(const Value &left, const Value &right)
        {
            return !ValueOperations::GreaterThan(right, left);
        }
        // A < B
        static bool LessThan(const Value &left, const Value &right)
        {
            return ValueOperations::GreaterThan(right, left);
        }
        // A <= B
        static bool LessThanEquals(const Value &left, const Value &right)
        {
            return !ValueOperations::GreaterThan(left, right);
        }
    };
} // DaseX
