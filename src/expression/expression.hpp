#pragma once

#include "expression_type.hpp"
#include "logical_type.hpp"
#include <string>
#include <stdexcept>
#include <memory>

namespace DaseX
{

class Expression;
// The BaseExpression class is a base class that can represent any expression part of a SQL statement.
class BaseExpression
{
public:
    //! Type of the expression
    ExpressionTypes type;
    //! The expression class of the node
    ExpressionClass expression_class;
    //! The alias of the expression,
    std::string alias;

public:
    //! Create an Expression
    BaseExpression(ExpressionTypes type, ExpressionClass expression_class)
        : type(type), expression_class(expression_class) {}
    virtual ~BaseExpression() {}
    //! Returns the type of the expression
    ExpressionTypes GetExpressionType() const { return type; }
    //! Returns the class of the expression
    ExpressionClass GetExpressionClass() const { return expression_class; }
    //! Returns true if this expression is an aggregate or not.
    virtual bool IsAggregate() const = 0;
    //! Returns true if expression does not contain a group ref or col ref or parameter
    virtual bool IsScalar() const = 0;
    //! Returns true if the expression has a parameter
    virtual bool HasParameter() const = 0;

    virtual std::shared_ptr<Expression> Copy() = 0;

public:
    template <class TARGET>
    TARGET &Cast()
    {
        if (expression_class != TARGET::TYPE)
        {
            throw std::runtime_error("Failed to cast expression to type - expression type mismatch");
        }
        return reinterpret_cast<TARGET &>(*this);
    }

    template <class TARGET>
    const TARGET &Cast() const
    {
        if (expression_class != TARGET::TYPE)
        {
            throw std::runtime_error("Failed to cast expression to type - expression type mismatch");
        }
        return reinterpret_cast<const TARGET &>(*this);
    }
};

    class Expression;
    using ExpressionRef = std::shared_ptr<Expression>;

class Expression : public BaseExpression
{

public:
    //! The return type of the expression
    LogicalType return_type;

public:
    Expression(ExpressionTypes type, ExpressionClass expression_class, LogicalType return_type)
        : BaseExpression(type, expression_class), return_type(std::move(return_type)) {}
    ~Expression() override {}
    bool IsAggregate() const override { return false; }
    bool IsScalar() const override { return false; }
    bool HasParameter() const override { return false; }
    virtual std::shared_ptr<Expression> Copy() override;
};

} // DaseX
