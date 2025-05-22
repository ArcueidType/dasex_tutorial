#include "common.hpp"
#include "algorithm"

namespace DaseX {


// {file_number}.pqt
// file_number为定长的8字节字符串
std::string GetFileName(int64_t file_number) {
    std::string ret;
    for (int i = 0; i < 8; ++i) {
        ret += (char)(file_number % 10 + '0');
        file_number /= 10;
    }
    std::reverse(ret.begin(), ret.end());
    ret += ".pqt";
    return ret;
}

std::string GetFilePath(std::string database_path,
                      std::string file_name) {
    return database_path + "/" + file_name;
}

// file|{tbl_name}|{partition_idx}|{file_number}.pqt -> {row_count}
std::string CreateFileKey(std::string &table_name, int partition_idx ,std::string &file_name) {
    return "file|" + table_name + "|" + std::to_string(partition_idx) + "|" + file_name;
}

int ParsePartitionIdxFromFileKey(std::string &input) {
    // 找到第一个 '|' 的位置
    size_t pos1 = input.find('|');
    if (pos1 == std::string::npos) return -1;
    
    // 找到第二个 '|' 的位置
    size_t pos2 = input.find('|', pos1 + 1);
    if (pos2 == std::string::npos) return -1;
    
    // 找到第三个 '|' 的位置
    size_t pos3 = input.find('|', pos2 + 1);
    if (pos3 == std::string::npos) return -1;
    
    // 提取 partition_idx（位于第二个和第三个 '|' 之间）
    return std::stoi(input.substr(pos2 + 1, pos3 - pos2 - 1));
}

std::string ParseFileNameFromFileKey(std::string &key) {
    std::string ret = key.substr(key.size() - 8 - 4);
    return ret;
}

}