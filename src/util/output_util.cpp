

#include "output_util.hpp"


namespace DaseX {

/*
把Chunk输出，输出格式：
+-----------+--------------------+--------------------------------+-----------------+-------------+
|c_custkey  |c_name              |c_address                       |c_phone          |c_acctbal    |
+-----------+--------------------+--------------------------------+-----------------+-------------+
|1          |Customer#000000001  |IVhzIApeRb ot,c,E               |25-989-741-2988  |711.56       |
|2          |Customer#000000002  |XSTf4,NCwDVaWNe6tEgvwfmRchLXak  |23-768-687-3665  |121.65       |
|3          |Customer#000000003  |MG9kdTD2WBHm                    |11-719-748-3364  |7498.12      |
|4          |Customer#000000004  |XxVSJsLAGtn                     |14-128-190-5944  |2866.83      |
|5          |Customer#000000005  |KvpyuHCplrB84WgAiGV6sYpZq7Tj    |13-750-942-6364  |794.47       |
+-----------+--------------------+--------------------------------+-----------------+-------------+
*/
void DisplayChunk(std::shared_ptr<arrow::RecordBatch> &record_batch, int limit, bool tail) {
    if(record_batch.get() == nullptr || record_batch->num_rows() == 0) {
        std::cout << "Output is empty.\n\n";
        return;
    }

    auto names = std::move(record_batch->ColumnNames());
    std::vector<size_t> width(record_batch->num_columns(), 0);
    
    // Get the width of each column
    for (int i=0; i < names.size(); i++) {
        width[i] = std::max(width[i], names[i].size()+2);
    }
    int64_t row = 0;
    int64_t upper = limit;
    if (tail) {
        row = record_batch->num_rows() - limit;
        upper = record_batch->num_rows();
    }
    for (; row < record_batch->num_rows() && row < upper; ++row) {
        for (int col = 0; col < record_batch->num_columns(); ++col) {
            auto column = record_batch->column(col);
            if (column->type()->id() == arrow::Type::INT32) {                       // LogicalType::INTEGER
                auto int32_array = std::static_pointer_cast<arrow::Int32Array>(column);
                width[col] = std::max(width[col], std::to_string(int32_array->Value(row)).size()+2);
            } else if (column->type()->id() == arrow::Type::FLOAT) {                // LogicalType::FLOAT
                auto float_array = std::static_pointer_cast<arrow::FloatArray>(column);
                width[col] = std::max(width[col], std::to_string(float_array->Value(row)).size()+2);
            } else if (column->type()->id() == arrow::Type::DOUBLE) {               // LogicalType::Double
                auto double_array = std::static_pointer_cast<arrow::DoubleArray>(column);
                width[col] = std::max(width[col], std::to_string(double_array->Value(row)).size()+2);
            } else if (column->type()->id() == arrow::Type::STRING) {               // LogicalType::STRING
                auto string_array = std::static_pointer_cast<arrow::StringArray>(column);
                width[col] = std::max(width[col], string_array->GetString(row).size()+2);
            } else if (column->type()->id() == arrow::Type::BOOL) {                // LogicalType::BOOL
                auto bool_array = std::static_pointer_cast<arrow::BooleanArray>(column);
                width[col] = std::max(width[col], std::to_string(bool_array->Value(row)).size()+2);
            }
        }
    }

    // print results
    std::cout << "\nNumber of rows: " << record_batch->num_rows() << "\n";
    std::cout << "+";
    for (size_t i = 0; i < names.size(); ++i) {
        std::cout << std::string(width[i], '-') << "+";
    }
    std::cout << "\n|";
    for (int col=0; col < names.size(); col++) {
        std::cout << std::setw(width[col]) << std::left << names[col] << "|";
    }

    std::cout << "\n+";
    for (size_t i = 0; i < names.size(); ++i) {
        std::cout << std::string(width[i], '-') << "+"; 
    }
    std::cout << "\n";

    // print each row
    row = 0;
    upper = limit;
    if (tail) {
        row = record_batch->num_rows() - limit;
        upper = record_batch->num_rows();
    }
    for (; row < record_batch->num_rows() && row < upper; ++row) {
        std::cout << "|";
        for (int col = 0; col < record_batch->num_columns(); ++col) {
            auto column = record_batch->column(col);
            if (column->type()->id() == arrow::Type::INT32) {                       // LogicalType::INTEGER
                auto int32_array = std::static_pointer_cast<arrow::Int32Array>(column);
                std::cout << std::setw(width[col]) << std::left << int32_array->Value(row) << "|";
            } else if (column->type()->id() == arrow::Type::FLOAT) {                // LogicalType::FLOAT
                auto float_array = std::static_pointer_cast<arrow::FloatArray>(column);
                std::cout << std::setw(width[col]) << std::left << float_array->Value(row) << "|";
            } else if (column->type()->id() == arrow::Type::DOUBLE) {               // LogicalType::Double
                auto double_array = std::static_pointer_cast<arrow::DoubleArray>(column);
                std::cout << std::setw(width[col]) << std::left << double_array->Value(row) << "|";
            } else if (column->type()->id() == arrow::Type::STRING) {               // LogicalType::STRING
                auto string_array = std::static_pointer_cast<arrow::StringArray>(column);
                std::cout << std::setw(width[col]) << std::left << string_array->GetString(row) << "|";
            } else if (column->type()->id() == arrow::Type::BOOL) {                // LogicalType::BOOL
                auto bool_array = std::static_pointer_cast<arrow::BooleanArray>(column);
                std::cout << std::setw(width[col]) << std::left << bool_array->Value(row) << "|";
            } else {
                std::cout << std::setw(width[col]) << std::left << "N/A" << "|";    // LogicalType::INVALID
            }
        }
        std::cout << "\n";
    }

    std::cout << "+";
    for (size_t i = 0; i < names.size(); ++i) {
        std::cout << std::string(width[i], '-') << "+";
    }
    std::cout << "\n\n";
}

void DisplayChunk(
	std::vector<std::shared_ptr<arrow::RecordBatch>>& record_batch_array,
	int limit, bool tail) {
    uint64_t num_rows = 0;

    for(auto &&rb : record_batch_array) {
        num_rows += rb->num_rows();
    }
//    std::cout << "\nNumber of chunks: " << record_batch_array.size() << "\n";
//    std::cout << "Number of total rows: " << num_rows << "\n";

    for(auto &&rb : record_batch_array) {
        DisplayChunk(rb, limit, tail);
    }
}


} // DaseX
