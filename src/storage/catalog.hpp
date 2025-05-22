#pragma once

#include "config.hpp"
#include "logical_type.hpp"
#include "rc.hpp"
#include "common.hpp"
#include "arrow_help.hpp"
#include <arrow/api.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <numa.h>
#include <sched.h>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <leveldb/db.h>

namespace DaseX {

class Table;
class CataLog;

class ColumnChunk {
public:
  std::shared_ptr<arrow::Array> data;

public:
  ColumnChunk() = default;

  ColumnChunk(std::shared_ptr<arrow::Array> data_) : data(std::move(data_)){};

  arrow::Status append_int_array(std::vector<int> &array) {
    arrow::Int32Builder int32builder;
    ARROW_RETURN_NOT_OK(int32builder.AppendValues(array));
    ARROW_ASSIGN_OR_RAISE(data, int32builder.Finish());
    return arrow::Status::OK();
  }

  RC append_int_data(std::vector<int> &array) {
    arrow::Status st = append_int_array(array);
    RC rc = RC::FAILED;
    if (st.ok()) {
      rc = RC::SUCCESS;
    }
    return rc;
  }

  arrow::Status append_float_array(std::vector<float> &array);

  arrow::Status append_float_data(std::vector<float> &array);

  arrow::Status append_double_array(std::vector<double> &array);

  arrow::Status append_double_data(std::vector<double> &array);

  arrow::Status append_string_array(std::vector<std::string> &array) {
    arrow::StringBuilder stringbuilder;
    ARROW_RETURN_NOT_OK(stringbuilder.AppendValues(array));
    ARROW_ASSIGN_OR_RAISE(data, stringbuilder.Finish());
    return arrow::Status::OK();
  }

  RC append_string_data(std::vector<std::string> &array) {
    arrow::Status st = append_string_array(array);
    RC rc = RC::FAILED;
    if (st.ok()) {
      rc = RC::SUCCESS;
    }
    return rc;
  }

  int get_column_chunk_size() const { return data->length(); }

  bool is_null(int64_t i) const { return data->IsNull(i); }

  std::shared_ptr<arrow::Array> get_data() { return data; }
};

class Column {
public:
  std::string colunm_name;
  std::vector<std::shared_ptr<ColumnChunk>> chunks;
  int chunk_nums = 0;
  std::mutex mutex_colunm;

public:
  Column(std::string colunm_name_) : colunm_name(colunm_name_) {}

  void insert_data(std::shared_ptr<ColumnChunk> chunk) {
    std::lock_guard<std::mutex> lock(mutex_colunm);
    chunks.emplace_back(chunk);
    chunk_nums++;
  }

  std::shared_ptr<ColumnChunk> get_data(int idx) const {
    if (idx > chunk_nums) {
      throw std::runtime_error("Out of Range");
    }
    return chunks[idx];
  }

  int get_chunk_nums() const { return chunk_nums; };
};

class Field {
public:
  std::string field_name;
  LogicalType type;
  std::shared_ptr<arrow::Field> field;

public:
  Field(std::string field_name_, LogicalType type_)
      : field_name(field_name_), type(type_) {
    switch (type) {
    case LogicalType::INTEGER:
      field = arrow::field(field_name, arrow::int32());
      break;
    case LogicalType::FLOAT:
      field = arrow::field(field_name, arrow::float32());
      break;
    case LogicalType::DOUBLE:
      field = arrow::field(field_name, arrow::float64());
      break;
    case LogicalType::STRING:
      field = arrow::field(field_name, arrow::utf8());
      break;
//	case LogicalType::VECTOR:
//		field = arrow::field(field_name,arrow::binary());
    default:
      break;
    }
  }

  std::string Tostring() { return field->ToString(); }
};

class TableInfo {
public:
  std::string table_name;
  std::vector<std::string> filed_names;
  std::vector<LogicalType> filed_types;
  int column_nums = 0;
  int partition_num = 0;

public:
  TableInfo() = default;

  TableInfo(const std::string &table_name_,
            std::vector<std::string> &filed_names_,
            std::vector<LogicalType> &filed_types_,
            int partition_num_ = Numa::CORES)
      : table_name(table_name_), filed_names(filed_names_),
        filed_types(filed_types_), partition_num(partition_num_) {
    column_nums = filed_names.size();
  }
};

class RecordBatchReader {
   public:
	RecordBatchReader(int64_t file_number, std::string file_path,
					  int64_t row_num,
					  std::unique_ptr<parquet::arrow::FileReader> reader)
		: file_number_(file_number),
		  row_num_(row_num),
		  file_path_(file_path),
		  reader_(std::move(reader)) {}

	// Delete copy operations
	RecordBatchReader(const RecordBatchReader &) = delete;
	RecordBatchReader &operator=(const RecordBatchReader &) = delete;

	// Move constructor
	RecordBatchReader(RecordBatchReader &&other) noexcept
		: file_number_(other.file_number_),
		  row_num_(other.row_num_),
		  file_path_(std::move(other.file_path_)),
		  reader_(std::move(other.reader_)) {}

	int64_t file_number_;
	int row_num_;
	std::string file_path_;
	std::unique_ptr<parquet::arrow::FileReader> reader_;
};

class PartitionedTable {
public:
    std::vector<std::shared_ptr<Field>> fileds;
    std::shared_ptr<arrow::Schema> schema;
    std::vector<std::shared_ptr<Column>> columns;
    std::vector<RecordBatchReader> record_batch_readers;
    int chunk_nums = 0;  // 列块
    int column_nums = 0; // 列数
    int row_nums = 0;    // 分区表共有多少行数据
    int partition_id;
    Table *table;
    std::mutex mutex_table;

public:
    PartitionedTable(int partition_id_, Table *table_) : partition_id(partition_id_), 
      table(table_) {}

    /// 从元数据（CataLog）中获取TableInfo,用从元数据中获取TableInfo初始化fileds +
    /// schema + columns
    void initailize(const TableInfo &table_info) {
        column_nums = table_info.column_nums;
        if (column_nums == 0) {
            throw std::runtime_error("Table is empty!!!");
        }
        arrow::FieldVector filed_vector;
        for (int i = 0; i < column_nums; i++) {
            fileds.push_back(std::move(std::make_shared<Field>(
                    table_info.filed_names[i], table_info.filed_types[i])));
            filed_vector.emplace_back(fileds[i]->field);
            columns.emplace_back(std::make_shared<Column>(table_info.filed_names[i]));
        }
        schema = arrow::schema(filed_vector);
    } // initailize_table

    std::shared_ptr<arrow::RecordBatch>
    get_data(int idx, const std::vector<int> &project_ids = std::vector<int>());

    RC insert_data(std::shared_ptr<arrow::RecordBatch> &record_batch);

    RC insert_data(RecordBatchReader &record_batch_reader);

    /// 获取分区所在的numa节点
    int get_socket() { return Numa::get_socket(partition_id); }

    /// FOR DEBUG
    void print_table() {
        std::cout << "==============================partition" << partition_id
                  << std::endl;
        int chunk_nums = columns[0]->chunk_nums;

        for (int i = 0; i < chunk_nums; i++) {
            std::shared_ptr<arrow::RecordBatch> rbatch = get_data(i);
            std::cout << "=======================chunk_nums" << i << std::endl;
            std::cout << rbatch->ToString() << std::endl;
        }
    }
};

class Table {
public:
  std::string db = "db_314";
  std::string table_name;
  std::vector<std::shared_ptr<Field>> fileds;
  std::shared_ptr<arrow::Schema> schema;
  std::vector<std::shared_ptr<PartitionedTable>> partition_tables;
  CataLog *cataLog;

  int partition_num;                 // 逻辑分区数
  int numa_num;                      // numa节点的个数
  int column_nums = 0;               // 列数
  int row_nums = 0;                  // 表共有多少行数据
  std::vector<int> partition_bitmap; // 分区有数据为1，没有数据为0
public:
  Table(TableInfo &table_info_) { initailize(table_info_); }

  void initailize(TableInfo &table_info) {
    table_name = table_info.table_name;
    partition_num = table_info.partition_num;
    for (int i = 0; i < partition_num; ++i) {
      partition_tables.push_back(std::make_shared<PartitionedTable>(i, this));
      partition_tables[i]->initailize(table_info);
      partition_bitmap.push_back(0);
    }

    arrow::FieldVector filed_vector;
    column_nums = table_info.column_nums;
    for (int i = 0; i < column_nums; i++) {
      fileds.push_back(std::move(std::make_shared<Field>(
          table_info.filed_names[i], table_info.filed_types[i])));
      filed_vector.push_back(fileds[i]->field);
    }
    schema = arrow::schema(filed_vector);
  } // initailize_table

  // 第一次导入数据可以使用这个接口
  RC insert_data(std::shared_ptr<arrow::RecordBatch> record_batch,
                 int partition_id) {
    RC rc = RC::FAILED;
    if (record_batch->schema()->Equals(schema)) {
      if (partition_id >= partition_num) {
        return rc;
      }
      if (record_batch->num_rows() == 0 || record_batch->num_columns() == 0) {
        rc = RC::INVALID_DATA;
        return rc;
      }

      partition_tables[partition_id]->insert_data(record_batch);
      partition_bitmap.at(partition_id) = 1;
      row_nums += record_batch->num_rows();
      rc = RC::SUCCESS;
    }
    return rc;
  } // insert_data

  RC insert_data(RecordBatchReader &record_batch_reader, int partition_id);
  void print_table() {
    for (int i = 0; i < partition_num; ++i) {
      if (partition_bitmap[i]) {
        partition_tables[i]->print_table();
      }
    }
  }
};



class CataLog {
public:
  // std::unordered_map<std::string, TableInfo> table_infos;   // 弃用
  std::unordered_map<std::string, std::shared_ptr<Table>> tables;  // 作为cache
  std::mutex mutex_catalog;
  const std::string  database_path_;

public:
  CataLog();

  CataLog(std::string database_path, leveldb::DB* meta_database);

  ~CataLog();

  RC register_table(TableInfo &table_info);

  RC delete_table(std::string table_name);

  RC get_table(std::string table_name, std::shared_ptr<Table> &table);

  RC load_from_single_file(std::string& table_name,
        std::shared_ptr<Table>& table,
        int partition_idx,
        std::string& file_name,
        int chunk_size,
        char separator);

  RC load_from_files(std::string& table_name,
        std::shared_ptr<Table>& table,
        std::vector<int>& work_ids,
        std::vector<int>& partition_idxs,
        std::vector<std::string>& file_names,
        int chunk_size,
        char separator);
  // 获取下一个文件号并递增
  int64_t get_next_file_number();
private:
 friend class PartitionedTable;
 friend class Table;
  leveldb::DB* meta_database_;  // 所有的元信息都会从这里面读取
};

RC OpenDatabase(std::string database_path, 
  std::shared_ptr<CataLog> &catalog, 
  bool create_if_missing = true
);

// 删除数据库
// 必须在数据库关闭的情况下执行
RC DeleteDatabase(std::string database_path);

// NOTE: 为了兼容老代码，暂时不删除这个
extern std::shared_ptr<CataLog> global_catalog;

// 目前很多要用到catalog的地方根本没有地方把catalog传进去，
// 为了方便获取当前用OpenDatabase打开的数据库，先用这个全局变量
extern std::weak_ptr<CataLog> open_catalog_ptr;

} // namespace DaseX