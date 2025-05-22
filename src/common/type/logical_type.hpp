#pragma once

#include <arrow/type.h>
#include <arrow/type_fwd.h>
#include "exception.h"
namespace DaseX
{

enum class LogicalType
{
    INVALID,
    INTEGER,
    FLOAT,
    DOUBLE,
    STRING, // VARCHAR
    BOOL
};

static LogicalType SchemaTypeToLogicalType(const std::shared_ptr<arrow::DataType>& dataType) {
    auto internal_col_type = LogicalType::INTEGER;
    if (dataType == arrow::int32()) {
        return LogicalType::INTEGER;
    } else if (dataType == arrow::float32()) {
        return LogicalType::FLOAT;
    }  else if (dataType == arrow::float64()) {
        return LogicalType::DOUBLE;
    } else if (dataType == arrow::boolean()) {
        return LogicalType::BOOL;
    } else if (dataType == arrow::utf8()) {
        return LogicalType::STRING;
    }
    throw NotImplementedException("SchemaTypeToLogicalType unsupported data type");
}

static std::string LogicalTypeToString(LogicalType type) {
    switch (type)
    {
    case LogicalType::INTEGER:
        return "INTEGER";
    case LogicalType::FLOAT:
        return "FLOAT";
    case LogicalType::DOUBLE:
        return "DOUBLE";
    case LogicalType::STRING:
        return "STRING";
    case LogicalType::BOOL:
        return "BOOL";
   default:
        return "ERROR";
    };
}

static LogicalType StringToLogicalType(const std::string &type) {
    switch (type[0]) {
    case 'I':
        return LogicalType::INTEGER;
    case 'F':
        return LogicalType::FLOAT;
    case 'D':
        return LogicalType::DOUBLE;
    case 'S':
        return LogicalType::STRING;
    case 'B':
        return LogicalType::BOOL;
    default:
        throw ExecutionException("StringToLogicalType unsupportedtype");
    }
}

} // namespace DaseX
