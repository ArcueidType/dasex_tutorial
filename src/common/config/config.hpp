#pragma once

#include "numa.h"
#include <cstdint>

namespace DaseX {
namespace Numa {

// 定义系统环境变量
extern int SOCKETS;
extern int CORES;

// report the node of the specified cpu. -1/errno on invalid cpu. */
int get_socket(int core);

} // namespace Numa

// src/operator/agg/aggregation_hashtable.hpp
// AggHashTable(int bucket_num = AGG_TABLE_BUCKET_NUMS)16384 65536 2097152
// 配置聚合算子桶初始化大小
const int AGG_TABLE_BUCKET_NUMS = 131072;

// 配置HashJoin桶初始化大小
const uint64_t RADIX_BITS = 18;

// 浮点数保留精度
const int PRECISION = 2;

// 配置布隆过滤器大小 1048576
const int BLOOM_FILTER_SIZE = 16777216;

} // namespace DaseX
