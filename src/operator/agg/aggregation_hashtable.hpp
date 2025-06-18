#pragma once

#include "physical_operator_type.hpp"
#include "value_type.hpp"
#include "common_macro.hpp"
#include "aggregation_state.hpp"
#include "config.hpp"
#include <vector>
#include <set>
#include <arrow/api.h>

namespace DaseX {

class EntryKey {
public:
    std::vector<std::shared_ptr<Value>> key_values;
    size_t hash_val;
public:
    EntryKey() = default;
    EntryKey(std::vector<std::shared_ptr<Value>> key_values, size_t hash_val) : key_values(std::move(key_values)), hash_val(hash_val) {}
public:
    bool operator==(const EntryKey &other) {
        if(hash_val != other.hash_val) {
            return false;
        }
        for(int i = 0; i < key_values.size(); i++) {
            if(*(key_values[i]) != *(other.key_values[i])) {
                return false;
            }
        }
        return true;
    }
};

class EntrySet {
public:
    std::shared_ptr<EntryKey> key;
    std::vector<std::shared_ptr<Aggstate>> states;
    std::vector<AggFunctionType> aggFunctionType;
    EntrySet() = default;
    EntrySet(std::shared_ptr<EntryKey> key, std::vector<std::shared_ptr<Aggstate>> states,
             std::vector<AggFunctionType> aggFunctionType) : key(key),
             states(std::move(states)), aggFunctionType(std::move(aggFunctionType)) {}
public:
    void UpdateState(std::vector<std::shared_ptr<Value>> &values, std::vector<int> &star_bitmap);
    void MergeState(std::vector<std::shared_ptr<Aggstate>> &other_states);
    std::vector<std::shared_ptr<Value>> GetFormatRow();
};

class AggBucket {
public:
    std::vector<std::shared_ptr<EntrySet>> bucket;
    int num_rows = 0;
    int bucket_size;
    // size_t tag_info = 0;
    AggBucket(int bucket_size = 16) : bucket_size(bucket_size){
        bucket.resize(bucket_size);
    }
public:
    void InsertEntrySet(std::shared_ptr<EntrySet> &entrySet);
    void UpdateState(int idx, std::vector<std::shared_ptr<Value>> &values, std::vector<int> &star_bitmap);
    int FindEntry(std::shared_ptr<EntryKey> &entryKey);
};

class AggHashTable {
public:
    int bucket_num;
    std::vector<std::shared_ptr<AggBucket>> buckets;
    std::vector<int8_t> bucket_map;       // 统计那个桶被使用，合并时就不用遍历所有桶了
    std::vector<int> group_set;
    std::vector<AggFunctionType> aggregate_function_types;
    int used_bucket_num = 0;
    int total_group_num = 0;                    // 统计该HT中共有多少个分组
public:
    AggHashTable(std::vector<int> group_set, std::vector<AggFunctionType> aggregate_function_types,
                 int bucket_num = AGG_TABLE_BUCKET_NUMS) : group_set(std::move(group_set)),
                 aggregate_function_types(std::move(aggregate_function_types)), bucket_num(bucket_num) {
        buckets.resize(bucket_num);
        bucket_map.resize(bucket_num);
    }
    void UpdateState(std::vector<std::shared_ptr<Value>> key, size_t hash,
                     std::vector<std::shared_ptr<Value>> &value,
                     std::vector<int> &star_bitmap);
    void InitializeState(std::vector<std::shared_ptr<Aggstate>> &states,
                         std::vector<std::shared_ptr<Value>> &value,
                         std::vector<int> &star_bitmap);
    void MergeState(std::shared_ptr<AggHashTable> &other);
    void ResizeBuckets();
};

class RadixPartitionTable {
public:
    int partition_num = 0;
    std::vector<std::shared_ptr<AggHashTable>> partitions;
    std::vector<int> group_set; // GroupBy字段
    std::vector<int> agg_set; // 聚合字段
    std::vector<int> star_bitmap; // 对应字段是否为 ‘*’ 表达式
    std::vector<AggFunctionType> aggregate_function_types;
public:
    RadixPartitionTable() = default;
    RadixPartitionTable(std::vector<int> group_set,
                        std::vector<int> agg_set, std::vector<int> star_bitmap,
                        std::vector<AggFunctionType> aggregate_function_types);
    void Initialize(int part_num);
    std::shared_ptr<AggHashTable> GetAggHashTable(int idx);
    void InsertData(std::shared_ptr<arrow::RecordBatch> &chunk);
    std::vector<size_t> Hashs(std::vector<std::shared_ptr<arrow::Array>> &columns, int count);
};

} // DaseX