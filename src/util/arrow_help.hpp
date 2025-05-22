#pragma once

#include "logical_type.hpp"
#include "value_type.hpp"
#include "expression_type.hpp"
#include <arrow/api.h>
#include <arrow/compute/api.h>
#include <arrow/csv/api.h>
#include <arrow/csv/writer.h>
#include <arrow/io/api.h>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <arrow/ipc/api.h>
#include <parquet/arrow/writer.h>
#include <parquet/arrow/reader.h>

namespace DaseX
{
namespace Util
{

/**
 * @description:
 * Join两个RecordBatch，并将结果保存到right中(RecordBatch只有一行数据)
 * @param {shared_ptr<arrow::RecordBatch>&} left    Join构建端元素
 * @param {int16_t} left_col_nums                   Join构建端列数
 * @param {int16_t} left_col                        Join构建端列
 * @param {shared_ptr<arrow::RecordBatch>&} right   Join探测端元素
 * @param {int16_t} right_col_nums                  Join探测端列数
 * @param {int16_t} right_col                       Join探测端列
 */
void join_two_record_batch(std::shared_ptr<arrow::RecordBatch> &left,
                           int left_col_nums, int left_col,
                           std::shared_ptr<arrow::RecordBatch> &right,
                           int right_col_nums, int right_col,
                           std::shared_ptr<arrow::RecordBatch> &res);

/**
 * @description:
 * 比较两个RecordBatch是否匹配(RecordBatch只有一行数据)，用于NestedLoopJoin比较两行数据
 * @param {shared_ptr<arrow::RecordBatch>&} left            左边行
 * @param {shared_ptr<arrow::RecordBatch>&} right           右边行
 * @param {std::vector<int>&} left_ids                      左边连接键所在的列号
 * @param {std::vector<int>&} right_ids                     右边连接键所在的列号
 * @param {std::vector<ExpressionTypes>&} compare_types     每个连接键的比较类型，例如： >, <, =, <>, >=, <=
 * @return {*} 如果匹配，返回true，否则返回false
 */
bool MatchTwoRecordBatch(std::shared_ptr<arrow::RecordBatch> &left,
                         std::shared_ptr<arrow::RecordBatch> &right,
                         const std::vector<int> &left_ids,
                         const std::vector<int> &right_ids,
                         const std::vector<ExpressionTypes> &compare_types);

std::shared_ptr<arrow::Schema> MergeTwoSchema(std::shared_ptr<arrow::Schema> &left, std::shared_ptr<arrow::Schema> &right);

// 返回只有一行值的RB，并且值全部为NULL
arrow::Status GetSinlgeRowNull(std::shared_ptr<arrow::RecordBatch> &res, std::shared_ptr<arrow::Schema> &schema);

// 返回只有一行值的RB，并且值全部为默认值
arrow::Status GetSinlgeRow(std::shared_ptr<arrow::RecordBatch> &res, std::shared_ptr<arrow::Schema> &schema);
/**
 * @description:
 * @param {shared_ptr<arrow::Array>} &target  需要切片的列
 * @param {vector<int>} &sel                  索引数组
 * @return {*}    返回新列，保留sel中的数据；例如， target = [5，1，4，3]  sel = [2] 则 res = [4]   sel不能为null或size为0。
 */
arrow::Status ArraySlice(std::shared_ptr<arrow::Array> &target, std::shared_ptr<arrow::Array> &res, std::vector<int> &sel);

/**
 * @description:  根据传入的sel切片target，保留sel中索引的数据。
 * 例如：
 *  before:            a ,    b                             after:
 *                     _________
 *                     | 1 | a |                                         _________
 *                     | 2 | b |                                         | 1 | a |
 *          targer =   | 3 | c |     sel = [0, 2, 4]  =====>   targer =  | 3 | c |
 *                     | 4 | d |                                         |_5_|_e_|
 *                     |_5_|_e_|
 *
 * 注意：sel不能为null或size为0
 * @param {shared_ptr<arrow::RecordBatch>} &target   需要切片的RecordBatch
 * @param {vector<int>} &sel                         切片索引
 * @return {*}
 */
arrow::Status RecordBatchSlice(std::shared_ptr<arrow::RecordBatch> &target, std::vector<int> &sel);

arrow::Status RecordBatchSlice2(std::shared_ptr<arrow::RecordBatch> &target, std::shared_ptr<arrow::RecordBatch> &resp, std::vector<int> &sel);

arrow::Status RecordBatchSlice3(std::shared_ptr<arrow::RecordBatch> &target, std::vector<int> &sel);

inline arrow::Status AppendIntArray(std::shared_ptr<arrow::Array> &array, std::vector<int> &col) {
    arrow::Int32Builder int32builder;
    ARROW_RETURN_NOT_OK(int32builder.AppendValues(col));
    ARROW_ASSIGN_OR_RAISE(array, int32builder.Finish());
    return arrow::Status::OK();
}

inline arrow::Status AppendFloatArray(std::shared_ptr<arrow::Array> &array, std::vector<float> &col) {
arrow::FloatBuilder floatbuilder;
ARROW_RETURN_NOT_OK(floatbuilder.AppendValues(col));
ARROW_ASSIGN_OR_RAISE(array, floatbuilder.Finish());
return arrow::Status::OK();
}

inline arrow::Status AppendDoubleArray(std::shared_ptr<arrow::Array> &array, std::vector<double> &col) {
    arrow::DoubleBuilder doublebuilder;
    ARROW_RETURN_NOT_OK(doublebuilder.AppendValues(col));
    ARROW_ASSIGN_OR_RAISE(array, doublebuilder.Finish());
    return arrow::Status::OK();
}

inline arrow::Status AppendStringArray(std::shared_ptr<arrow::Array> &array, std::vector<std::string> &col) {
    arrow::StringBuilder stringbuilder;
    ARROW_RETURN_NOT_OK(stringbuilder.AppendValues(col));
    ARROW_ASSIGN_OR_RAISE(array, stringbuilder.Finish());
    return arrow::Status::OK();
}

inline arrow::Status AppendValueArray( std::shared_ptr<arrow::Array> &array, std::vector<std::shared_ptr<Value>> &col) {
    if(col.empty()) {
        return arrow::Status::OK();
    }
    arrow::Status ok;
    int size = col.size();
    switch (col[0]->type_) {
        case LogicalType::INTEGER:
        {
            std::vector<int> tmp(size);
            for(int i = 0; i < size; i++) {
                tmp[i] = col[i]->GetValueUnsafe<int>();
            }
            ok = AppendIntArray(array, tmp);
            break;
        }
        case LogicalType::FLOAT:
        {
            std::vector<float> tmp(size);
            for(int i = 0; i < size; i++) {
                tmp[i] = col[i]->GetValueUnsafe<float>();
            }
            ok = AppendFloatArray(array, tmp);
            break;
        }
        case LogicalType::DOUBLE:
        {
            std::vector<double> tmp(size);
            for(int i = 0; i < size; i++) {
                tmp[i] = col[i]->GetValueUnsafe<double>();
            }
            ok = AppendDoubleArray(array, tmp);
            break;
        }
        case LogicalType::STRING:
        {
            std::vector<std::string> tmp(size);
            for(int i = 0; i < size; i++) {
                tmp[i] = col[i]->GetValueUnsafe<std::string>();
            }
            ok = AppendStringArray(array, tmp);
            break;
        }
        default:
            throw std::runtime_error("Invalid LogicalType::Type!!!");
    } // switch
    return ok;
}

inline arrow::Status AppendIntArrayWithNull( std::shared_ptr<arrow::Array> &array, std::vector<int> &col, std::vector<bool> &is_valid) {
    arrow::Int32Builder int32builder;
    ARROW_RETURN_NOT_OK(int32builder.AppendValues(col, is_valid));
    ARROW_ASSIGN_OR_RAISE(array, int32builder.Finish());
    return arrow::Status::OK();
}

inline arrow::Status AppendFloatArrayWithNull( std::shared_ptr<arrow::Array> &array, std::vector<float> &col, std::vector<bool> &is_valid) {
    arrow::FloatBuilder floatbuilder;
    ARROW_RETURN_NOT_OK(floatbuilder.AppendValues(col, is_valid));
    ARROW_ASSIGN_OR_RAISE(array, floatbuilder.Finish());
    return arrow::Status::OK();
}

inline arrow::Status AppendDoubleArrayWithNull( std::shared_ptr<arrow::Array> &array, std::vector<double> &col, std::vector<bool> &is_valid) {
    arrow::DoubleBuilder doublebuilder;
    ARROW_RETURN_NOT_OK(doublebuilder.AppendValues(col, is_valid));
    ARROW_ASSIGN_OR_RAISE(array, doublebuilder.Finish());
    return arrow::Status::OK();
}

inline arrow::Status AppendStringArrayWithNull( std::shared_ptr<arrow::Array> &array, std::vector<std::string> &col, std::vector<uint8_t> &is_valid) {
    arrow::StringBuilder stringbuilder;
    ARROW_RETURN_NOT_OK(stringbuilder.AppendValues(col, is_valid.data()));
    ARROW_ASSIGN_OR_RAISE(array, stringbuilder.Finish());
    return arrow::Status::OK();
}

arrow::Status AppendValueArrayWithNull( std::shared_ptr<arrow::Array> &array, std::vector<std::shared_ptr<Value>> &col);

// Sort
bool RecordBatchComparator(std::shared_ptr<arrow::RecordBatch> left, std::shared_ptr<arrow::RecordBatch> right, const std::vector<arrow::compute::SortKey> *sort_keys, const std::vector<LogicalType> *types);

void MergeRecordBatch(std::vector<std::shared_ptr<arrow::RecordBatch>> &tar, std::vector<std::shared_ptr<arrow::UInt64Array>> &tar_idx, std::vector<std::shared_ptr<arrow::RecordBatch>> &result, std::vector<arrow::compute::SortKey> *sort_keys, std::vector<LogicalType> *types);

// 将RB数据写入CSV中，方便查看结果，用于调试目的
arrow::Status WriteToCSVFromRecordBatch(std::vector<std::shared_ptr<arrow::RecordBatch>> &source, std::string &fileName);

// 合并两个有序数组
std::vector<int> MergeTwoSortedArrays(std::vector<int>& arr1, std::vector<int>& arr2);

// 获取两个集合的差集
void GetSetDifference(std::vector<int> &res, int count, std::vector<int> &tar);

arrow::Status OpenParquetFile(
	const std::string &file_path,
	std::shared_ptr<arrow::Schema> &schema_target,
	std::unique_ptr<parquet::arrow::FileReader> &reader_ret);

arrow::Status ReadRecordBatchFromReader(
	std::unique_ptr<parquet::arrow::FileReader> &reader,
	const std::vector<int> &column_indices,
	std::shared_ptr<arrow::Schema> &schema,
	std::shared_ptr<arrow::RecordBatch> &batches_ret);

arrow::Status WriteRecordBatchToDisk(const std::shared_ptr<arrow::RecordBatch>& batch, const std::string& file_path);

arrow::Status ReadRecordBatchFromDisk(const std::string& file_path, const std::vector<int> &column_indices, 
    std::shared_ptr<arrow::Schema> &schema_target, std::shared_ptr<arrow::RecordBatch> &batches);

bool AreRecordBatchVectorsEqual(const arrow::RecordBatchVector &vec1,
								const arrow::RecordBatchVector &vec2,
								bool check_metadata = false);

} // namespace Util
} // namespace DaseX
