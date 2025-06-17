#include "aggregation_hashtable.hpp"
#include <spdlog/spdlog.h>

namespace {

inline double RoundToDecimalPlaces(double value, int precision) {
    double factor = std::pow(10.0, precision);
    return std::round(value * factor) / factor;
}

inline float RoundToDecimalPlaces(float value, int precision) {
    float factor = std::pow(10.0f, precision);
    return std::round(value * factor) / factor;
}

// 将arrow ArrayVector转换为行数据，Array可以有多个数据
void ArrayVecToRowVec(std::vector<std::shared_ptr<arrow::Array>> &cols,
                      std::vector<std::vector<std::shared_ptr<DaseX::Value>>> &keys) {
    int col_nums = cols.size();
    int row_nums = cols[0]->length();
    keys.resize(row_nums, std::vector<std::shared_ptr<DaseX::Value>>(col_nums));
    for(int row_idx = 0; row_idx < row_nums; row_idx++) {
        for(int col_idx = 0; col_idx < col_nums; col_idx++) {
            auto &col = cols[col_idx];
            switch (col->type_id()) {
                case arrow::Type::INT32:
                {
                    auto array = std::static_pointer_cast<arrow::Int32Array>(col);
                    if(array->IsNull(row_idx)) {
                        auto val_p = std::make_shared<DaseX::Value>(0);
                        val_p->is_null = true;
                        keys[row_idx][col_idx] = val_p;
                    } else {
                        int val = array->Value(row_idx);
                        auto val_p = std::make_shared<DaseX::Value>(val);
                        keys[row_idx][col_idx] = val_p;
                    }
                    break;
                }
                case arrow::Type::FLOAT:
                {
                    auto array = std::static_pointer_cast<arrow::FloatArray>(col);
                    if(array->IsNull(row_idx)) {
                        auto val_p = std::make_shared<DaseX::Value>(0.0F);
                        val_p->is_null = true;
                        keys[row_idx][col_idx] = val_p;
                    } else {
                        float val = array->Value(row_idx);
                        auto val_p = std::make_shared<DaseX::Value>(val);
                        keys[row_idx][col_idx] = val_p;
                    }
                    break;
                }
                case arrow::Type::DOUBLE:
                {
                    auto array = std::static_pointer_cast<arrow::DoubleArray>(col);
                    if(array->IsNull(row_idx)) {
                        auto val_p = std::make_shared<DaseX::Value>(0.0);
                        val_p->is_null = true;
                        keys[row_idx][col_idx] = val_p;
                    } else {
                        double val = array->Value(row_idx);
                        auto val_p = std::make_shared<DaseX::Value>(val);
                        keys[row_idx][col_idx] = val_p;
                    }
                    break;
                }
                case arrow::Type::STRING:
                {
                    auto array = std::static_pointer_cast<arrow::StringArray>(col);
                    if(array->IsNull(row_idx)) {
                        auto val_p = std::make_shared<DaseX::Value>("0");
                        val_p->is_null = true;
                        keys[row_idx][col_idx] = val_p;
                    } else {
                        std::string val = array->GetString(row_idx);
                        auto val_p = std::make_shared<DaseX::Value>(val);
                        keys[row_idx][col_idx] = val_p;
                    }
                    break;
                }
                default:
                    throw std::runtime_error("Invalid arrow::Type!!!");
            } // switch
        }
    }
}

// 提取sel_vec指定的列
inline std::vector<std::shared_ptr<arrow::Array>> SelectColumns(std::shared_ptr<arrow::RecordBatch> &chunk, std::vector<int> &sel_vec) {
    std::vector<std::shared_ptr<arrow::Array>> res;
    auto cols = chunk->columns();
    for (int i = 0; i < sel_vec.size(); i++) {
        res.emplace_back(cols[sel_vec[i]]);
    }
    return res;
}

std::vector<int> ComputePartitionIndices(std::vector<size_t> &hash_vals, int count, int MASK) {
    std::vector<int> bucket_idxs(count);
    for(int i = 0; i < count; i++) {
        size_t hash = hash_vals[i];
        bucket_idxs[i] = hash & MASK;
    }
    return bucket_idxs;
}

}

namespace DaseX {
// ================================================EntrySet==========================================================
void EntrySet::UpdateState(std::vector<std::shared_ptr<Value>> &values,
                           std::vector<int> &star_bitmap) {
    int values_size = values.size();
    for(int i = 0; i < values_size; i++) {
        if(aggFunctionType[i] != AggFunctionType::COUNT) {
            if(!(values[i]->is_null)) {
                states[i]->add(*(values[i]));
            }
        } else {
            if((!(values[i]->is_null)) || star_bitmap[i] == 1) {
                states[i]->add(1);
            }
        }
    }
}

void EntrySet::MergeState(std::vector<std::shared_ptr<Aggstate>> &other_states) {
    int states_size = states.size();
    for(int i = 0; i < states_size; i++) {
        states[i]->merge(*(other_states[i]));
    }
}

std::vector<std::shared_ptr<Value>> EntrySet::GetFormatRow() {
    std::vector<std::shared_ptr<Value>> res;
    for(int i = 0; i < key->key_values.size(); i++) {
        res.push_back(key->key_values[i]);
    }
    for(int i = 0; i < states.size(); i++) {
        switch (aggFunctionType[i]) {
            case AggFunctionType::AVG:
            {
                std::shared_ptr<Value> val = std::make_shared<Value>(std::static_pointer_cast<AvgState>(states[i])->finalize_avg());
                res.push_back(val);
                break;
            }
            case AggFunctionType::MAX:
            case AggFunctionType::MIN:
            case AggFunctionType::COUNT:
            case AggFunctionType::SUM:
            {
                std::shared_ptr<Value> val = std::make_shared<Value>(states[i]->finalize());
                res.push_back(val);
                break;
            }
            default:
                throw std::runtime_error("Not Supported For Unknown AggType");
        }
    }
    return res;
}

// ================================================AggBucket=========================================================
void AggBucket::InsertEntrySet(std::shared_ptr<EntrySet> &entrySet) {
    if(num_rows < bucket_size) {
        bucket[num_rows++] = entrySet;
    } else {
        bucket.emplace_back(entrySet);
        num_rows++;
        bucket_size++;
    }
}

void AggBucket::UpdateState(int idx, std::vector<std::shared_ptr<Value>> &values,
                            std::vector<int> &star_bitmap) {
    bucket[idx]->UpdateState(values,star_bitmap);
}

int AggBucket::FindEntry(std::shared_ptr<EntryKey> &entryKey) {
    auto &entry = *entryKey;
    int res = -1;
    for(int i = 0; i < num_rows; i++) {
        if(bucket[i] == nullptr) {
            continue;
        }
        if(*(bucket[i]->key) == entry) {
            res = i;
            break;
        }
    }
    return res;
}

// ================================================AggHashTable======================================================

void AggHashTable::UpdateState(std::vector<std::shared_ptr<Value>> key, size_t hash,
                               std::vector<std::shared_ptr<Value>> &value,
                               std::vector<int> &star_bitmap) {
    int bucket_idx = hash & (bucket_num - 1);
    auto &aggBucket = buckets[bucket_idx];
    bucket_map[bucket_idx] = 1;
    std::shared_ptr<EntryKey> entryKey = std::make_shared<EntryKey>(key, hash);
    // 初始化状态
    if(aggBucket == nullptr) {
        aggBucket = std::make_shared<AggBucket>();
        std::vector<std::shared_ptr<Aggstate>> states;
        InitializeState(states, value, star_bitmap);
        std::shared_ptr<EntrySet> entrySet = std::make_shared<EntrySet>(entryKey, states, aggregate_function_types);
        aggBucket->InsertEntrySet(entrySet);
        total_group_num++;
        used_bucket_num++;
    } else {
        int res = aggBucket->FindEntry(entryKey);
        if(res != -1) {
            aggBucket->UpdateState(res,value, star_bitmap);
        } else {
            std::vector<std::shared_ptr<Aggstate>> states;
            InitializeState(states, value, star_bitmap);
            std::shared_ptr<EntrySet> entrySet = std::make_shared<EntrySet>(entryKey, states, aggregate_function_types);
            aggBucket->InsertEntrySet(entrySet);
            total_group_num++;
        }
    }
    if (total_group_num / used_bucket_num > AVG_BUCKET_LOAD) {
        ResizeBuckets();
    }
}

void AggHashTable::ResizeBuckets() {
    int new_bucket_num = bucket_num * 2;
    std::vector<std::shared_ptr<AggBucket>> new_buckets(new_bucket_num);
    std::vector<int8_t> new_bucket_map(new_bucket_num);
    used_bucket_num = 0;

    for (int i = 0; i < bucket_num; ++i) {
        if (bucket_map[i] == 0 || buckets[i] == nullptr) continue;
        for (auto &entry : buckets[i]->bucket) {
            if (!entry) continue;
            size_t new_hash = entry->key->hash_val;
            int new_idx = new_hash & (new_bucket_num - 1);
            if (new_buckets[new_idx] == nullptr) {
                new_buckets[new_idx] = std::make_shared<AggBucket>();
                new_bucket_map[new_idx] = 1;
                used_bucket_num++;
            }
            new_buckets[new_idx]->InsertEntrySet(entry);
        }
    }

    buckets = std::move(new_buckets);
    bucket_map = std::move(new_bucket_map);
    bucket_num = new_bucket_num;
}


void AggHashTable::InitializeState(std::vector<std::shared_ptr<Aggstate>> &states,
                                   std::vector<std::shared_ptr<Value>> &value,
                                   std::vector<int> &star_bitmap) {
    for(int i = 0; i < aggregate_function_types.size(); i++) {
        auto aggFunctionType = aggregate_function_types[i];
        switch (aggFunctionType) {
            case AggFunctionType::COUNT:
            {
                // 初始化状态
                auto aggState = std::make_shared<CountState>();
                if((!(value[i]->is_null)) || star_bitmap[i] == 1) {
                    aggState->add(1);
                }
                states.emplace_back(aggState);
                break;
            }
            case AggFunctionType::AVG:
            {
                // 初始化状态
                auto aggState = std::make_shared<AvgState>();
                aggState->add(*(value[i]));
                states.emplace_back(aggState);
                break;
            }
            case AggFunctionType::MIN:
            {
                // 初始化状态
                auto aggState = std::make_shared<MinState>();
                aggState->add(*(value[i]));
                states.emplace_back(aggState);
                break;
            }
            case AggFunctionType::MAX:
            {
                auto aggState = std::make_shared<MaxState>();
                aggState->add(*(value[i]));
                states.emplace_back(aggState);
                break;
            }
            case AggFunctionType::SUM:
            {
                auto aggState = std::make_shared<SumState>();
                aggState->add(*(value[i]));
                states.emplace_back(aggState);
                break;
            }
            default:
                throw std::runtime_error("Not Supported For Unknown AggType");
        }
    }
}

void AggHashTable::MergeState(std::shared_ptr<AggHashTable> &other) {
    if(total_group_num == 0) {
        buckets = other->buckets;
        bucket_num = other->bucket_num;
        bucket_map = other->bucket_map;
        group_set = other->group_set;
        aggregate_function_types = other->aggregate_function_types;
        total_group_num = other->total_group_num;
    } else {
        auto &other_buckets = other->buckets;
        auto &other_bucket_map = other->bucket_map;
        for(int i = 0; i < bucket_num; i++) {
            if(other_bucket_map[i] == 0) {
                continue;
            }
            if(buckets[i] == nullptr) {
                buckets[i] = std::make_shared<AggBucket>();
            }
            auto &other_bucket = other_buckets[i]->bucket;
            for(auto &entrySet : other_bucket) {
                if(entrySet == nullptr) {
                    continue;
                }
                auto &key = entrySet->key;
                int res = buckets[i]->FindEntry(key);
                if(res != -1) {
                    auto &entry_p = buckets[i]->bucket[res];
                    entry_p->MergeState(entrySet->states);
                } else {
                    bucket_map[i] = 1;
                    buckets[i]->InsertEntrySet(entrySet);
                }
            }
        }
    }
}


// ================================================RadixPartitionTable===============================================
RadixPartitionTable::RadixPartitionTable(std::vector<int> group_set,
                    std::vector<int> agg_set, std::vector<int> star_bitmap,
                    std::vector<AggFunctionType> aggregate_function_types)
        : group_set(std::move(group_set)),
          agg_set(std::move(agg_set)),
          star_bitmap(std::move(star_bitmap)),
          aggregate_function_types(std::move(aggregate_function_types)) {

}

void RadixPartitionTable::Initialize(int part_num) {
    partition_num = part_num;
    partitions.resize(partition_num);
    for(int i = 0; i < partition_num; i++) {
        partitions[i] = std::make_shared<AggHashTable>(group_set, aggregate_function_types);
    }
}

std::shared_ptr<AggHashTable> RadixPartitionTable::GetAggHashTable(int idx) {
    return partitions[idx];
}

void RadixPartitionTable::InsertData(std::shared_ptr<arrow::RecordBatch> &chunk) {
    // 求Key的Hash
    int num_rows = chunk->num_rows();
    std::vector<std::shared_ptr<arrow::Array>> group_columns = SelectColumns(chunk, group_set) ;
    std::vector<std::shared_ptr<arrow::Array>> agg_columns = SelectColumns(chunk, agg_set);
    std::vector<std::vector<std::shared_ptr<Value>>> group_col;
    std::vector<std::vector<std::shared_ptr<Value>>> agg_col;
    ArrayVecToRowVec(group_columns, group_col);
    ArrayVecToRowVec(agg_columns, agg_col);
    const int MASK = partition_num - 1;
    std::vector<size_t> hashs = Hashs(group_columns, num_rows);
    std::vector<int> partitions_idxs = ComputePartitionIndices(hashs, num_rows, MASK);
    // 根据Hash值更新状态
    for (int i = 0; i < num_rows; i++) {
        auto &agg_ht = partitions[partitions_idxs[i]];
        agg_ht->UpdateState(group_col[i], hashs[i], agg_col[i], star_bitmap);
    }
}

// 求Hash的同时，把GroupBy字段的值全部记录下来(行存模式)
std::vector<size_t> RadixPartitionTable::Hashs(std::vector<std::shared_ptr<arrow::Array>> &join_keys, int count) {
    std::vector<size_t> hashes(count);
    for(int k = 0; k < count; k++) {
        std::vector<size_t> hashs_t;
        int column_size = join_keys.size();
        for(int i = 0; i < column_size; i++) {
            auto &join_key = join_keys[i];
            switch (join_key->type_id()) {
                case arrow::Type::INT32:
                {
                    std::hash<int> intHasher;
                    auto int32array = std::static_pointer_cast<arrow::Int32Array>(join_key);
                    int val = int32array->Value(k);
                    size_t hash_p = intHasher(val);
                    hashs_t.emplace_back(hash_p);
                    break;
                }
                case arrow::Type::FLOAT:
                {
                    std::hash<float> float32Hasher;
                    auto float32array = std::static_pointer_cast<arrow::FloatArray>(join_key);
                    float val = float32array->Value(k);
                    float val_p = RoundToDecimalPlaces(val, 2);
                    size_t hash_p = float32Hasher(val_p);
                    hashs_t.emplace_back(hash_p);
                    break;
                }
                case arrow::Type::DOUBLE:
                {
                    std::hash<double> doubleHasher;
                    auto doubleArray = std::static_pointer_cast<arrow::DoubleArray>(join_key);
                    double val = doubleArray->Value(k);
                    double val_p = RoundToDecimalPlaces(val, 2);
                    size_t hash_p = doubleHasher(val_p);
                    hashs_t.emplace_back(hash_p);
                    break;
                }
                case arrow::Type::STRING:
                {
                    std::hash<std::string> stringHasher;
                    auto stringArray = std::static_pointer_cast<arrow::StringArray>(join_key);
                    std::string val = stringArray->GetString(k);
                    size_t hash_p = stringHasher(val);
                    hashs_t.emplace_back(hash_p);
                    break;
                }
                default:
                    throw std::runtime_error("Not Supported For Arrow Type!!!");
            } // switch
        }
        size_t hash_tmp = 0;
        for(int i = 0; i < column_size; i++) {
            hash_tmp = hash_tmp ^ (hashs_t[i] << i);
        }
        hashes[k] = hash_tmp;
    } // for
    return hashes;
}


} // DaseX