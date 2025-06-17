#include "arrow_help.hpp"


namespace DaseX {
namespace Util {

void join_two_record_batch(std::shared_ptr<arrow::RecordBatch> &left,
                           int left_col_nums, int left_col,
                           std::shared_ptr<arrow::RecordBatch> &right,
                           int right_col_nums, int right_col,
                           std::shared_ptr<arrow::RecordBatch> &res) {
    auto left_schema = left->schema();
    auto right_schema = right->schema();
    res = left->Slice(0, 1);
    int offset = left_col_nums;
    for (int i = 0; i < right_col_nums; i++) {
        auto filed = right_schema->field(i);
        std::string filed_name = "l." + filed->name(); // 这里前缀需要替换成实际的表名
        auto column = right->column(i);
        res = res->AddColumn(offset++, filed_name, column).ValueOrDie();
    }
} // join_two_record_batch

bool MatchTwoRecordBatch(std::shared_ptr<arrow::RecordBatch> &left,
                         std::shared_ptr<arrow::RecordBatch> &right,
                         const std::vector<int> &left_ids,
                         const std::vector<int> &right_ids,
                         const std::vector<ExpressionTypes> &compare_types) {
    for(int i = 0; i < left_ids.size(); i++) {
        auto left_column = left->column(left_ids[i]);
        auto right_column = right->column(right_ids[i]);
        switch (left_column->type_id()) {
            case arrow::Type::INT32:
            {
                auto int32array_left = std::static_pointer_cast<arrow::Int32Array>(left_column);
                switch (right_column->type_id()) {
                    case arrow::Type::INT32:
                    {
                        auto floatArray_right = std::static_pointer_cast<arrow::Int32Array>(right_column);
                        int left_val = int32array_left->Value(0);
                        int right_val = floatArray_right->Value(0);
                        ExpressionTypes compare_type = compare_types[i];
                        switch(compare_type) {
                            case ExpressionTypes::COMPARE_LESSTHAN:
                            {
                                if(left_val >= right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_LESSTHANOREQUALTO:
                            {
                                if(left_val > right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_EQUAL:
                            {
                                if(left_val != right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_NOTEQUAL:
                            {
                                if(left_val == right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_GREATERTHAN:
                            {
                                if(left_val <= right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_GREATERTHANOREQUALTO:
                            {
                                if(left_val < right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            default:
                                throw std::runtime_error("Unknown comparison type!");
                        } // switch
                        break;
                    }
                    case arrow::Type::FLOAT:
                    {
                        auto floatArray_right = std::static_pointer_cast<arrow::FloatArray>(right_column);
                        int left_val = int32array_left->Value(0);
                        float right_val = floatArray_right->Value(0);
                        ExpressionTypes compare_type = compare_types[i];
                        switch(compare_type) {
                            case ExpressionTypes::COMPARE_LESSTHAN:
                            {
                                if(left_val >= right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_LESSTHANOREQUALTO:
                            {
                                if(left_val > right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_EQUAL:
                            {
                                if(left_val != right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_NOTEQUAL:
                            {
                                if(left_val == right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_GREATERTHAN:
                            {
                                if(left_val <= right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_GREATERTHANOREQUALTO:
                            {
                                if(left_val < right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            default:
                                throw std::runtime_error("Unknown comparison type!");
                        } // switch
                        break;
                    }
                    case arrow::Type::DOUBLE:
                    {
                        auto floatArray_right = std::static_pointer_cast<arrow::DoubleArray>(right_column);
                        int left_val = int32array_left->Value(0);
                        double right_val = floatArray_right->Value(0);
                        ExpressionTypes compare_type = compare_types[i];
                        switch(compare_type) {
                            case ExpressionTypes::COMPARE_LESSTHAN:
                            {
                                if(left_val >= right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_LESSTHANOREQUALTO:
                            {
                                if(left_val > right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_EQUAL:
                            {
                                if(left_val != right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_NOTEQUAL:
                            {
                                if(left_val == right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_GREATERTHAN:
                            {
                                if(left_val <= right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_GREATERTHANOREQUALTO:
                            {
                                if(left_val < right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            default:
                                throw std::runtime_error("Unknown comparison type!");
                        } // switch
                        break;
                    }
                    default:
                        throw std::runtime_error("Not Supported For Arrow Type!!!");
                }
                break;
            }
            case arrow::Type::FLOAT:
            {
                auto floatArray_left = std::static_pointer_cast<arrow::FloatArray>(left_column);
                switch (right_column->type_id()) {
                    case arrow::Type::INT32:
                    {
                        auto floatArray_right = std::static_pointer_cast<arrow::Int32Array>(right_column);
                        float left_val = floatArray_left->Value(0);
                        int right_val = floatArray_right->Value(0);
                        ExpressionTypes compare_type = compare_types[i];
                        switch(compare_type) {
                            case ExpressionTypes::COMPARE_LESSTHAN:
                            {
                                if(left_val >= right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_LESSTHANOREQUALTO:
                            {
                                if(left_val > right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_EQUAL:
                            {
                                if(left_val != right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_NOTEQUAL:
                            {
                                if(left_val == right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_GREATERTHAN:
                            {
                                if(left_val <= right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_GREATERTHANOREQUALTO:
                            {
                                if(left_val < right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            default:
                                throw std::runtime_error("Unknown comparison type!");
                        } // switch
                        break;
                    }
                    case arrow::Type::FLOAT:
                    {
                        auto floatArray_right = std::static_pointer_cast<arrow::FloatArray>(right_column);
                        float left_val = floatArray_left->Value(0);
                        float right_val = floatArray_right->Value(0);
                        ExpressionTypes compare_type = compare_types[i];
                        switch(compare_type) {
                            case ExpressionTypes::COMPARE_LESSTHAN:
                            {
                                if(left_val >= right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_LESSTHANOREQUALTO:
                            {
                                if(left_val > right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_EQUAL:
                            {
                                if(left_val != right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_NOTEQUAL:
                            {
                                if(left_val == right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_GREATERTHAN:
                            {
                                if(left_val <= right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_GREATERTHANOREQUALTO:
                            {
                                if(left_val < right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            default:
                                throw std::runtime_error("Unknown comparison type!");
                        } // switch
                        break;
                    }
                    case arrow::Type::DOUBLE:
                    {
                        auto floatArray_right = std::static_pointer_cast<arrow::DoubleArray>(right_column);
                        float left_val = floatArray_left->Value(0);
                        double right_val = floatArray_right->Value(0);
                        ExpressionTypes compare_type = compare_types[i];
                        switch(compare_type) {
                            case ExpressionTypes::COMPARE_LESSTHAN:
                            {
                                if(left_val >= right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_LESSTHANOREQUALTO:
                            {
                                if(left_val > right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_EQUAL:
                            {
                                if(left_val != right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_NOTEQUAL:
                            {
                                if(left_val == right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_GREATERTHAN:
                            {
                                if(left_val <= right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_GREATERTHANOREQUALTO:
                            {
                                if(left_val < right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            default:
                                throw std::runtime_error("Unknown comparison type!");
                        } // switch
                        break;
                    }
                    default:
                        throw std::runtime_error("Not Supported For Arrow Type!!!");
                }
                break;
            } // case-FLOAT
            case arrow::Type::DOUBLE:
            {
                auto doubleArray_left = std::static_pointer_cast<arrow::DoubleArray>(left_column);
                switch (right_column->type_id()) {
                    case arrow::Type::INT32:
                    {
                        auto floatArray_right = std::static_pointer_cast<arrow::Int32Array>(right_column);
                        double left_val = doubleArray_left->Value(0);
                        int right_val = floatArray_right->Value(0);
                        ExpressionTypes compare_type = compare_types[i];
                        switch(compare_type) {
                            case ExpressionTypes::COMPARE_LESSTHAN:
                            {
                                if(left_val >= right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_LESSTHANOREQUALTO:
                            {
                                if(left_val > right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_EQUAL:
                            {
                                if(left_val != right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_NOTEQUAL:
                            {
                                if(left_val == right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_GREATERTHAN:
                            {
                                if(left_val <= right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_GREATERTHANOREQUALTO:
                            {
                                if(left_val < right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            default:
                                throw std::runtime_error("Unknown comparison type!");
                        } // switch
                        break;
                    }
                    case arrow::Type::FLOAT:
                    {
                        auto floatArray_right = std::static_pointer_cast<arrow::FloatArray>(right_column);
                        double left_val = doubleArray_left->Value(0);
                        float right_val = floatArray_right->Value(0);
                        ExpressionTypes compare_type = compare_types[i];
                        switch(compare_type) {
                            case ExpressionTypes::COMPARE_LESSTHAN:
                            {
                                if(left_val >= right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_LESSTHANOREQUALTO:
                            {
                                if(left_val > right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_EQUAL:
                            {
                                if(left_val != right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_NOTEQUAL:
                            {
                                if(left_val == right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_GREATERTHAN:
                            {
                                if(left_val <= right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_GREATERTHANOREQUALTO:
                            {
                                if(left_val < right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            default:
                                throw std::runtime_error("Unknown comparison type!");
                        } // switch
                        break;
                    }
                    case arrow::Type::DOUBLE:
                    {
                        auto floatArray_right = std::static_pointer_cast<arrow::DoubleArray>(right_column);
                        double left_val = doubleArray_left->Value(0);
                        double right_val = floatArray_right->Value(0);
                        ExpressionTypes compare_type = compare_types[i];
                        switch(compare_type) {
                            case ExpressionTypes::COMPARE_LESSTHAN:
                            {
                                if(left_val >= right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_LESSTHANOREQUALTO:
                            {
                                if(left_val > right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_EQUAL:
                            {
                                if(left_val != right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_NOTEQUAL:
                            {
                                if(left_val == right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_GREATERTHAN:
                            {
                                if(left_val <= right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            case ExpressionTypes::COMPARE_GREATERTHANOREQUALTO:
                            {
                                if(left_val < right_val) {
                                    return false;
                                } else {
                                    break;
                                }
                            }
                            default:
                                throw std::runtime_error("Unknown comparison type!");
                        } // switch
                        break;
                    }
                    default:
                        throw std::runtime_error("Not Supported For Arrow Type!!!");
                }
                break;
            }
            case arrow::Type::STRING:
            {
                auto stringArray_left = std::static_pointer_cast<arrow::StringArray>(left_column);
                auto stringArray_right = std::static_pointer_cast<arrow::StringArray>(right_column);
                auto left_val = stringArray_left->GetString(0);
                auto right_val = stringArray_right->GetString(0);
                ExpressionTypes compare_type = compare_types[i];
                switch(compare_type) {
                    case ExpressionTypes::COMPARE_LESSTHAN:
                    {
                        if(left_val >= right_val) {
                            return false;
                        } else {
                            break;
                        }
                    }
                    case ExpressionTypes::COMPARE_LESSTHANOREQUALTO:
                    {
                        if(left_val > right_val) {
                            return false;
                        } else {
                            break;
                        }
                    }
                    case ExpressionTypes::COMPARE_EQUAL:
                    {
                        if(left_val != right_val) {
                            return false;
                        } else {
                            break;
                        }
                    }
                    case ExpressionTypes::COMPARE_NOTEQUAL:
                    {
                        if(left_val == right_val) {
                            return false;
                        } else {
                            break;
                        }
                    }
                    case ExpressionTypes::COMPARE_GREATERTHAN:
                    {
                        if(left_val <= right_val) {
                            return false;
                        } else {
                            break;
                        }
                    }
                    case ExpressionTypes::COMPARE_GREATERTHANOREQUALTO:
                    {
                        if(left_val < right_val) {
                            return false;
                        } else {
                            break;
                        }
                    }
                    default:
                        throw std::runtime_error("Unknown comparison type!");
                } // switch
                break;
            }
            default:
                throw std::runtime_error("Invalid arrow::Type!!!");
        } // switch
    } // for
    return true;
}

std::shared_ptr<arrow::Schema> MergeTwoSchema(std::shared_ptr<arrow::Schema> &left, std::shared_ptr<arrow::Schema> &right) {
    std::shared_ptr<arrow::Schema> res;
    auto left_fields_vector = left->fields();
    auto right_fields_vector = right->fields();
    arrow::FieldVector res_fields_vector;
    for(auto &en : left_fields_vector) {
        res_fields_vector.push_back(en);
    }
    for (auto &en : right_fields_vector) {
        std::string filed_name = "l." + en->name();
        auto field = en->WithName(filed_name);
        res_fields_vector.push_back(field);
    }
    res = std::make_shared<arrow::Schema>(res_fields_vector);
    return res;
}

arrow::Status GetSinlgeRowNull(std::shared_ptr<arrow::RecordBatch> &res, std::shared_ptr<arrow::Schema>& schema) {
    int num_fields = schema->num_fields();
    arrow::ArrayVector data_vector;
    for(int i = 0; i < num_fields; i++) {
        std::shared_ptr<arrow::Field> field_p = schema->field(i);
        switch (field_p->type()->id()) {
            case arrow::Type::INT32:
            {
                arrow::Int32Builder builder;
                ARROW_RETURN_NOT_OK(builder.AppendNull());
                std::shared_ptr<arrow::Array> int32Column;
                ARROW_ASSIGN_OR_RAISE(int32Column, builder.Finish());
                data_vector.push_back(int32Column);
                break;
            }
            case arrow::Type::FLOAT:
            {
                arrow::FloatBuilder builder;
                ARROW_RETURN_NOT_OK(builder.AppendNull());
                std::shared_ptr<arrow::Array> floatColumn;
                ARROW_ASSIGN_OR_RAISE(floatColumn, builder.Finish());
                data_vector.push_back(floatColumn);
                break;
            }
            case arrow::Type::DOUBLE:
            {
                arrow::DoubleBuilder builder;
                ARROW_RETURN_NOT_OK(builder.AppendNull());
                std::shared_ptr<arrow::Array> doubleColumn;
                ARROW_ASSIGN_OR_RAISE(doubleColumn, builder.Finish());
                data_vector.push_back(doubleColumn);
                break;
            }
            case arrow::Type::STRING:
            {
                arrow::StringBuilder builder;
                ARROW_RETURN_NOT_OK(builder.AppendNull());
                std::shared_ptr<arrow::Array> stringColumn;
                ARROW_ASSIGN_OR_RAISE(stringColumn, builder.Finish());
                data_vector.push_back(stringColumn);
                break;
            }
            default:
                throw std::runtime_error("Unsupported data type !!!");
        } // switch
    }
    res = arrow::RecordBatch::Make(schema, 1, data_vector);
    return arrow::Status::OK();
}

arrow::Status GetSinlgeRow(std::shared_ptr<arrow::RecordBatch> &res, std::shared_ptr<arrow::Schema>& schema) {
    int num_fields = schema->num_fields();
    arrow::ArrayVector data_vector;
    for(int i = 0; i < num_fields; i++) {
        std::shared_ptr<arrow::Field> field_p = schema->field(i);
        switch (field_p->type()->id()) {
            case arrow::Type::INT32:
            {
                arrow::Int32Builder builder;
                int int_default[] = {-1};
                ARROW_RETURN_NOT_OK(builder.AppendValues(int_default, 1));
                std::shared_ptr<arrow::Array> int32Column;
                ARROW_ASSIGN_OR_RAISE(int32Column, builder.Finish());
                data_vector.push_back(int32Column);
                break;
            }
            case arrow::Type::FLOAT:
            {
                arrow::FloatBuilder builder;
                float float_default[] = {-1.0F};
                ARROW_RETURN_NOT_OK(builder.AppendValues(float_default, 1));
                std::shared_ptr<arrow::Array> floatColumn;
                ARROW_ASSIGN_OR_RAISE(floatColumn, builder.Finish());
                data_vector.push_back(floatColumn);
                break;
            }
            case arrow::Type::DOUBLE:
            {
                arrow::DoubleBuilder builder;
                double double_default[] = {-1.0};
                ARROW_RETURN_NOT_OK(builder.AppendValues(double_default, 1));
                std::shared_ptr<arrow::Array> doubleColumn;
                ARROW_ASSIGN_OR_RAISE(doubleColumn, builder.Finish());
                data_vector.push_back(doubleColumn);
                break;
            }
            case arrow::Type::STRING:
            {
                arrow::StringBuilder builder;
                std::vector<std::string> string_default = {"string_default_XX"};
                ARROW_RETURN_NOT_OK(builder.AppendValues(string_default));
                std::shared_ptr<arrow::Array> stringColumn;
                ARROW_ASSIGN_OR_RAISE(stringColumn, builder.Finish());
                data_vector.push_back(stringColumn);
                break;
            }
            default:
                throw std::runtime_error("Unsupported data type !!!");
        } // switch
    }
    res = arrow::RecordBatch::Make(schema, 1, data_vector);
    return arrow::Status::OK();
}

arrow::Status ArraySlice(std::shared_ptr<arrow::Array> &target, std::shared_ptr<arrow::Array> &res, std::vector<int> &sel) {
    int size = sel.size();
    switch (target->type_id()) {
        case arrow::Type::INT32: {
            auto array = std::static_pointer_cast<arrow::Int32Array>(target);
            std::vector<int> data;
            for (int i = 0; i < size; i++) {
                data.push_back(array->Value(sel[i]));
            }
            arrow::Int32Builder builder;
            ARROW_RETURN_NOT_OK(builder.AppendValues(data));
            ARROW_ASSIGN_OR_RAISE(res, builder.Finish());
            return arrow::Status::OK();
        }
        case arrow::Type::FLOAT: {
            auto array = std::static_pointer_cast<arrow::FloatArray>(target);
            std::vector<float> data;
            for (int i = 0; i < size; i++) {
                data.push_back(array->Value(sel[i]));
            }
            arrow::FloatBuilder builder;
            ARROW_RETURN_NOT_OK(builder.AppendValues(data));
            ARROW_ASSIGN_OR_RAISE(res, builder.Finish());
            return arrow::Status::OK();
        }
        case arrow::Type::DOUBLE: {
            auto array = std::static_pointer_cast<arrow::DoubleArray>(target);
            std::vector<double> data;
            for (int i = 0; i < size; i++) {
                data.push_back(array->Value(sel[i]));
            }
            arrow::DoubleBuilder builder;
            ARROW_RETURN_NOT_OK(builder.AppendValues(data));
            ARROW_ASSIGN_OR_RAISE(res, builder.Finish());
            return arrow::Status::OK();
        }
        case arrow::Type::STRING: {
            auto array = std::static_pointer_cast<arrow::StringArray>(target);
            std::vector<std::string> data;
            for (int i = 0; i < size; i++) {
                data.push_back(array->GetString(sel[i]));
            }
            arrow::StringBuilder builder;
            ARROW_RETURN_NOT_OK(builder.AppendValues(data));
            ARROW_ASSIGN_OR_RAISE(res, builder.Finish());
            return arrow::Status::OK();
        }
        default:
            throw std::runtime_error("Invalid arrow::Type!!!");
    } // switch
}

arrow::Status RecordBatchSlice(std::shared_ptr<arrow::RecordBatch> &target, std::vector<int> &sel) {
    int size = target->num_rows();
    std::shared_ptr<arrow::Array> filter_arr;
    std::vector<bool> bool_arr(size, false); // 所有元素初始化为false
    for (int i = 0; i < sel.size(); i++) {
        bool_arr[sel[i]] = true;
    }
    arrow::BooleanBuilder builder;
    ARROW_RETURN_NOT_OK(builder.AppendValues(bool_arr));
    ARROW_ASSIGN_OR_RAISE(filter_arr, builder.Finish());
    arrow::Datum res;
    arrow::compute::FilterOptions filter_options;
    ARROW_ASSIGN_OR_RAISE(res, arrow::compute::CallFunction("filter", {target, filter_arr}, &filter_options));
    target = res.record_batch();
    return arrow::Status::OK();
}

arrow::Status RecordBatchSlice2(std::shared_ptr<arrow::RecordBatch> &target, std::shared_ptr<arrow::RecordBatch> &resp, std::vector<int> &sel) {
    int size = target->num_rows();
    auto cols = target->columns();
    std::vector<std::shared_ptr<arrow::Array>> col_array;
    for(int i = 0; i < target->num_columns(); i++) {
        auto col = cols[i];
        switch (col->type_id()) {
            case arrow::Type::INT32:
            {
                std::vector<int> arr;
                auto int32_array = std::static_pointer_cast<arrow::Int32Array>(col);
                int *raw_data = const_cast<int *>(int32_array->raw_values());
                for(int k = 0; k < sel.size(); k++) {
                    int val = raw_data[sel[k]];
                    arr.emplace_back(val);
                }
                arrow::Int32Builder int32builder;
                ARROW_RETURN_NOT_OK(int32builder.AppendValues(arr));
                std::shared_ptr<arrow::Array> tmp;
                ARROW_ASSIGN_OR_RAISE(tmp, int32builder.Finish());
                col_array.emplace_back(tmp);
                break;
            }
            case arrow::Type::FLOAT:
            {
                std::vector<float> arr;
                auto array = std::static_pointer_cast<arrow::FloatArray>(col);
                float *raw_data = const_cast<float *>(array->raw_values());
                for(int k = 0; k < sel.size(); k++) {
                    float val = raw_data[sel[k]];
                    arr.emplace_back(val);
                }
                arrow::FloatBuilder builder;
                ARROW_RETURN_NOT_OK(builder.AppendValues(arr));
                std::shared_ptr<arrow::Array> tmp;
                ARROW_ASSIGN_OR_RAISE(tmp, builder.Finish());
                col_array.emplace_back(tmp);
                break;
            }
            case arrow::Type::DOUBLE:
            {
                std::vector<double> arr;
                auto array = std::static_pointer_cast<arrow::DoubleArray>(col);
                double *raw_data = const_cast<double *>(array->raw_values());
                for(int k = 0; k < sel.size(); k++) {
                    double val = raw_data[sel[k]];
                    arr.emplace_back(val);
                }
                arrow::DoubleBuilder builder;
                ARROW_RETURN_NOT_OK(builder.AppendValues(arr));
                std::shared_ptr<arrow::Array> tmp;
                ARROW_ASSIGN_OR_RAISE(tmp, builder.Finish());
                col_array.emplace_back(tmp);
                break;
            }
            case arrow::Type::STRING:
            {
                std::vector<std::string> arr;
                auto array = std::static_pointer_cast<arrow::StringArray>(col);
                for(int k = 0; k < sel.size(); k++) {
                    arr.push_back(array->GetString(sel[k]));
                }
                arrow::StringBuilder builder;
                ARROW_RETURN_NOT_OK(builder.AppendValues(arr));
                std::shared_ptr<arrow::Array> tmp;
                ARROW_ASSIGN_OR_RAISE(tmp, builder.Finish());
                col_array.emplace_back(tmp);
                break;
            }
            default:
                throw std::runtime_error("Invalid arrow::Type!!!");
        } // switch
    }
    resp = arrow::RecordBatch::Make(target->schema(), sel.size(), col_array);
    return arrow::Status::OK();
}

arrow::Status RecordBatchSlice3(std::shared_ptr<arrow::RecordBatch> &target, std::vector<int> &sel) {
    auto cols = target->columns();
    std::vector<std::shared_ptr<arrow::Array>> col_array;
    for(int i = 0; i < target->num_columns(); i++) {
        auto col = cols[i];
        switch (col->type_id()) {
            case arrow::Type::INT32:
            {
                std::vector<int> arr;
                auto int32_array = std::static_pointer_cast<arrow::Int32Array>(col);
                int *raw_data = const_cast<int *>(int32_array->raw_values());
                for(int k = 0; k < sel.size(); k++) {
                    int val = raw_data[sel[k]];
                    arr.emplace_back(val);
                }
                arrow::Int32Builder int32builder;
                ARROW_RETURN_NOT_OK(int32builder.AppendValues(arr));
                std::shared_ptr<arrow::Array> tmp;
                ARROW_ASSIGN_OR_RAISE(tmp, int32builder.Finish());
                col_array.emplace_back(tmp);
                break;
            }
            case arrow::Type::FLOAT:
            {
                std::vector<float> arr;
                auto array = std::static_pointer_cast<arrow::FloatArray>(col);
                float *raw_data = const_cast<float *>(array->raw_values());
                for(int k = 0; k < sel.size(); k++) {
                    float val = raw_data[sel[k]];
                    arr.emplace_back(val);
                }
                arrow::FloatBuilder builder;
                ARROW_RETURN_NOT_OK(builder.AppendValues(arr));
                std::shared_ptr<arrow::Array> tmp;
                ARROW_ASSIGN_OR_RAISE(tmp, builder.Finish());
                col_array.emplace_back(tmp);
                break;
            }
            case arrow::Type::DOUBLE:
            {
                std::vector<double> arr;
                auto array = std::static_pointer_cast<arrow::DoubleArray>(col);
                double *raw_data = const_cast<double *>(array->raw_values());
                for(int k = 0; k < sel.size(); k++) {
                    double val = raw_data[sel[k]];
                    arr.emplace_back(val);
                }
                arrow::DoubleBuilder builder;
                ARROW_RETURN_NOT_OK(builder.AppendValues(arr));
                std::shared_ptr<arrow::Array> tmp;
                ARROW_ASSIGN_OR_RAISE(tmp, builder.Finish());
                col_array.emplace_back(tmp);
                break;
            }
            case arrow::Type::STRING:
            {
                std::vector<std::string> arr;
                auto array = std::static_pointer_cast<arrow::StringArray>(col);
                for(int k = 0; k < sel.size(); k++) {
                    arr.push_back(array->GetString(sel[k]));
                }
                arrow::StringBuilder builder;
                ARROW_RETURN_NOT_OK(builder.AppendValues(arr));
                std::shared_ptr<arrow::Array> tmp;
                ARROW_ASSIGN_OR_RAISE(tmp, builder.Finish());
                col_array.emplace_back(tmp);
                break;
            }
            default:
                throw std::runtime_error("Invalid arrow::Type!!!");
        } // switch
    }
    target = arrow::RecordBatch::Make(target->schema(), sel.size(), col_array);
    return arrow::Status::OK();
}

arrow::Status AppendValueArrayWithNull( std::shared_ptr<arrow::Array> &array, std::vector<std::shared_ptr<Value>> &col) {
    if(col.empty()) {
        return arrow::Status::OK();
    }
    arrow::Status ok;
    int size = col.size();
    switch (col[0]->type_) {
        case LogicalType::INTEGER:
        {
            std::vector<int> tmp(size);
            // 1有效，0为null
            std::vector<bool> is_valid(size, true);
            for(int i = 0; i < size; i++) {
                tmp[i] = col[i]->GetValueUnsafe<int>();
                if(col[i]->is_null) {
                    is_valid[i] = false;
                }
            }
            ok = AppendIntArrayWithNull(array, tmp, is_valid);

            break;
        }
        case LogicalType::FLOAT:
        {
            std::vector<float> tmp(size);
            std::vector<bool> is_valid(size, true);
            for(int i = 0; i < size; i++) {
                tmp[i] = col[i]->GetValueUnsafe<float>();
                if(col[i]->is_null) {
                    is_valid[i] = false;
                }
            }
            ok = AppendFloatArrayWithNull(array, tmp, is_valid);
            break;
        }
        case LogicalType::DOUBLE:
        {
            std::vector<double> tmp(size);
            std::vector<bool> is_valid(size, true);
            for(int i = 0; i < size; i++) {
                tmp[i] = col[i]->GetValueUnsafe<double>();
                if(col[i]->is_null) {
                    is_valid[i] = false;
                }
            }
            ok = AppendDoubleArrayWithNull(array, tmp, is_valid);
            break;
        }
        case LogicalType::STRING:
        {
            std::vector<std::string> tmp(size);
            std::vector<uint8_t> is_valid(size, true);
            for(int i = 0; i < size; i++) {
                tmp[i] = col[i]->GetValueUnsafe<std::string>();
                if(col[i]->is_null) {
                    is_valid[i] = 0;
                }
            }
            ok = AppendStringArrayWithNull(array, tmp, is_valid);
            break;
        }
        default:
            throw std::runtime_error("Invalid LogicalType::Type!!!");
    } // switch
    return ok;
}

// Sort
struct Element {
    std::shared_ptr<arrow::RecordBatch> value;
    std::vector<arrow::compute::SortKey> *sort_keys;
    std::vector<LogicalType> *types;
    int sequence_index; // 记录所属序列的索引
    Element(std::shared_ptr<arrow::RecordBatch> val, std::vector<arrow::compute::SortKey> *sort_keys, std::vector<LogicalType> *types, int idx) : value(val), sort_keys(sort_keys), types(types), sequence_index(idx) {}
};

struct Compare {
    bool operator()(const Element &left, const Element &right) const {
        return RecordBatchComparator(left.value, right.value, left.sort_keys, left.types);
    }
};

bool RecordBatchComparator(std::shared_ptr<arrow::RecordBatch> left, std::shared_ptr<arrow::RecordBatch> right, const std::vector<arrow::compute::SortKey> *sort_keys, const std::vector<LogicalType> *types) {
    int size = sort_keys->size();
    bool res = false;
    for(int i = 0; i < size; i++) {
        int column_index = (*sort_keys)[i].target.field_path()->indices()[0];

        std::shared_ptr<arrow::Array> left_p = left->column(column_index);
        std::shared_ptr<arrow::Array> right_p = right->column(column_index);
        LogicalType type_p = (*types)[i];
        arrow::compute::SortOrder order_type = (*sort_keys)[i].order;
        switch (type_p) {
            case LogicalType::INTEGER :
            {
                int left_k = std::static_pointer_cast<arrow::Int32Array>(left_p)->Value(0);
                int right_k = std::static_pointer_cast<arrow::Int32Array>(right_p)->Value(0);
                if(left_k != right_k) {
                    return ((order_type == arrow::compute::SortOrder::Descending) ? (left_k < right_k) : (left_k > right_k));
                }
                break;
            }
            case LogicalType::FLOAT :
            {
                float left_k = std::static_pointer_cast<arrow::FloatArray>(left_p)->Value(0);
                float right_k = std::static_pointer_cast<arrow::FloatArray>(right_p)->Value(0);
                if(left_k != right_k) {
                    return ((order_type == arrow::compute::SortOrder::Descending) ? (left_k < right_k) : (left_k > right_k));
                }
                break;
            }
            case LogicalType::DOUBLE :
            {
                double left_k = std::static_pointer_cast<arrow::DoubleArray>(left_p)->Value(0);
                double right_k = std::static_pointer_cast<arrow::DoubleArray>(right_p)->Value(0);
                if(left_k != right_k) {
                    return ((order_type == arrow::compute::SortOrder::Descending) ? (left_k < right_k) : (left_k > right_k));
                }
                break;
            }
            case LogicalType::STRING :
            {
                std::string left_k = std::static_pointer_cast<arrow::StringArray>(left_p)->GetString(0);
                std::string right_k = std::static_pointer_cast<arrow::StringArray>(right_p)->GetString(0);
                if(left_k != right_k) {
                    return ((order_type == arrow::compute::SortOrder::Descending) ? (left_k < right_k) : (left_k > right_k));
                }
                break;
            }
            default:
                break;
        } // switch
    } // for
    return res;
}

void MergeRecordBatch(std::vector<std::shared_ptr<arrow::RecordBatch>> &tar, std::vector<std::shared_ptr<arrow::UInt64Array>> &tar_idx, std::vector<std::shared_ptr<arrow::RecordBatch>> &result, std::vector<arrow::compute::SortKey> *sort_keys, std::vector<LogicalType> *types) {
    int size = tar.size();
    std::vector<int> tar_size;
    std::vector<int> slice_idx(size, 1);
    std::priority_queue<Element, std::vector<Element>, Compare> minHeap;
    // 初始化优先队列，将每个序列的第一个元素放入队列中
    for(int i = 0; i < size; i++) {
        if(tar[i] != nullptr && tar[i]->num_rows() != 0) {
            uint64_t offset = tar_idx[i]->Value(0);
            if (offset < tar[i]->num_rows()) {
                auto slice = tar[i]->Slice(offset, 1);
                minHeap.emplace(Element(slice, sort_keys, types, i));
            }
        }
    }
    for(int i = 0; i < size; i++) {
        if(tar[i] == nullptr) {
            tar_size.push_back(0);
        } else {
            tar_size.push_back(tar[i]->num_rows());
        }
    }
    // 从优先队列中不断取出最小元素，将其放入结果数组中，并将其所属序列的下一个元素放入队列中
    while (!minHeap.empty()) {
        Element minElement = minHeap.top();
        minHeap.pop();
        result.push_back(minElement.value);
        int nextIndex = minElement.sequence_index;
        if (slice_idx[nextIndex] < tar_size[nextIndex]) {
            int &idx = slice_idx[nextIndex];
            uint64_t offset = tar_idx[nextIndex]->Value(idx++);
            minHeap.emplace(Element(tar[nextIndex]->Slice(offset, 1), sort_keys, types, nextIndex));
        }
    }
}

arrow::Status WriteToCSVFromRecordBatch(std::vector<std::shared_ptr<arrow::RecordBatch>> &rbatchs, std::string &filePath) {
    std::shared_ptr<arrow::Table> csv_table = arrow::Table::FromRecordBatches(rbatchs).ValueOrDie();
    std::shared_ptr<arrow::io::FileOutputStream> outfile;
    ARROW_ASSIGN_OR_RAISE(outfile, arrow::io::FileOutputStream::Open(filePath));
    ARROW_RETURN_NOT_OK(arrow::csv::WriteCSV(
            *csv_table, arrow::csv::WriteOptions::Defaults(), outfile.get()));
    return arrow::Status::OK();
}

std::vector<int> MergeTwoSortedArrays(std::vector<int>& arr1, std::vector<int>& arr2) {
    int arr1_size = arr1.size();
    int arr2_size = arr2.size();
    if(arr1_size == 0) {
        return arr2;
    }
    if(arr2_size == 0) {
        return arr1;
    }
    std::vector<int> merged;
    size_t i = 0, j = 0;
    // Traverse both arrays and insert smaller element from arr1 or arr2 into merged array
    while (i < arr1_size && j < arr2_size) {
        if (arr1[i] < arr2[j]) {
            merged.push_back(arr1[i++]);
        } else if (arr1[i] == arr2[j]) {
            merged.push_back(arr1[i++]);
            j++;
        } else {
            merged.push_back(arr2[j++]);
        }
    }

    while (i < arr1.size()) {
        merged.push_back(arr1[i++]);
    }

    while (j < arr2.size()) {
        merged.push_back(arr2[j++]);
    }
    return merged;
}

void GetSetDifference(std::vector<int> &res, int count, std::vector<int> &tar) {
    std::vector<int> res_tmp(count);
    for(int i = 0; i < count; i++) {
        res_tmp[i] = i;
    }
    std::sort(tar.begin(), tar.end());
    std::set_difference(res_tmp.begin(), res_tmp.end(), tar.begin(), tar.end(), std::back_inserter(res));
}

arrow::Status OpenParquetFile(
	const std::string &file_path,
	std::shared_ptr<arrow::Schema> &schema_target,
	std::unique_ptr<parquet::arrow::FileReader> &reader_ret) {
	// 打开文件输入流
	std::shared_ptr<arrow::io::ReadableFile> infile;
	ARROW_ASSIGN_OR_RAISE(infile, arrow::io::ReadableFile::Open(file_path));

	// 创建 Parquet 文件读取器
	std::unique_ptr<parquet::arrow::FileReader> reader;
	ARROW_RETURN_NOT_OK(parquet::arrow::OpenFile(
		infile, arrow::default_memory_pool(), &reader));

	// 获取 Parquet 文件的 schema，并检查
	std::shared_ptr<arrow::Schema> schema;
	ARROW_RETURN_NOT_OK(reader->GetSchema(&schema));
	if (schema->Equals(schema_target) == false) {
		return arrow::Status::Invalid("schema inconsistency");
	}

    reader_ret = std::move(reader);

	return arrow::Status::OK();
}

arrow::Status ReadRecordBatchFromReader(
	std::unique_ptr<parquet::arrow::FileReader> &reader,
	const std::vector<int> &column_indices,
    std::shared_ptr<arrow::Schema> &schema,
	std::shared_ptr<arrow::RecordBatch> &batches_ret) {
    
	std::vector<std::shared_ptr<arrow::Field>> first_column_fields;
	for (auto col_i : column_indices) {
		if (col_i >= schema->num_fields()) {
			return arrow::Status::Invalid("column index out of range");
		}
		first_column_fields.push_back(schema->field(col_i));
	}

	// 确保 schema 至少有一列
	if (schema->num_fields() == 0 || first_column_fields.size() == 0) {
		return arrow::Status::Invalid("Parquet file has no columns");
	}
	auto filtered_schema = std::make_shared<arrow::Schema>(first_column_fields);

	std::shared_ptr<arrow::Table> table;
	ARROW_RETURN_NOT_OK(reader->ReadTable(column_indices, &table));

	// 将 Table 转换为 RecordBatch 列表
	batches_ret =
		table->CombineChunksToBatch(arrow::default_memory_pool()).ValueOrDie();

	return arrow::Status::OK();
}

arrow::Status ReadRecordBatchFromDisk(
	const std::string &file_path, const std::vector<int> &column_indices,
	std::shared_ptr<arrow::Schema> &schema_target,
	std::shared_ptr<arrow::RecordBatch> &batches) {
	// 打开文件输入流
	std::shared_ptr<arrow::io::ReadableFile> infile;
	ARROW_ASSIGN_OR_RAISE(infile, arrow::io::ReadableFile::Open(file_path));

	// 创建 Parquet 文件读取器
	std::unique_ptr<parquet::arrow::FileReader> reader;
	ARROW_RETURN_NOT_OK(parquet::arrow::OpenFile(
		infile, arrow::default_memory_pool(), &reader));

	// 获取 Parquet 文件的 schema，并检查
	std::shared_ptr<arrow::Schema> schema;
	ARROW_RETURN_NOT_OK(reader->GetSchema(&schema));
	if (schema->Equals(schema_target) == false) {
		return arrow::Status::Invalid("schema inconsistency");
	}

	std::vector<std::shared_ptr<arrow::Field>> first_column_fields;
	for (auto col_i : column_indices) {
		if (col_i >= schema->num_fields()) {
			return arrow::Status::Invalid("column index out of range");
		}
		first_column_fields.push_back(schema->field(col_i));
	}

	// 确保 schema 至少有一列
	if (schema->num_fields() == 0 || first_column_fields.size() == 0) {
		return arrow::Status::Invalid("Parquet file has no columns");
	}
	auto filtered_schema = std::make_shared<arrow::Schema>(first_column_fields);

	std::shared_ptr<arrow::Table> table;
	ARROW_RETURN_NOT_OK(reader->ReadTable(column_indices, &table));

	// 将 Table 转换为 RecordBatch 列表
	batches =
		table->CombineChunksToBatch(arrow::default_memory_pool()).ValueOrDie();

	return arrow::Status::OK();
}

// 这里要改成从磁盘上读取数据。
arrow::Status WriteRecordBatchToDisk(
	const std::shared_ptr<arrow::RecordBatch> &batch,
	const std::string &file_path) {
	// 打开文件输出流
	std::shared_ptr<arrow::io::FileOutputStream> outfile;
	ARROW_ASSIGN_OR_RAISE(outfile,
						  arrow::io::FileOutputStream::Open(file_path));

	// 创建 Parquet 写入器
	std::shared_ptr<parquet::arrow::FileWriter> writer;
	ARROW_ASSIGN_OR_RAISE(
		writer, parquet::arrow::FileWriter::Open(
					*batch->schema(), arrow::default_memory_pool(), outfile));

	// 写入 RecordBatch
	ARROW_RETURN_NOT_OK(writer->WriteRecordBatch(*batch));

	// 关闭写入器
	ARROW_RETURN_NOT_OK(writer->Close());
	ARROW_RETURN_NOT_OK(outfile->Close());

	return arrow::Status::OK();
}

// 比较两个 RecordBatchVector 是否严格相等
bool AreRecordBatchVectorsEqual(const arrow::RecordBatchVector &vec1,
								const arrow::RecordBatchVector &vec2,
								bool check_metadata) {
	// 检查向量大小
	if (vec1.size() != vec2.size()) {
		std::cerr << "Vectors have different sizes: " << vec1.size() << " vs "
				  << vec2.size() << "\n";
		return false;
	}

	// 逐个比较 RecordBatch
	for (size_t i = 0; i < vec1.size(); ++i) {
		if (!vec1[i]->Equals(*vec2[i], check_metadata)) {
			std::cerr << "RecordBatch at index " << i << " differs\n";
			return false;
		}
	}

	return true;
}

} // namespace Util
} // namespace DaseX
