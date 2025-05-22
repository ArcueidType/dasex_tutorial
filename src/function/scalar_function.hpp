#pragma once

#include "function.hpp"
#include "expression_executor_state.hpp"
#include "logical_type.hpp"
#include "expression.hpp"
#include <arrow/api.h>
#include <functional>
#include <vector>
#include <memory>

namespace DaseX {

    //! The type used for scalar functions
    using scalar_function_t = std::function<void(std::shared_ptr<arrow::Array> &input, ExpressionState &state, std::vector<int> &result)>;
    typedef std::shared_ptr<FunctionData> (*bind_scalar_function_t)(ScalarFunction &bound_function, std::vector<std::shared_ptr<Expression>> &arguments);

    class ScalarFunction : public BaseScalarFunction{
    public:
        //! The main scalar function to execute
        scalar_function_t function;
        //! The bind function (if any)
        bind_scalar_function_t bind;
    public:
        ScalarFunction(std::string name, std::vector<LogicalType> arguments, LogicalType return_type,
                                  scalar_function_t function, bind_scalar_function_t bind = nullptr,
                                  LogicalType varargs = LogicalType::INVALID);

        ScalarFunction(std::vector<LogicalType> arguments, LogicalType return_type,
                       scalar_function_t function, bind_scalar_function_t bind = nullptr,
                       LogicalType varargs = LogicalType::INVALID);
        /*template <class TA, class TR, class OP>
        static void UnaryFunction(std::shared_ptr<arrow::Array> &input, ExpressionState &state, std::vector<int> &result) {
            UnaryExecutor::Execute<TA, TR, OP>(input, result, input->length());
        }*/
    };

    //! The type used for scalar functions
    using scalar_function_p = std::function<void(std::vector<std::shared_ptr<arrow::Array>> &input, ExpressionState &state, std::shared_ptr<arrow::Array> &result)>;
    typedef std::shared_ptr<FunctionData> (*bind_scalar_function_p)(ScalarProjectFunction &bound_function, std::vector<std::shared_ptr<Expression>> &arguments);

    class ScalarProjectFunction : public BaseScalarFunction{
    public:
        //! The main scalar function to execute
        scalar_function_p function;
        //! The bind function (if any)
        bind_scalar_function_p bind;
    public:
        ScalarProjectFunction(std::string name, std::vector<LogicalType> arguments, LogicalType return_type,
                              scalar_function_p function, bind_scalar_function_p bind = nullptr,
                              LogicalType varargs = LogicalType::INVALID);

        ScalarProjectFunction(std::vector<LogicalType> arguments, LogicalType return_type,
                              scalar_function_p function, bind_scalar_function_p bind = nullptr,
                              LogicalType varargs = LogicalType::INVALID);
    };
} // DaseX
