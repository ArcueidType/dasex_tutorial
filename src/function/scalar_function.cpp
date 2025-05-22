#include "scalar_function.hpp"

namespace DaseX {
    ScalarFunction::ScalarFunction(std::string name,
                                   std::vector<LogicalType> arguments,
                                   LogicalType return_type,
                                   scalar_function_t function,
                                   bind_scalar_function_t bind,
                                   LogicalType varargs)
            : BaseScalarFunction(std::move(name), std::move(arguments), std::move(return_type), std::move(varargs)),
              function(std::move(function)), bind(bind) {}

    ScalarFunction::ScalarFunction(std::vector<LogicalType> arguments,
                                   LogicalType return_type,
                                   scalar_function_t function,
                                   bind_scalar_function_t bind,
                                   LogicalType varargs)
            : ScalarFunction(std::string(),
                             std::move(arguments),
                             std::move(return_type),
                             std::move(function),
                             bind,
                             std::move(varargs)) {}


ScalarProjectFunction::ScalarProjectFunction(std::string name,
                                             std::vector<LogicalType> arguments,
                                             LogicalType return_type,
                                             scalar_function_p function,
                                             bind_scalar_function_p bind,
                                             LogicalType varargs)
            : BaseScalarFunction(std::move(name), std::move(arguments), std::move(return_type), std::move(varargs)),
              function(std::move(function)), bind(bind) {}

ScalarProjectFunction::ScalarProjectFunction(std::vector<LogicalType> arguments, LogicalType return_type,
                                             scalar_function_p function, bind_scalar_function_p bind,
                                             LogicalType varargs)
            : ScalarProjectFunction(std::string(),
                                    std::move(arguments),
                                    std::move(return_type),
                                    std::move(function),
                                    bind,
                                    std::move(varargs)) {}
} // DaseX
