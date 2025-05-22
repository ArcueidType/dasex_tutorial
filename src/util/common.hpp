#pragma once

#include "typedefs.hpp"
#include "string"
namespace DaseX {

template <typename T>
const T Load(const_data_ptr_t ptr) {
    T ret;
    memcpy(&ret, ptr, sizeof(ret));
    return ret;
}

// {file_number}.pqt
// file_number为定长的8字节字符串
std::string GetFileName(int64_t file_number);

std::string GetFilePath(std::string database_path,
                      std::string file_name);

// file|{tbl_name}|{partition_idx}|{file_number}.pqt -> {row_count}
std::string CreateFileKey(std::string &table_name, int partition_idx, 
    std::string &file_name);

int ParsePartitionIdxFromFileKey(std::string &input);

std::string ParseFileNameFromFileKey(std::string &key);

} // DaseX
