#include "catalog.hpp"
#include "scheduler.hpp"
#include "spdlog/spdlog.h"
#include "arrow_help.hpp"
#include "time_help.hpp"

#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <fstream>
#include <any>
#include <sstream>
#include <filesystem>

namespace DaseX {

namespace {

std::mutex mtx;
std::condition_variable cv;
std::atomic<int> counter(0);

template<typename T>
std::vector<T> convert_any_vector_to(const std::vector<std::any>& anyVec) {
    std::vector<T> result;
    result.reserve(anyVec.size()); // 预分配空间以提高效率
    for (size_t i = 0; i < anyVec.size(); ++i) {
        if (anyVec[i].type() == typeid(T)) {
            result.push_back(std::any_cast<T>(anyVec[i]));
        } else {
            throw std::runtime_error("Type mismatch at index " + std::to_string(i) + 
                                     ": expected " + typeid(T).name() + 
                                     ", got " + anyVec[i].type().name());
        }
    }

    return result;
}

RC remove_directory(const std::string& path) {
    try {
        if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
            std::filesystem::remove_all(path);
            spdlog::info("目录 {} 已删除.", path);
            return RC::SUCCESS;
        } else {
            return RC::IO_ERROR;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        spdlog::error("目录 {} 时出错: {}", path, e.what());
        return RC::IO_ERROR;
    }
}

RC load_from_single_file_func(std::string& table_name,
								  std::shared_ptr<Table>& table,
								  int partition_idx, 
                                  std::string& file_name,
                                  int chunk_size,
                                  char separator) {
    auto rc = RC::FAILED;

    std::ifstream infile(file_name);
    std::string line;
    if (!infile.is_open()) {
        std::cerr << "无法打开文件: " << file_name << std::endl;
        return RC::IO_ERROR;
    }

    auto start_execute = std::chrono::steady_clock::now();

    // 文件预读取的数据先存到一个临时数组中，后面一起打包
    // 目前只支持int、float、double、string
    auto fields = table->schema->fields();
    std::vector<std::vector<std::any>> temp_array;
    temp_array.resize(fields.size());
    for (size_t i = 0; i < fields.size(); i++){
        temp_array[i].reserve(chunk_size);
    }

    int count = 0;
    int round = 0;
    int num_row = 0;

    auto make_chunk = [&]() {
        round++;
        std::vector<std::shared_ptr<arrow::Array>> data_arrays(fields.size());
        for(int i = 0; i < fields.size(); i++) {
            try {
                if(fields[i]->type()->Equals(arrow::int32())) {
                    auto temp_v = convert_any_vector_to<int>(temp_array[i]);
                    DaseX::Util::AppendIntArray(data_arrays[i], temp_v);
                } else if (fields[i]->type()->Equals(arrow::float32())) {
                    auto temp_v = convert_any_vector_to<float>(temp_array[i]);
                    DaseX::Util::AppendFloatArray(data_arrays[i], temp_v);
                } else if (fields[i]->type()->Equals(arrow::float64())) {
                    auto temp_v = convert_any_vector_to<double>(temp_array[i]);
                    DaseX::Util::AppendDoubleArray(data_arrays[i], temp_v);
                } else if (fields[i]->type()->Equals(arrow::utf8())) {
                    auto temp_v = convert_any_vector_to<std::string>(temp_array[i]);
                    DaseX::Util::AppendStringArray(data_arrays[i], temp_v);
                } else {
                    throw std::invalid_argument("Unsupported data type");
                }
            } catch (const std::invalid_argument& e) {
                spdlog::error("[{}:{}] Invalid argument for field {}: {}", __FILE__, __LINE__, i, e.what());
                return RC::INCONSISTENT_DATA_TYPE;
            } catch (const std::out_of_range& e) {
                spdlog::error("[{}:{}] Out of range for field {}: {}", __FILE__, __LINE__, i, e.what());
                return RC::INCONSISTENT_DATA_TYPE;
            }
        }

        std::shared_ptr<arrow::RecordBatch> rbatch;
        rbatch = arrow::RecordBatch::Make(table->schema, count, data_arrays);
        table->insert_data(rbatch, partition_idx);

        // 清空向量
        for(int i = 0; i < fields.size(); i++) {
            temp_array[i].clear();
        }
        count = 0;
        return RC::SUCCESS;
    };

    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string part;
        std::vector<std::string> parts;
        while (std::getline(iss, part, separator)) {
            parts.emplace_back(part);
        }
        if(parts.size() != table->schema->num_fields()) {
            spdlog::error("[{}:{}] The number of fields in the file ({}:{})"
                          " is not equal to the number of fields in the table ({}:{})", 
                          __FILE__, __LINE__, file_name, parts.size(),
                          table_name, table->schema->num_fields());
            return RC::INCONSISTENT_SCHEMA_LENGTH;
        }

        for(int i = 0; i < parts.size(); i++) {
            try {
                if(fields[i]->type()->Equals(arrow::int32())) {
                    temp_array[i].push_back(std::stoi(parts[i]));
                } else if (fields[i]->type()->Equals(arrow::float32())) {
                    temp_array[i].push_back(std::stof(parts[i]));
                } else if (fields[i]->type()->Equals(arrow::float64())) {
                    temp_array[i].push_back(std::stod(parts[i]));
                } else if (fields[i]->type()->Equals(arrow::utf8())) {
                    temp_array[i].push_back(parts[i]);
                }
            } catch (const std::invalid_argument& e) {
                spdlog::error("[{}:{}] Invalid argument for field {}: {}", __FILE__, __LINE__, i, e.what());
                return RC::INCONSISTENT_DATA_TYPE;
            } catch (const std::out_of_range& e) {
                spdlog::error("[{}:{}] Out of range for field {}: {}", __FILE__, __LINE__, i, e.what());
                return RC::INCONSISTENT_DATA_TYPE;
            }
        }
        num_row++;
        count++;
        if(count == chunk_size) {
            auto rc = make_chunk();
            if(rc != RC::SUCCESS) {
                return rc;
            }
        }
    }
    if(count != 0) {
        auto rc = make_chunk();
        if(rc != RC::SUCCESS) {
            return rc;
        }
    }

    auto end_execute = std::chrono::steady_clock::now();

    infile.close();
    spdlog::info("[{}:{}] Insert {} rows into table {}, partition {}, "
        "Chunk num is {}, Duration {} s.", 
        __FILE__, __LINE__, num_row, table_name, partition_idx, round, 
        1.0 * Util::time_difference(start_execute, end_execute) / 1000);

	return RC::SUCCESS;
}

// 把schema序列化为字符串
std::string schema_to_string(std::vector<std::string> &filed_names,
    std::vector<LogicalType> &filed_types) {
    std::string table_schema_value;
    for (int i = 0; i < filed_names.size(); i++) {
        if (i != 0) {
            table_schema_value += "|";
        }
        table_schema_value += filed_names[i] + ":" + 
            LogicalTypeToString(filed_types[i]);
    }
    return table_schema_value;
}

// 从字符串中解析出schema
RC string_to_schema(std::string &table_schema_value,
    TableInfo &table_info) {
    // schema格式：{filed_name1:filed_type1}|{filed_name2:filed_type2}| ...  
    // 把table_schema_value按照|切分
    // 遍历每个字段，解析出字段名，字段类型，字段名和字段类型用冒号分隔
    std::istringstream iss(table_schema_value);
    std::string field;
    table_info.column_nums = 0;
    while (std::getline(iss, field, '|')) {
        std::istringstream field_iss(field);
        std::string field_name, field_type_str;
        if (std::getline(field_iss, field_name, ':') && std::getline(field_iss, field_type_str)) {
            LogicalType field_type = StringToLogicalType(field_type_str);
            table_info.filed_names.push_back(field_name);
            table_info.filed_types.push_back(field_type);
            table_info.column_nums++;
        } else {
            spdlog::error("[{}:{}] Failed to parse fields: {}", __FILE__, __LINE__, table_schema_value);
            return RC::FAILED;
        }
    }
    return RC::SUCCESS;
}

}

// 这个是一个临时的catalog，不会持久化，不用Open函数打开就会调用这个构造函数
CataLog::CataLog():database_path_("./__temp__database__") {
    // 删除上次的数据库catalog
    remove_directory(database_path_);
    bool res = std::filesystem::create_directory(database_path_);
    if(res == false) {
        spdlog::error("[{}:{}] Create database path {} failed.", __FILE__, __LINE__, database_path_);
        abort();
    }

    leveldb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = false;
    options.write_buffer_size = 100  << 20;
    options.max_open_files = 100;

    // 打开临时元信息数据库
    leveldb::Status status = leveldb::DB::Open(options, database_path_ + "/metadata", &meta_database_);
    if (status.ok() == false) {
        spdlog::error("[{}:{}] Open database path {} failed.", __FILE__, __LINE__, database_path_);
        abort();
    }
}

CataLog::CataLog(std::string database_path, leveldb::DB* meta_database) :
    database_path_(database_path), meta_database_(meta_database){
}

CataLog::~CataLog()  {
  if (meta_database_ != nullptr) {
    delete meta_database_;
    spdlog::info("[{}:{}] close meta database of {}", __FILE__, __LINE__, database_path_);
  }
}


// 把表的元信息（TableInfo）保存到leveldb
RC CataLog::register_table(TableInfo &table_info) {
    RC rc = RC::SUCCESS;
    leveldb::Status status;

    // 表名：table_name|{tbl_name1} -> null 
    // 表schema: meta|{tbl_name}|schema -> {filed_name1:filed_type1}| ...  
    // 分区数量：meta|{tbl_name}|partition_num -> int
    std::string table_name_key = "table_name|" + table_info.table_name;
    std::string table_schema_key = "meta|" + table_info.table_name + "|schema";
    std::string table_schema_value;
    std::string table_partition_num_key = "meta|" + table_info.table_name + "|partition_num";

    std::lock_guard<std::mutex> lock(mutex_catalog);

    std::string table_name_value;
    status = meta_database_->Get(leveldb::ReadOptions(), table_name_key, &table_name_value);
    
    if (!status.IsNotFound()) {
        rc = RC::TABLE_ALREADY_EXISTS;
        return rc;
    }
    
    table_schema_value = schema_to_string(table_info.filed_names, table_info.filed_types);
    
     // 保证原子写入
    leveldb::WriteBatch batch;
    batch.Put(table_name_key, "");
    batch.Put(table_schema_key, table_schema_value);
    batch.Put(table_partition_num_key, std::to_string(table_info.partition_num));
    status = meta_database_->Write(leveldb::WriteOptions(), &batch);

    if (status.ok() == false) {
        rc = RC::FAILED;
        spdlog::error("[{}:{}] Write table metadata to database failed.", __FILE__, __LINE__);
    }
    return rc;
}

RC CataLog::delete_table(std::string table_name) {
    RC rc = RC::SUCCESS;
    leveldb::Status status;
    std::lock_guard<std::mutex> lock(mutex_catalog);
    std::string table_name_key = "table_name|" + table_name;
    std::string table_schema_key = "meta|" + table_name + "|schema";
    std::string table_partition_num_key = "meta|" + table_name + "|partition_num";
    std::string table_name_value;
    status = meta_database_->Get(leveldb::ReadOptions(), table_name_key, &table_name_value);
    
    if (status.IsNotFound()) {
        rc = RC::TABLE_NOT_EXISTS;
        return rc;
    }

    tables.erase(table_name);

    // 原子删除
    leveldb::WriteBatch batch;
    batch.Delete(table_name_key);
    batch.Delete(table_schema_key);
    batch.Delete(table_partition_num_key);
    // TODO: 数据文件删除

    status = meta_database_->Write(leveldb::WriteOptions(), &batch);
    if (status.ok() == false) {
        rc = RC::FAILED;
        spdlog::error("[{}:{}] Delete metadata of table {} to database failed.", 
            __FILE__, __LINE__, table_name);
    }
    return rc;
}

// 从leveldb读取table的元信息并恢复
// 这需要把数据文件元信息读取上来并解析出RecordBatch插入table对象
// 这个操作还蛮重的，不建议频繁调用
// NOTE: 打开数据库后，同一个table只能获取一次，否则会有不一致的问题
RC CataLog::get_table(std::string table_name, std::shared_ptr<Table> &table) {
    RC rc = RC::SUCCESS;

    std::lock_guard<std::mutex> lock(mutex_catalog);
    if (tables.find(table_name) != tables.end()) {
        table = tables[table_name];
        return rc;
    }

    leveldb::Status status;
    std::string table_name_key = "table_name|" + table_name;
    std::string table_name_value;
    std::string table_schema_key = "meta|" + table_name + "|schema";
    std::string table_schema_value;
    std::string table_partition_num_key = "meta|" + table_name + "|partition_num";
    std::string table_partition_num_value;
    
    status = meta_database_->Get(leveldb::ReadOptions(), table_name_key, &table_name_value);

    if (status.IsNotFound()) {
        rc = RC::TABLE_NOT_EXISTS;
        return rc;
    }

    TableInfo table_info;
    table_info.table_name = table_name;

    // 读取schema
    status = meta_database_->Get(leveldb::ReadOptions(), table_schema_key, &table_schema_value);
    rc = string_to_schema(table_schema_value, table_info);
    if (rc != RC::SUCCESS) {
        spdlog::error("[{}:{}] String to schema failed.", __FILE__, __LINE__);
        return rc;
    }

    // 获取分区数量
    status = meta_database_->Get(leveldb::ReadOptions(), table_partition_num_key, 
        &table_partition_num_value);
    table_info.partition_num = std::stoi(table_partition_num_value);

    // 创建表对象，但是这里面是空的，需要导入持久化的表的元信息
    table = std::make_shared<Table>(table_info);
    table->cataLog = this;

    // 从metadatabse读取当前表的文件
    // file|{tbl_name}|{partition_idx}|{file_number}.pqt -> {row_count}
    std::string file_name_key_start = "file|" + table_name + "|";
    std::string file_name_key_end = "file|" + table_name + "}";
    auto iter = meta_database_->NewIterator(leveldb::ReadOptions());
    iter->Seek(file_name_key_start);
    for (;iter->key().compare(file_name_key_end) < 0;iter->Next()) {
        int partition_idx = -1;
        int64_t file_number = 0;
        std::string file_name;
        std::string file_path;
        int row_cnt = std::stoi(iter->value().ToString());
        std::unique_ptr<parquet::arrow::FileReader> reader;

        std::string k = iter->key().ToString();
        partition_idx = ParsePartitionIdxFromFileKey(k);
        file_name = ParseFileNameFromFileKey(k);
        file_number = std::stol(file_name.substr(0, file_name.size() - 4));
        file_path = GetFilePath(table->cataLog->database_path_, file_name);

        auto s = Util::OpenParquetFile(file_path, table->schema, reader);
        if (s.ok() == false) {
            spdlog::error("[{}:{}] Open parquet file {} failed.", __FILE__, __LINE__, file_path);
            return RC::FAILED;
        }

        RecordBatchReader recoedbatch_reader(file_number, file_path, row_cnt, std::move(reader));
        table->insert_data(recoedbatch_reader, partition_idx);
    }
    
    tables.emplace(table_info.table_name, table);
    rc = RC::SUCCESS;
    return rc;
}

RC CataLog::load_from_single_file(std::string& table_name,
								  std::shared_ptr<Table>& table,
								  int partition_idx, std::string& file_name,
                                  int chunk_size,
                                  char separator) {
    if(table == nullptr) {
        spdlog::error("table is null");
        return RC::FAILED;
    }
	return load_from_single_file_func(table_name, table, partition_idx, file_name, chunk_size, separator);
}


// 1. 检查表是否存在，如果不存在则报错。
// 2. 如果存在则检查开始加载数据
// 3. 加载数据过程中依据分隔符切分数据，每一行都要判断是否符合schema
// 4. 所有数据先打包为内存中的Chunk结构
// 5. 把Chunk结构插入数据库

RC CataLog::load_from_files(std::string& table_name,
    std::shared_ptr<Table>& table,
    std::vector<int>& work_ids,
    std::vector<int>& partition_idxs,
    std::vector<std::string>& file_names,
    int chunk_size,
    char separator) {
    auto rc = RC::SUCCESS;

    if(table == nullptr) {
        spdlog::error("table is null");
        return RC::FAILED;
    }

    std::shared_ptr<Scheduler> scheduler = std::make_shared<Scheduler>(72); // 72核，基本上够了
    int size = work_ids.size();
    for(int i = 0; i < size; i++) {
        scheduler->submit_task(load_from_single_file_func, work_ids[i], true,
            table_name,
            std::ref(table), 
            partition_idxs[i], 
            std::ref(file_names[i]), chunk_size, separator);
    }
    scheduler->shutdown();

    spdlog::info("[{}:{}] Load data all finish!!!", __FILE__, __LINE__);
	return rc;
}

int64_t CataLog::get_next_file_number()  {
    std::lock_guard<std::mutex> lock(mutex_catalog);
    const std::string key = "global_file_number";
    std::string value;
    int64_t current_number = 0;

    // 读取当前文件号
    leveldb::Status status = meta_database_->Get(leveldb::ReadOptions(), key, &value);
    if (status.IsNotFound()) {
        // 键不存在，初始化为 1
        current_number = 1;
    } else if (!status.ok()) {
        throw std::runtime_error("Failed to get file number: " + status.ToString());
    } else {
        // 解析当前值
        try {
            current_number = std::stoll(value);
            current_number++; // 递增
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to parse file number: " + std::string(e.what()));
        }
    }

    // 写回新文件号
    status = meta_database_->Put(leveldb::WriteOptions(), key, std::to_string(current_number));
    if (!status.ok()) {
        throw std::runtime_error("Failed to put file number: " + status.ToString());
    }

    return current_number;
  }

RC OpenDatabase(std::string database_path, std::shared_ptr<CataLog> &catalog, bool create_if_missing) {

    // 如果不存在，且create_if_missing=true则创建
    if (std::filesystem::exists(database_path) == false) {
        if(create_if_missing) {
            bool res = std::filesystem::create_directory(database_path);
            if(res == false) {
                spdlog::error("[{}:{}] Create database path {} failed.", __FILE__, __LINE__, database_path);
                return RC::IO_ERROR;
            }

            spdlog::info("[{}:{}] Create database path {}.", __FILE__, __LINE__, database_path);

        } else {
            spdlog::error("[{}:{}] Database path {} does not exist.", __FILE__, __LINE__, database_path);
            return RC::IO_ERROR;
        }
    } else if (std::filesystem::is_directory(database_path) == false) {
        spdlog::error("[{}:{}] Database path {} is not a directory.", __FILE__, __LINE__, database_path);
        return RC::IO_ERROR;
    }

    // 如果不存在则在当前目录下创建名为metadata的leveldb数据库
    leveldb::DB* meta_database;
    leveldb::Options options;
    options.create_if_missing = true;
    options.write_buffer_size = 100  << 20;
    options.max_open_files = 100;

    leveldb::Status status = leveldb::DB::Open(options, database_path + "/metadata", &meta_database);
    if (status.ok() == false) {
        spdlog::error("[{}:{}] Open meta database {} failed. [{}]", __FILE__, __LINE__, database_path + "/metadata", status.ToString());
        return RC::IO_ERROR;
    }

    catalog = std::make_shared<CataLog>(database_path, meta_database);
    open_catalog_ptr = catalog;
	return RC::SUCCESS;
}

// TODO: 检查数据库是否已经关闭
RC DeleteDatabase(std::string database_path) { 
    return remove_directory(database_path);
}

std::shared_ptr<arrow::RecordBatch> PartitionedTable::get_data(
	int idx, const std::vector<int>& project_ids) {
	std::shared_ptr<arrow::RecordBatch> rbatch;
    // 如果idx大于等于chunk_nums，则返回一个空的RecordBatch。NOTE: 感觉这个逻辑不是很合理
	if (idx >= chunk_nums) {
		arrow::FieldVector res_fields_vector;
		for (int i = 0; i < project_ids.size(); i++) {
			res_fields_vector.emplace_back(schema->field(project_ids[i]));
		}
		std::shared_ptr<arrow::Schema> res_schema =
			std::make_shared<arrow::Schema>(res_fields_vector);
		rbatch = arrow::RecordBatch::MakeEmpty(res_schema).ValueOrDie();
		return rbatch;
	}

    std::shared_ptr<arrow::RecordBatch> tmp;
    // 从columns结构里把列拿出来，并拼成一个RecordBatch返回，
    // 只有刚刚导完数据这个结构才会起作用，重新打开数据库后会从磁盘上读取
    // TODO: 设计一个Cache，把磁盘上读取出来的数据保存起来
    if(columns.size() && columns[0]->get_chunk_nums() == chunk_nums) {
        arrow::ArrayVector column_vector;
        int length = 0;
        for (int i = 0; i < columns.size(); i++) {
            std::shared_ptr<ColumnChunk> column_chunk = columns[i]->get_data(idx);
            column_vector.emplace_back(column_chunk->data);
        }
        length = column_vector[0]->length();
        rbatch = arrow::RecordBatch::Make(schema, length, column_vector);
        if (project_ids.size() == 0) {
            return rbatch;
        }
        tmp = rbatch->SelectColumns(project_ids).ValueOrDie();
    } else {
        // 从磁盘上读取数据
        if(record_batch_readers.size() != chunk_nums || record_batch_readers.size() < idx) {
            return nullptr;
        }
        auto file_name = record_batch_readers[idx].file_path_;
        auto s = Util::ReadRecordBatchFromReader(record_batch_readers[idx].reader_, project_ids, schema, tmp);
        if (!s.ok()) {
            return nullptr;
        }
        spdlog::debug("[{}:{}] Read file: {} from disc", file_name);
    }

	return tmp;
}

RC PartitionedTable::insert_data(
	std::shared_ptr<arrow::RecordBatch>& record_batch) {
	RC rc;
	if (record_batch->num_rows() == 0 || record_batch->num_columns() == 0) {
		rc = RC::INVALID_DATA;
		return rc;
	}
	const std::vector<std::shared_ptr<arrow::Array>>& column_vector =
		record_batch->columns();
	for (int i = 0; i < column_vector.size(); i++) {
		std::lock_guard<std::mutex> lock(mutex_table);
		columns[i]->insert_data(
			std::move(std::make_shared<ColumnChunk>(column_vector[i])));
	}
	row_nums += record_batch->num_rows();
	chunk_nums++;

    // 把RecordBatch写入磁盘
	// 获取文件名 file_num.pqt
	auto file_number = table->cataLog->get_next_file_number();
	std::string file_name = GetFileName(file_number);
	std::string file_path =
		GetFilePath(table->cataLog->database_path_, file_name);
	auto s = Util::WriteRecordBatchToDisk(record_batch, file_path);
	if (!s.ok()) {
		rc = RC::IO_ERROR;
		return rc;
	}
    spdlog::debug("[{}:{}] write file {} success, row_count: {}", __FILE__, __LINE__, 
            file_path, record_batch->num_rows());

    // file|{tbl_name}|{partition_idx}|{file_number}.pqt -> {row_count}
    auto sl = table->cataLog->meta_database_->Put(leveldb::WriteOptions(), 
        CreateFileKey(table->table_name, partition_id, file_name),
		std::to_string(record_batch->num_rows()));
    if (!sl.ok()) {
		rc = RC::IO_ERROR;
		return rc;
	}

    std::unique_ptr<parquet::arrow::FileReader> reader;
    s = Util::OpenParquetFile(file_path, schema, reader);
    if (!s.ok()) {
        rc = RC::IO_ERROR;
        spdlog::error("[{}:{}] Open parquet file {} failed. [{}]", 
            __FILE__, __LINE__, file_path, s.ToString());
        return rc;
    }
    record_batch_readers.emplace_back(file_number,file_path, record_batch->num_rows(), std::move(reader));

	return rc;
}  // insert_data

RC PartitionedTable::insert_data(RecordBatchReader& record_batch_reader) {
    record_batch_readers.emplace_back(std::move(record_batch_reader));
    row_nums += record_batch_reader.row_num_;
	chunk_nums++;
	return RC::SUCCESS;
}

RC Table::insert_data(RecordBatchReader& record_batch_reader, int partition_id) { 
    if (partition_id >= partition_num) {
        return RC::INVALID_DATA;
    }
    partition_tables[partition_id]->insert_data(record_batch_reader);
    partition_bitmap.at(partition_id) = 1;
    row_nums += record_batch_reader.row_num_;
    return RC::SUCCESS; 
}



// NOTE: 太多地方直接通过global_catalog获取table了，这里采用了一个临时解决方案：
// 如果不通过 OpenDatabase 打开数据库，则会产生一个临时数据库global_catalog
// 临时数据库目录__temp__database__，用完即删，
// 如果未来要开源给别人看，建议把相关的逻辑重新设计，不然太丢人了。
std::shared_ptr<CataLog> global_catalog = std::make_shared<CataLog>();

std::weak_ptr<CataLog> open_catalog_ptr = global_catalog;

}  // namespace DaseX
