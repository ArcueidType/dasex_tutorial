#pragma once

#include "config.hpp"
#include "physical_operator_type.hpp"
#include "value_type.hpp"
#include "bloom_filter.hpp"
#include "expression_type.hpp"
#include <arrow/api.h>
#include <functional>

namespace DaseX {

//using KEY = Value;
//using VALUE = std::shared_ptr<arrow::RecordBatch>;
using ValuePtr = std::shared_ptr<Value>;
using KEY      = std::vector<ValuePtr>;     //记录连接键
using VALUE    = std::vector<ValuePtr>;     // 记录完整的一行记录

struct EntrySingle {
    KEY    key;
    VALUE  val;
    size_t hash_val;
    // 畅：添加tagged_info字段，就近放在hash_val后面
    uint32_t tagged_info;
    EntrySingle(KEY key, VALUE val, size_t hash_val, uint32_t tagged_info)
        : key(std::move(key)), val(std::move(val)), hash_val(hash_val), tagged_info(tagged_info) {}
};

// 行数据分区，也可以理解为HashTable的桶，用来存储数据
struct TupleBucket {
    int tuple_nums = 0;
    std::vector<std::shared_ptr<EntrySingle>> entry_set;
    uint32_t tagged_info_or = 0;
    void InsertEntry(std::shared_ptr<EntrySingle> &entrySingle) {
        if (entry_set.empty()) {
            tagged_info_or = entrySingle->tagged_info;
        } else {
            tagged_info_or |= entrySingle->tagged_info;
        }
        entry_set.emplace_back(entrySingle);
        tuple_nums++;
    }
    std::shared_ptr<EntrySingle> GetEntry(int idx) {
        return entry_set[idx];
    }
};
// 记录探测状态
struct ProbeState {
    std::vector<int> match_row_idx;   // 记录匹配行在块中的偏移量
    // TODO: 可用用更节省内存的方式表示bit_map，后续优化
    std::vector<int8_t> bit_map;  // 记录probe块有那些行匹配到数据，那些没匹配到，1匹配，0未匹配
    // 探测时收集左表数据
    std::vector<VALUE> left_result;
    void AddMatch(int idx) {
        match_row_idx.emplace_back(idx);
    }
};

class JoinHashTable {
public:
    JoinTypes join_type;
    std::vector<int> build_ids;
    std::vector<int> probe_ids;
    std::unique_ptr<BloomFilter> bloom_filter;
    // TODO: 多比较条件暂时只用于SEMI、ANTI-join，应该扩展到所有类型JOIN，后续优化
    std::vector<ExpressionTypes> comparison_types;
    std::vector<std::shared_ptr<TupleBucket>> buckets;
    std::vector<std::shared_ptr<arrow::RecordBatch>> data_chunks;
    // 记录Build表的模式，用于外连接补空
    std::shared_ptr<arrow::Schema> schema;
    std::shared_ptr<arrow::RecordBatch> null_row;
    int64_t nums_chunk = 0;
    const uint64_t radix_bits = RADIX_BITS;
    uint64_t NUM_PARTITIONS;
    uint64_t SHIFT;
    uint64_t MASK;
    int64_t total_count = 0;
    int precision = PRECISION;
public:
    JoinHashTable(std::vector<int> build_ids, std::vector<int> probe_ids, std::vector<ExpressionTypes> comparison_types, JoinTypes join_type = JoinTypes::INNER);
    void Build(std::shared_ptr<arrow::RecordBatch> &chunk);
    std::shared_ptr<ProbeState> Probe(std::shared_ptr<arrow::RecordBatch> &chunk);
    // 根据Probe结果，获取最终的连接数据
    template<JoinTypes join_type>
    void GetJoinResult(std::shared_ptr<arrow::RecordBatch> &chunk, std::shared_ptr<arrow::RecordBatch> &result, std::shared_ptr<ProbeState> &probe_state, std::shared_ptr<ProbeState> &last_probe_state, bool is_final_probe = false);
public:
    // 对输入数组求Hash
    std::vector<size_t> Hashs(std::vector<std::shared_ptr<arrow::Array>> &join_keys, int count);
    // 畅：计算tagged_info向量
    std::vector<uint32_t> ComputeTaggedInfo(const std::vector<size_t>& hashes);
    // 对输入Hash数组求每条数据对应的桶编号
    std::vector<int> ComputeBucketIndices(std::vector<size_t> &join_key_hash, int count);
    // 根据桶编号将数据散列到各个桶
    void ScatterData(std::vector<int> &bucket_idx, int count, std::vector<size_t> &hashs, std::vector<uint32_t> &tagged_infos);
    // 根据桶编号匹配所有数据
    std::shared_ptr<ProbeState> GatherData(std::vector<std::shared_ptr<arrow::Array>> &join_keys, std::vector<int> &bucket_idx, int count, std::vector<size_t> &hashes);
    // 用于SEMI join
    std::shared_ptr<ProbeState> GatherSemiData(std::vector<std::shared_ptr<arrow::Array>> &join_keys,
                                               std::vector<int> &bucket_idx,
                                               int count,
                                               std::vector<size_t> &hashes);
    // 用于ANTI join
    std::shared_ptr<ProbeState> GatherAntiData(std::vector<std::shared_ptr<arrow::Array>> &join_keys,
                                               std::vector<int> &bucket_idx,
                                               int count,
                                               std::vector<size_t> &hashes);

    // 构建一行空数据
    void GetSingleNullRow();
    // Radix分区相关方法
    static inline constexpr uint64_t NumberOfPartitions(uint64_t radix_bits) {
        return uint64_t(1) << radix_bits;
    }
    //! Radix bits begin after uint16_t because these bits are used as salt in the aggregate HT
    static inline constexpr uint64_t Shift(uint64_t radix_bits) {
        return (sizeof(uint64_t) - sizeof(uint16_t)) * 8 - radix_bits;
    }
    //! Mask of the radix bits of the hash
    static inline constexpr uint64_t Mask(uint64_t radix_bits) {
        return (uint64_t(1 << radix_bits) - 1) << Shift(radix_bits);
    }
};

} // DaseX
