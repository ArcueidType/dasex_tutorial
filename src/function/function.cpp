//
// Created by root on 24-4-17.
//
#include "function.hpp"

namespace DaseX {
    Function::Function(std::string name_p) : name(std::move(name_p)) {}

    SimpleFunction::SimpleFunction(std::string name_p,
                                   std::vector<LogicalType> arguments_p,
                                   LogicalType varargs_p)
            : Function(std::move(name_p)),
              arguments_type(std::move(arguments_p)),
              varargs(std::move(varargs_p)) {}

    BaseScalarFunction::BaseScalarFunction(std::string name_p,
                                           std::vector<LogicalType> arguments_p,
                                           LogicalType return_type_p,
                                           LogicalType varargs_p)
            : SimpleFunction(std::move(name_p), std::move(arguments_p), std::move(varargs_p)),
              return_type(std::move(return_type_p)) {}
} // DaseX