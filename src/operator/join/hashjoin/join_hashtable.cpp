#include "join_hashtable.hpp"
#include "arrow_help.hpp"
#include "common_macro.hpp"
#include "spdlog/spdlog.h"
#include <cmath>

#define ExtractTaggedInfo(hash) ((hash))

namespace {

inline bool AreAlmostEqual(double a, double b, int precision) {
    // 计算容忍度
    double epsilon = std::pow(10.0, -precision);

    // 四舍五入到指定精度
    double roundedA = std::round(a * std::pow(10.0, precision)) / std::pow(10.0, precision);
    double roundedB = std::round(b * std::pow(10.0, precision)) / std::pow(10.0, precision);

    // 比较两数的差值是否在容忍度范围内
    return std::fabs(roundedA - roundedB) < epsilon;
}

inline bool AreAlmostEqual(float a, float b, int precision) {
    // 计算容忍度
    float epsilon = std::pow(10.0f, -precision);

    // 四舍五入到指定精度
    float roundedA = std::round(a * std::pow(10.0, precision)) / std::pow(10.0, precision);
    float roundedB = std::round(b * std::pow(10.0, precision)) / std::pow(10.0, precision);

    // 比较两数的差值是否在容忍度范围内
    return std::fabs(roundedA - roundedB) < epsilon;
}

inline double RoundToDecimalPlaces(double value, int precision) {
    double factor = std::pow(10.0, precision);
    return std::round(value * factor) / factor;
}

inline float RoundToDecimalPlaces(float value, int precision) {
    float factor = std::pow(10.0f, precision);
    return std::round(value * factor) / factor;
}

// // 畅：取tagged_info
// inline size_t ExtractTaggedInfo(size_t hash) {
//     return static_cast<size_t>(hash);
// }

// 提取sel_vec指定的列
inline std::vector<std::shared_ptr<arrow::Array>> SelectColumns(std::shared_ptr<arrow::RecordBatch> &chunk,
                                                                std::vector<int> &sel_vec) {
    std::vector<std::shared_ptr<arrow::Array>> res;
    auto cols = chunk->columns();
    for (int i = 0; i < sel_vec.size(); i++) {
        res.emplace_back(cols[sel_vec[i]]);
    }
    return res;
}

void RbToRowVec(std::shared_ptr<arrow::RecordBatch> &target,
                std::vector<std::vector<std::shared_ptr<DaseX::Value>>> &keys) {
    int col_nums = target->num_columns();
    int row_nums = target->num_rows();
    auto cols = target->columns();
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

// 将行数据追加到行格式的ValueVec
inline void AppendRowToValueVec(std::vector<std::vector<std::shared_ptr<DaseX::Value>>> &left_result,
                                std::vector<std::shared_ptr<DaseX::Value>> &row_data) {
    int col_nums = row_data.size();
    for (int i = 0; i < col_nums; i++) {
        auto &col = left_result[i];
        col.emplace_back(row_data[i]);
    }
}

// 将行数据追加到行格式的ValueVec
inline void AppendNullRowToValueVec(std::vector<std::vector<std::shared_ptr<DaseX::Value>>> &left_result, std::shared_ptr<arrow::Schema> &schema) {
    int col_nums = schema->num_fields();
    for (int i = 0; i < col_nums; i++) {
        auto &col = left_result[i];
        auto field = schema->field(i);
        switch (field->type()->id()) {
            case arrow::Type::INT32:
            {
                auto val_p = std::make_shared<DaseX::Value>(0);
                val_p->is_null = true;
                col.emplace_back(val_p);
                break;
            }
            case arrow::Type::FLOAT:
            {
                auto val_p = std::make_shared<DaseX::Value>(0.0F);
                val_p->is_null = true;
                col.emplace_back(val_p);
                break;
            }
            case arrow::Type::DOUBLE:
            {
                auto val_p = std::make_shared<DaseX::Value>(0.0);
                val_p->is_null = true;
                col.emplace_back(val_p);
                break;
            }
            case arrow::Type::STRING:
            {
                auto val_p = std::make_shared<DaseX::Value>("");
                val_p->is_null = true;
                col.emplace_back(val_p);
                break;
            }
            default:
                throw std::runtime_error("Invalid arrow::Type!!!");
        } // switch
    }
}

// 将行格式的ValueVec转换为Arrow的Array数组
inline void ValueVecToArrayVec(std::vector<std::vector<std::shared_ptr<DaseX::Value>>> &left_result,
                               std::vector<std::shared_ptr<arrow::Array>>& build_columns) {
    int col_nums = left_result.size();
    for (int i = 0; i < col_nums; i++) {
        auto &col = left_result[i];
        std::shared_ptr<arrow::Array> array;
        DaseX::Util::AppendValueArray(array, col);
        build_columns.emplace_back(array);
    }
}


// 将带有空值的行格式的ValueVec转换为Arrow的Array数组
inline void ValueVecToArrayVecWithNull(std::vector<std::vector<std::shared_ptr<DaseX::Value>>> &left_result,
                                       std::vector<std::shared_ptr<arrow::Array>>& build_columns) {
    int col_nums = left_result.size();
    for (int i = 0; i < col_nums; i++) {
        auto &col = left_result[i];
        std::shared_ptr<arrow::Array> array;
        DaseX::Util::AppendValueArrayWithNull(array, col);
        build_columns.emplace_back(array);
    }
}


}

namespace DaseX {

JoinHashTable::JoinHashTable(std::vector<int> build_ids,
                             std::vector<int> probe_ids,
                             std::vector<ExpressionTypes> comparison_types,
                             JoinTypes join_type)
                             : build_ids(std::move(build_ids)),
                             probe_ids(std::move(probe_ids)),
                             comparison_types(comparison_types),
                             join_type(join_type) {
    NUM_PARTITIONS = NumberOfPartitions(radix_bits);
    SHIFT = Shift(radix_bits);
    MASK = Mask(radix_bits);
    buckets.resize(NUM_PARTITIONS);
    bloom_filter = std::make_unique<BloomFilter>();
    // 初始化 buckets，包含NUM_PARTITIONS个TupleBucket对象，后续优化，可以使用惰性初始化
    for (auto &bucket : buckets) {
        bucket = std::make_shared<TupleBucket>();
    }
}

void JoinHashTable::Build(std::shared_ptr<arrow::RecordBatch> &chunk) {
    if(unlikely(schema == nullptr)) {
        schema = chunk->schema();
        switch (join_type) {
            case JoinTypes::LEFT:
            case JoinTypes::RIGHT:
            {
                GetSingleNullRow();
                break;
            }
            default:
                break;
        } // switch
    }
    int num_rows = chunk->num_rows();
    if(num_rows == 0) {
        return;
    }
    data_chunks.emplace_back(chunk);
    total_count += num_rows;
    std::vector<std::shared_ptr<arrow::Array>> join_keys;
    if(comparison_types.empty()) {
        join_keys =SelectColumns(chunk, build_ids);
    }
    else {
        for (int i = 0; i < comparison_types.size(); i++) {
            if(comparison_types[i] == ExpressionTypes::COMPARE_EQUAL) {
                std::shared_ptr<arrow::Array> join_key_p = chunk->column(build_ids[i]);
                join_keys.emplace_back(join_key_p);
            }
        }
    }
    // 计算Hash值
    std::vector<size_t> hashes = Hashs(join_keys, num_rows);
    for(int i = 0; i < hashes.size(); i++) {
        bloom_filter->Add(hashes[i]);
    }
    // 计算每行数据应该属于那个桶
    std::vector<int> bucket_idx = ComputeBucketIndices(hashes, num_rows);
    // 畅：调用计算tagged_info
    std::vector<size_t> tagged_infos = ComputeTaggedInfo(hashes);
    ScatterData(bucket_idx, num_rows, hashes, tagged_infos);
}

// 畅：计算tagged_info的封装函数
std::vector<size_t> JoinHashTable::ComputeTaggedInfo(const std::vector<size_t>& hashes) {
    std::vector<size_t> tagged_infos(hashes.size());
    for (int i = 0; i < hashes.size(); ++i) {
        tagged_infos[i] = ExtractTaggedInfo(hashes[i]);
    }
    return tagged_infos;
}

std::vector<size_t> JoinHashTable::Hashs(std::vector<std::shared_ptr<arrow::Array>> &join_keys, int count) {
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
                    float val_p = RoundToDecimalPlaces(val, precision);
                    size_t hash_p = float32Hasher(val_p);
                    hashs_t.emplace_back(hash_p);
                    break;
                }
                case arrow::Type::DOUBLE:
                {
                    std::hash<double> doubleHasher;
                    auto doubleArray = std::static_pointer_cast<arrow::DoubleArray>(join_key);
                    double val = doubleArray->Value(k);
                    double val_p = RoundToDecimalPlaces(val, precision);
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
        } // for
        size_t hash_tmp = 0;
        for(int i = 0; i < column_size; i++) {
            hash_tmp = hash_tmp ^ (hashs_t[i] << i);
        }
        hashes[k] = hash_tmp;
    } // for
    return hashes;
} // Hashs

std::vector<int> JoinHashTable::ComputeBucketIndices(std::vector<size_t> &join_key_hash, int count) {
    std::vector<int> bucket_idx(count);
    for(int i = 0; i < count; i++) {
        size_t hash = join_key_hash[i];
        bucket_idx[i] = hash & (NUM_PARTITIONS - 1);
    }
    return bucket_idx;
}

// 畅：修改了函数参数，以及写入EntrySingle的参数
void JoinHashTable::ScatterData(std::vector<int> &bucket_idx, int count, std::vector<size_t> &hashs, std::vector<size_t> &tagged_infos) {
    std::vector<std::vector<std::shared_ptr<DaseX::Value>>> rows;
    RbToRowVec(data_chunks[nums_chunk], rows);
    for(int i = 0; i < count; i++) {
        auto &bucket = buckets[bucket_idx[i]];
        std::vector<std::shared_ptr<Value>> key_row;
        std::vector<std::shared_ptr<Value>> &val_row = rows[i];
        for(int j = 0; j < build_ids.size(); j++) {
            key_row.emplace_back(val_row[build_ids[j]]);
        }
        std::shared_ptr<EntrySingle> entry_p = std::make_shared<EntrySingle>(key_row, val_row, hashs[i], tagged_infos[i]);
        bucket->InsertEntry(entry_p);
    }
    nums_chunk++;
}

std::shared_ptr<ProbeState> JoinHashTable::Probe(std::shared_ptr<arrow::RecordBatch> &chunk) {
    if(total_count == 0) {
        return std::make_shared<ProbeState>();
    }
    int num_rows = chunk->num_rows();
    std::vector<std::shared_ptr<arrow::Array>> join_keys;
    if(comparison_types.empty()) {
        for(auto probe_id : probe_ids){
            std::shared_ptr<arrow::Array> join_key = chunk->column(probe_id);
            join_keys.emplace_back(join_key);
        }
    } else {
        for (int i = 0; i < comparison_types.size(); i++) {
            if(comparison_types[i] == ExpressionTypes::COMPARE_EQUAL) {
                std::shared_ptr<arrow::Array> join_key_p = chunk->column(probe_ids[i]);
                join_keys.emplace_back(join_key_p);
            }
        }
    }
    // 计算Hash值
    std::vector<size_t> hashes = Hashs(join_keys, num_rows);
    // 计算每行数据应该属于那个桶
    std::vector<int> bucket_idx = ComputeBucketIndices(hashes, num_rows);
    // 获取探测结果
    std::shared_ptr<ProbeState> probe_state;
    switch (join_type) {
        case JoinTypes::SEMI:
        {
            std::vector<std::shared_ptr<arrow::Array>> join_keys_p;
            for(auto probe_id : probe_ids){
                std::shared_ptr<arrow::Array> join_key = chunk->column(probe_id);
                join_keys_p.emplace_back(join_key);
            }
            probe_state = GatherSemiData(join_keys_p, bucket_idx, num_rows, hashes);
            break;
        }
        case JoinTypes::ANTI:
        {
            std::vector<std::shared_ptr<arrow::Array>> join_keys_p;
            for(auto probe_id : probe_ids){
                std::shared_ptr<arrow::Array> join_key = chunk->column(probe_id);
                join_keys_p.emplace_back(join_key);
            }
            probe_state = GatherAntiData(join_keys_p, bucket_idx, num_rows, hashes);
            break;
        }
        default:
            probe_state = GatherData(join_keys, bucket_idx, num_rows, hashes);
    } // switch
    return probe_state;
}

std::shared_ptr<ProbeState> JoinHashTable::GatherData(std::vector<std::shared_ptr<arrow::Array>> &join_keys,
                                                      std::vector<int> &bucket_idx,
                                                      int count,
                                                      std::vector<size_t> &hashes) {
    std::shared_ptr<ProbeState> probe_state = std::make_shared<ProbeState>();
    // probe_state->bit_map.resize(count);
    probe_state->InitBitmap(count);
    // probe_state->left_result.resize(schema->num_fields());
    for(int i = 0; i < count; i++) {
        // if(!bloom_filter->IsInBloomFilter(hashes[i])) {
        //     probe_state->bit_map[i] = 0;
        //     // spdlog::info("触发了bloom_filter");
        //     continue;
        // }
        auto &bucket = buckets[bucket_idx[i]];
        // 畅：添加相关计算tagged_info操作
        size_t probe_tagged = ExtractTaggedInfo(hashes[i]);
        if ((probe_tagged | bucket->tagged_info_or) != probe_tagged) {
            // probe_state->bit_map[i] = 0;
            // spdlog::info("触发了tagged_info");
            continue;
        }
        // 下面没改
        int tuple_nums = bucket->tuple_nums;
        bool is_match = false;
        size_t hash_p = hashes[i];
        for(int j = 0; j < tuple_nums; j++) {
            auto entry_single = bucket->GetEntry(j);
            bool flag = true; //是否所有字段都满足
            if(hash_p != entry_single->hash_val) {
                flag = false;
                continue;
            }
            else {
                for(int k = 0; k < join_keys.size(); k++) {
                    switch (join_keys[k]->type_id()) {
                        case arrow::Type::INT32:
                        {
                            auto arr = std::static_pointer_cast<arrow::Int32Array>(join_keys[k]);
                            int left = arr->Value(i);
                            int right = entry_single->key[k]->GetValueUnsafe<int>();
                            if(left != right) {
                                flag = false;
                            }
                            break;
                        }
                        case arrow::Type::FLOAT:
                        {
                            auto arr = std::static_pointer_cast<arrow::FloatArray>(join_keys[k]);
                            float left = arr->Value(i);
                            float right = entry_single->key[k]->GetValueUnsafe<float>();
                            if(!AreAlmostEqual(left, right ,2)) {
                                flag = false;
                            }
                            break;
                        }
                        case arrow::Type::DOUBLE:
                        {
                            auto arr = std::static_pointer_cast<arrow::DoubleArray>(join_keys[k]);
                            double left = arr->Value(i);
                            double right = entry_single->key[k]->GetValueUnsafe<double>();
                            if(!AreAlmostEqual(left, right ,2)) {
                                flag = false;
                            }
                            break;
                        }
                        case arrow::Type::STRING:
                        {
                            auto arr = std::static_pointer_cast<arrow::StringArray>(join_keys[k]);
                            std::string left = arr->GetString(i);
                            std::string right = entry_single->key[k]->GetValueUnsafe<std::string>();
                            if(left != right) {
                                flag = false;
                            }
                            break;
                        }
                        default:
                            throw std::runtime_error("Invalid arrow::Type!!!");
                    } // switch
                    if(!flag)
                        break;
                } // for
            }
            if(flag){
                is_match = true;
                probe_state->AddMatch(i);
                // VALUE &row_data = entry_single->val;
                // AppendRowToValueVec(probe_state->left_result, row_data);
                // 畅
                probe_state->matched_entries.emplace_back(entry_single); 
            }
        } // for
        // probe_state->bit_map[i] = is_match ? 1 : 0;
        if (is_match) {
            probe_state->SetBit(i);
        }
    } // for
    return probe_state;
}

std::shared_ptr<ProbeState> JoinHashTable::GatherSemiData(std::vector<std::shared_ptr<arrow::Array>> &join_keys,
                                                      std::vector<int> &bucket_idx,
                                                      int count,
                                                      std::vector<size_t> &hashes) {
    std::shared_ptr<ProbeState> probe_state = std::make_shared<ProbeState>();
    // probe_state->bit_map.resize(count);
    probe_state->InitBitmap(count);
    probe_state->left_result.resize(schema->num_fields());
    for(int i = 0; i < count; i++) {
        if(!bloom_filter->IsInBloomFilter(hashes[i])) {
            // probe_state->bit_map[i] = 0;
            continue;
        }
        auto &bucket = buckets[bucket_idx[i]];
        int tuple_nums = bucket->tuple_nums;
        bool is_match = false;
        size_t hash_p = hashes[i];
        for(int j = 0; j < tuple_nums; j++) {
            auto entry_single = bucket->GetEntry(j);
            bool flag = true; //是否所有字段都满足
            if(hash_p != entry_single->hash_val) {
                flag = false;
                continue;
            }
            else {
                for(int k = 0; k < join_keys.size(); k++) {
                    if(comparison_types.empty()) {
                        switch (join_keys[k]->type_id()) {
                            case arrow::Type::INT32:
                            {
                                auto arr = std::static_pointer_cast<arrow::Int32Array>(join_keys[k]);
                                int left = arr->Value(i);
                                int right = entry_single->key[k]->GetValueUnsafe<int>();
                                if(left != right) {
                                    flag = false;
                                }
                                break;
                            }
                            case arrow::Type::FLOAT:
                            {
                                auto arr = std::static_pointer_cast<arrow::FloatArray>(join_keys[k]);
                                float left = arr->Value(i);
                                float right = entry_single->key[k]->GetValueUnsafe<float>();
                                if(!AreAlmostEqual(left, right ,2)) {
                                    flag = false;
                                }
                                break;
                            }
                            case arrow::Type::DOUBLE:
                            {
                                auto arr = std::static_pointer_cast<arrow::DoubleArray>(join_keys[k]);
                                double left = arr->Value(i);
                                double right = entry_single->key[k]->GetValueUnsafe<double>();
                                if(!AreAlmostEqual(left, right ,2)) {
                                    flag = false;
                                }
                                break;
                            }
                            case arrow::Type::STRING:
                            {
                                auto arr = std::static_pointer_cast<arrow::StringArray>(join_keys[k]);
                                std::string left = arr->GetString(i);
                                std::string right = entry_single->key[k]->GetValueUnsafe<std::string>();
                                if(left != right) {
                                    flag = false;
                                }
                                break;
                            }
                            default:
                                throw std::runtime_error("Invalid arrow::Type!!!");
                        } // switch
                        if(!flag)
                            break;
                    }
                    else {
                        // <, >, !=, =, >=, <=
                        switch (comparison_types[k]) {
                            case ExpressionTypes::COMPARE_LESSTHAN:
                            {
                                switch (join_keys[k]->type_id()) {
                                    case arrow::Type::INT32:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::Int32Array>(join_keys[k]);
                                        int left = arr->Value(i);
                                        int right = entry_single->key[k]->GetValueUnsafe<int>();
                                        if(left > right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::FLOAT:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::FloatArray>(join_keys[k]);
                                        float left = arr->Value(i);
                                        float right = entry_single->key[k]->GetValueUnsafe<float>();
                                        if(left > right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::DOUBLE:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::DoubleArray>(join_keys[k]);
                                        double left = arr->Value(i);
                                        double right = entry_single->key[k]->GetValueUnsafe<double>();
                                        if(left > right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::STRING:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::StringArray>(join_keys[k]);
                                        std::string left = arr->GetString(i);
                                        std::string right = entry_single->key[k]->GetValueUnsafe<std::string>();
                                        if(left > right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    default:
                                        throw std::runtime_error("Invalid arrow::Type!!!");
                                } // switch
                                break;
                            }
                            case ExpressionTypes::COMPARE_GREATERTHAN:
                            {
                                switch (join_keys[k]->type_id()) {
                                    case arrow::Type::INT32:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::Int32Array>(join_keys[k]);
                                        int left = arr->Value(i);
                                        int right = entry_single->key[k]->GetValueUnsafe<int>();
                                        if(left <= right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::FLOAT:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::FloatArray>(join_keys[k]);
                                        float left = arr->Value(i);
                                        float right = entry_single->key[k]->GetValueUnsafe<float>();
                                        if(left <= right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::DOUBLE:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::DoubleArray>(join_keys[k]);
                                        double left = arr->Value(i);
                                        double right = entry_single->key[k]->GetValueUnsafe<double>();
                                        if(left <= right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::STRING:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::StringArray>(join_keys[k]);
                                        std::string left = arr->GetString(i);
                                        std::string right = entry_single->key[k]->GetValueUnsafe<std::string>();
                                        if(left <= right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    default:
                                        throw std::runtime_error("Invalid arrow::Type!!!");
                                } // switch
                                break;
                            }
                            case ExpressionTypes::COMPARE_NOTEQUAL:
                            {
                                switch (join_keys[k]->type_id()) {
                                    case arrow::Type::INT32:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::Int32Array>(join_keys[k]);
                                        int left = arr->Value(i);
                                        int right = entry_single->key[k]->GetValueUnsafe<int>();
                                        if(left == right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::FLOAT:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::FloatArray>(join_keys[k]);
                                        float left = arr->Value(i);
                                        float right = entry_single->key[k]->GetValueUnsafe<float>();
                                        if(left == right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::DOUBLE:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::DoubleArray>(join_keys[k]);
                                        double left = arr->Value(i);
                                        double right = entry_single->key[k]->GetValueUnsafe<double>();
                                        if(left == right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::STRING:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::StringArray>(join_keys[k]);
                                        std::string left = arr->GetString(i);
                                        std::string right = entry_single->key[k]->GetValueUnsafe<std::string>();
                                        if(left == right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    default:
                                        throw std::runtime_error("Invalid arrow::Type!!!");
                                } // switch
                                break;
                            }
                            case ExpressionTypes::COMPARE_EQUAL:
                            {
                                switch (join_keys[k]->type_id()) {
                                    case arrow::Type::INT32:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::Int32Array>(join_keys[k]);
                                        int left = arr->Value(i);
                                        int right = entry_single->key[k]->GetValueUnsafe<int>();
                                        if(left != right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::FLOAT:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::FloatArray>(join_keys[k]);
                                        float left = arr->Value(i);
                                        float right = entry_single->key[k]->GetValueUnsafe<float>();
                                        if(left != right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::DOUBLE:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::DoubleArray>(join_keys[k]);
                                        double left = arr->Value(i);
                                        double right = entry_single->key[k]->GetValueUnsafe<double>();
                                        if(left != right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::STRING:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::StringArray>(join_keys[k]);
                                        std::string left = arr->GetString(i);
                                        std::string right = entry_single->key[k]->GetValueUnsafe<std::string>();
                                        if(left != right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    default:
                                        throw std::runtime_error("Invalid arrow::Type!!!");
                                } // switch
                                break;
                            }
                            case ExpressionTypes::COMPARE_GREATERTHANOREQUALTO:
                            {
                                switch (join_keys[k]->type_id()) {
                                    case arrow::Type::INT32:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::Int32Array>(join_keys[k]);
                                        int left = arr->Value(i);
                                        int right = entry_single->key[k]->GetValueUnsafe<int>();
                                        if(left < right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::FLOAT:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::FloatArray>(join_keys[k]);
                                        float left = arr->Value(i);
                                        float right = entry_single->key[k]->GetValueUnsafe<float>();
                                        if(left < right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::DOUBLE:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::DoubleArray>(join_keys[k]);
                                        double left = arr->Value(i);
                                        double right = entry_single->key[k]->GetValueUnsafe<double>();
                                        if(left < right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::STRING:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::StringArray>(join_keys[k]);
                                        std::string left = arr->GetString(i);
                                        std::string right = entry_single->key[k]->GetValueUnsafe<std::string>();
                                        if(left < right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    default:
                                        throw std::runtime_error("Invalid arrow::Type!!!");
                                } // switch
                                break;
                            }
                            case ExpressionTypes::COMPARE_LESSTHANOREQUALTO:
                            {
                                switch (join_keys[k]->type_id()) {
                                    case arrow::Type::INT32:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::Int32Array>(join_keys[k]);
                                        int left = arr->Value(i);
                                        int right = entry_single->key[k]->GetValueUnsafe<int>();
                                        if(left > right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::FLOAT:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::FloatArray>(join_keys[k]);
                                        float left = arr->Value(i);
                                        float right = entry_single->key[k]->GetValueUnsafe<float>();
                                        if(left > right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::DOUBLE:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::DoubleArray>(join_keys[k]);
                                        double left = arr->Value(i);
                                        double right = entry_single->key[k]->GetValueUnsafe<double>();
                                        if(left > right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::STRING:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::StringArray>(join_keys[k]);
                                        std::string left = arr->GetString(i);
                                        std::string right = entry_single->key[k]->GetValueUnsafe<std::string>();
                                        if(left > right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    default:
                                        throw std::runtime_error("Invalid arrow::Type!!!");
                                } // switch
                                break;
                            }
                            default:
                                throw std::runtime_error("Invalid Compare Type!!!");
                        } // switch
                        if(!flag)
                            break;
                    }
                } // for
            }
            if(flag){
                is_match = true;
                probe_state->AddMatch(i);
                break;
            }
        } // for
        // probe_state->bit_map[i] = is_match ? 1 : 0;
        if (is_match) {
            probe_state->SetBit(i);
        }
    } // for
    return probe_state;
}

std::shared_ptr<ProbeState> JoinHashTable::GatherAntiData(std::vector<std::shared_ptr<arrow::Array>> &join_keys,
                                                      std::vector<int> &bucket_idx,
                                                      int count,
                                                      std::vector<size_t> &hashes) {
    std::shared_ptr<ProbeState> probe_state = std::make_shared<ProbeState>();
    // probe_state->bit_map.resize(count);
    probe_state->InitBitmap(count);
    probe_state->left_result.resize(schema->num_fields());
    for(int i = 0; i < count; i++) {
        if(!bloom_filter->IsInBloomFilter(hashes[i])) {
            // probe_state->bit_map[i] = 0;
            continue;
        }
        auto &bucket = buckets[bucket_idx[i]];
        int tuple_nums = bucket->tuple_nums;
        std::vector<VALUE> res;
        bool is_match = false;
        size_t hash_p = hashes[i];
        for(int j = 0; j < tuple_nums; j++) {
            auto entry_single = bucket->GetEntry(j);
            bool flag = true; //是否所有字段都满足
            if(hash_p != entry_single->hash_val) {
                flag = false;
                continue;
            } else {
                for(int k = 0; k < join_keys.size(); k++) {
                    if(comparison_types.empty()) {
                        switch (join_keys[k]->type_id()) {
                            case arrow::Type::INT32:
                            {
                                auto arr = std::static_pointer_cast<arrow::Int32Array>(join_keys[k]);
                                int left = arr->Value(i);
                                int right = entry_single->key[k]->GetValueUnsafe<int>();
                                if(left != right) {
                                    flag = false;
                                }
                                break;
                            }
                            case arrow::Type::FLOAT:
                            {
                                auto arr = std::static_pointer_cast<arrow::FloatArray>(join_keys[k]);
                                float left = arr->Value(i);
                                float right = entry_single->key[k]->GetValueUnsafe<float>();
                                if(!AreAlmostEqual(left, right ,2)) {
                                    flag = false;
                                }
                                break;
                            }
                            case arrow::Type::DOUBLE:
                            {
                                auto arr = std::static_pointer_cast<arrow::DoubleArray>(join_keys[k]);
                                double left = arr->Value(i);
                                double right = entry_single->key[k]->GetValueUnsafe<double>();
                                if(!AreAlmostEqual(left, right ,2)) {
                                    flag = false;
                                }
                                break;
                            }
                            case arrow::Type::STRING:
                            {
                                auto arr = std::static_pointer_cast<arrow::StringArray>(join_keys[k]);
                                std::string left = arr->GetString(i);
                                std::string right = entry_single->key[k]->GetValueUnsafe<std::string>();
                                if(left != right) {
                                    flag = false;
                                }
                                break;
                            }
                            default:
                                throw std::runtime_error("Invalid arrow::Type!!!");
                        } // switch
                        if(!flag)
                            break;
                    } else {
                        // <, >, !=, =, >=, <=
                        switch (comparison_types[k]) {
                            case ExpressionTypes::COMPARE_LESSTHAN:
                            {
                                switch (join_keys[k]->type_id()) {
                                    case arrow::Type::INT32:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::Int32Array>(join_keys[k]);
                                        int left = arr->Value(i);
                                        int right = entry_single->key[k]->GetValueUnsafe<int>();
                                        if(left > right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::FLOAT:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::FloatArray>(join_keys[k]);
                                        float left = arr->Value(i);
                                        float right = entry_single->key[k]->GetValueUnsafe<float>();
                                        if(left > right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::DOUBLE:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::DoubleArray>(join_keys[k]);
                                        double left = arr->Value(i);
                                        double right = entry_single->key[k]->GetValueUnsafe<double>();
                                        if(left > right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::STRING:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::StringArray>(join_keys[k]);
                                        std::string left = arr->GetString(i);
                                        std::string right = entry_single->key[k]->GetValueUnsafe<std::string>();
                                        if(left > right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    default:
                                        throw std::runtime_error("Invalid arrow::Type!!!");
                                } // switch
                                break;
                            }
                            case ExpressionTypes::COMPARE_GREATERTHAN:
                            {
                                switch (join_keys[k]->type_id()) {
                                    case arrow::Type::INT32:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::Int32Array>(join_keys[k]);
                                        int left = arr->Value(i);
                                        int right = entry_single->key[k]->GetValueUnsafe<int>();
                                        if(left <= right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::FLOAT:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::FloatArray>(join_keys[k]);
                                        float left = arr->Value(i);
                                        float right = entry_single->key[k]->GetValueUnsafe<float>();
                                        if(left <= right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::DOUBLE:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::DoubleArray>(join_keys[k]);
                                        double left = arr->Value(i);
                                        double right = entry_single->key[k]->GetValueUnsafe<double>();
                                        if(left <= right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::STRING:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::StringArray>(join_keys[k]);
                                        std::string left = arr->GetString(i);
                                        std::string right = entry_single->key[k]->GetValueUnsafe<std::string>();
                                        if(left <= right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    default:
                                        throw std::runtime_error("Invalid arrow::Type!!!");
                                } // switch
                                break;
                            }
                            case ExpressionTypes::COMPARE_NOTEQUAL:
                            {
                                switch (join_keys[k]->type_id()) {
                                    case arrow::Type::INT32:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::Int32Array>(join_keys[k]);
                                        int left = arr->Value(i);
                                        int right = entry_single->key[k]->GetValueUnsafe<int>();
                                        if(left == right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::FLOAT:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::FloatArray>(join_keys[k]);
                                        float left = arr->Value(i);
                                        float right = entry_single->key[k]->GetValueUnsafe<float>();
                                        if(left == right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::DOUBLE:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::DoubleArray>(join_keys[k]);
                                        double left = arr->Value(i);
                                        double right = entry_single->key[k]->GetValueUnsafe<double>();
                                        if(left == right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::STRING:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::StringArray>(join_keys[k]);
                                        std::string left = arr->GetString(i);
                                        std::string right = entry_single->key[k]->GetValueUnsafe<std::string>();
                                        if(left == right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    default:
                                        throw std::runtime_error("Invalid arrow::Type!!!");
                                } // switch
                                break;
                            }
                            case ExpressionTypes::COMPARE_EQUAL:
                            {
                                switch (join_keys[k]->type_id()) {
                                    case arrow::Type::INT32:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::Int32Array>(join_keys[k]);
                                        int left = arr->Value(i);
                                        int right = entry_single->key[k]->GetValueUnsafe<int>();
                                        if(left != right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::FLOAT:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::FloatArray>(join_keys[k]);
                                        float left = arr->Value(i);
                                        float right = entry_single->key[k]->GetValueUnsafe<float>();
                                        if(left != right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::DOUBLE:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::DoubleArray>(join_keys[k]);
                                        double left = arr->Value(i);
                                        double right = entry_single->key[k]->GetValueUnsafe<double>();
                                        if(left != right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::STRING:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::StringArray>(join_keys[k]);
                                        std::string left = arr->GetString(i);
                                        std::string right = entry_single->key[k]->GetValueUnsafe<std::string>();
                                        if(left != right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    default:
                                        throw std::runtime_error("Invalid arrow::Type!!!");
                                } // switch
                                break;
                            }
                            case ExpressionTypes::COMPARE_GREATERTHANOREQUALTO:
                            {
                                switch (join_keys[k]->type_id()) {
                                    case arrow::Type::INT32:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::Int32Array>(join_keys[k]);
                                        int left = arr->Value(i);
                                        int right = entry_single->key[k]->GetValueUnsafe<int>();
                                        if(left < right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::FLOAT:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::FloatArray>(join_keys[k]);
                                        float left = arr->Value(i);
                                        float right = entry_single->key[k]->GetValueUnsafe<float>();
                                        if(left < right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::DOUBLE:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::DoubleArray>(join_keys[k]);
                                        double left = arr->Value(i);
                                        double right = entry_single->key[k]->GetValueUnsafe<double>();
                                        if(left < right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::STRING:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::StringArray>(join_keys[k]);
                                        std::string left = arr->GetString(i);
                                        std::string right = entry_single->key[k]->GetValueUnsafe<std::string>();
                                        if(left < right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    default:
                                        throw std::runtime_error("Invalid arrow::Type!!!");
                                } // switch
                                break;
                            }
                            case ExpressionTypes::COMPARE_LESSTHANOREQUALTO:
                            {
                                switch (join_keys[k]->type_id()) {
                                    case arrow::Type::INT32:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::Int32Array>(join_keys[k]);
                                        int left = arr->Value(i);
                                        int right = entry_single->key[k]->GetValueUnsafe<int>();
                                        if(left > right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::FLOAT:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::FloatArray>(join_keys[k]);
                                        float left = arr->Value(i);
                                        float right = entry_single->key[k]->GetValueUnsafe<float>();
                                        if(left > right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::DOUBLE:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::DoubleArray>(join_keys[k]);
                                        double left = arr->Value(i);
                                        double right = entry_single->key[k]->GetValueUnsafe<double>();
                                        if(left > right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    case arrow::Type::STRING:
                                    {
                                        auto arr = std::static_pointer_cast<arrow::StringArray>(join_keys[k]);
                                        std::string left = arr->GetString(i);
                                        std::string right = entry_single->key[k]->GetValueUnsafe<std::string>();
                                        if(left > right) {
                                            flag = false;
                                        }
                                        break;
                                    }
                                    default:
                                        throw std::runtime_error("Invalid arrow::Type!!!");
                                } // switch
                                break;
                            }
                            default:
                                throw std::runtime_error("Invalid Compare Type!!!");
                        } // switch
                        if(!flag)
                            break;
                    }
                } // for
            }
            if(flag){
                is_match = true;
            }
        } // for
        // probe_state->bit_map[i] = is_match ? 1 : 0;
        if (is_match) {
            probe_state->SetBit(i);
        }
    } // for
    return probe_state;
}

template<JoinTypes join_type>
void JoinHashTable::GetJoinResult(std::shared_ptr<arrow::RecordBatch> &chunk, std::shared_ptr<arrow::RecordBatch> &result, std::shared_ptr<ProbeState> &probe_state, std::shared_ptr<ProbeState> &last_probe_state, bool is_final_probe) {
    throw std::runtime_error("Unknown JoinType");
}

template<>
void JoinHashTable::GetJoinResult<JoinTypes::INNER>(std::shared_ptr<arrow::RecordBatch> &chunk, std::shared_ptr<arrow::RecordBatch> &result, std::shared_ptr<ProbeState> &probe_state, std::shared_ptr<ProbeState> &last_probe_state, bool is_final_probe) {
    auto &match = probe_state->match_row_idx;
    if(match.empty()) {
        return;
    }
     
    int col_num_L = schema->num_fields();
    std::vector<std::vector<std::shared_ptr<Value>>> left_result;
    left_result.resize(col_num_L);

    for (auto &entry : probe_state->matched_entries) {
        const auto &val = entry->val;
        if (val.size() != col_num_L) {
            throw std::runtime_error("Mismatched entry column count during materialization");
        }
        for (int i = 0; i < col_num_L; i++) {
            left_result[i].emplace_back(val[i]);
        }
    }

    std::vector<std::shared_ptr<arrow::Array>> build_columns;
    ValueVecToArrayVec(left_result, build_columns);

    std::shared_ptr<arrow::RecordBatch> rbR;
    Util::RecordBatchSlice2(chunk, rbR, match);

    // int col_num_L = probe_state->left_result.size();

    int col_num_R = rbR->num_columns();
    std::vector<std::shared_ptr<arrow::Array>> columns;
    for(int i = 0; i < col_num_L; i++) {
        auto &col = build_columns[i];
        columns.emplace_back(col);
    }
    for(int i = 0; i < col_num_R; i++) {
        auto col = rbR->column(i);
        columns.emplace_back(col);
    }
    std::shared_ptr<arrow::Schema> &leftS = schema;
    std::shared_ptr<arrow::Schema> rightS = rbR->schema();
    std::shared_ptr<arrow::Schema> schemaM = Util::MergeTwoSchema(leftS, rightS);
    result = arrow::RecordBatch::Make(schemaM, rbR->num_rows(), columns);
}

template<>
void JoinHashTable::GetJoinResult<JoinTypes::LEFT>(std::shared_ptr<arrow::RecordBatch> &chunk, std::shared_ptr<arrow::RecordBatch> &result, std::shared_ptr<ProbeState> &probe_state, std::shared_ptr<ProbeState> &last_probe_state, bool is_final_probe) {
    auto &match = probe_state->match_row_idx;
    if(last_probe_state != nullptr) {
        // auto &bit_map = probe_state->bit_map;
        // auto &last_bit_map = last_probe_state->bit_map;
        // int size_map = bit_map.size();
        // if(!last_bit_map.empty()) {
        //     for(int i = 0; i < size_map; i++) {
        //         bit_map[i] |= last_bit_map[i];
        //     }
        // }
        auto &bm = probe_state->bit_map_packed;
        auto &lbm = last_probe_state->bit_map_packed;
        size_t min_words = std::min(bm.size(), lbm.size());
        for (size_t i = 0; i < min_words; ++i) {
            bm[i] |= lbm[i];
        }
        if(is_final_probe) {
            // for(int i = 0; i < size_map; i++) {
            //     if(bit_map[i] == 0) {
            //         AppendNullRowToValueVec(probe_state->left_result, schema);
            //         match.emplace_back(i);
            //     }
            // }
            int total_rows = chunk->num_rows();
            for (int i = 0; i < total_rows; ++i) {
                if (!probe_state->GetBit(i)) {
                    AppendNullRowToValueVec(probe_state->left_result, schema);
                    match.emplace_back(i);
                }
            }
        } // if(is_final_probe)
    } // if(last_probe_state != nullptr)
    else {
        if(is_final_probe) {
            // auto &bit_map = probe_state->bit_map;
            // int size_map = bit_map.size();
            // for(int i = 0; i < size_map; i++) {
            //     if(bit_map[i] == 0) {
            //         AppendNullRowToValueVec(probe_state->left_result, schema);
            //         match.emplace_back(i);
            //     }
            // }
            int total_rows = chunk->num_rows();
            for (int i = 0; i < total_rows; ++i) {
                if (!probe_state->GetBit(i)) {
                    AppendNullRowToValueVec(probe_state->left_result, schema);
                    match.emplace_back(i);
                }
            }
        }
    }
    if(match.empty()) {
        return;
    }
    std::vector<std::shared_ptr<arrow::Array>> build_columns;
    ValueVecToArrayVecWithNull(probe_state->left_result, build_columns);
    std::shared_ptr<arrow::RecordBatch> rbR;
    Util::RecordBatchSlice2(chunk, rbR, match);
    int col_num_L = probe_state->left_result.size();
    int col_num_R = rbR->num_columns();
    std::vector<std::shared_ptr<arrow::Array>> columns;
    for(int i = 0; i < col_num_L; i++) {
        auto &col = build_columns[i];
        columns.emplace_back(col);
    }
    for(int i = 0; i < col_num_R; i++) {
        auto col = rbR->column(i);
        columns.emplace_back(col);
    }
    std::shared_ptr<arrow::Schema> &leftS = schema;
    std::shared_ptr<arrow::Schema> rightS = rbR->schema();
    std::shared_ptr<arrow::Schema> schemaM = Util::MergeTwoSchema(leftS, rightS);
    result = arrow::RecordBatch::Make(schemaM, rbR->num_rows(), columns);
//    spdlog::info("[{} : {}] LEFT 结果: {}", __FILE__, __LINE__, result->ToString());
}

// Right JOIN实现逻辑同LEFT，只有Build和Probe的表序不同，Right JOIN必须右表为Probe表，而LEFT JOIN必须左表为Probe表
template<>
void JoinHashTable::GetJoinResult<JoinTypes::RIGHT>(std::shared_ptr<arrow::RecordBatch> &chunk, std::shared_ptr<arrow::RecordBatch> &result, std::shared_ptr<ProbeState> &probe_state, std::shared_ptr<ProbeState> &last_probe_state, bool is_final_probe) {
    auto &match = probe_state->match_row_idx;
    if(last_probe_state != nullptr) {
        // auto &bit_map = probe_state->bit_map;
        // auto &last_bit_map = last_probe_state->bit_map;
        // int size_map = bit_map.size();
        // for(int i = 0; i < size_map; i++) {
        //     bit_map[i] |= last_bit_map[i];
        // }
        auto &bm = probe_state->bit_map_packed;
        auto &lbm = last_probe_state->bit_map_packed;
        size_t min_words = std::min(bm.size(), lbm.size());
        for (size_t i = 0; i < min_words; ++i) {
            bm[i] |= lbm[i];
        }
        if(is_final_probe) {
            // for(int i = 0; i < size_map; i++) {
            //     if(bit_map[i] == 0) {
            //         AppendNullRowToValueVec(probe_state->left_result, schema);
            //         match.emplace_back(i);
            //     }
            // }
            int total_rows = chunk->num_rows();
            for (int i = 0; i < total_rows; ++i) {
                if (!probe_state->GetBit(i)) {
                    AppendNullRowToValueVec(probe_state->left_result, schema);
                    match.emplace_back(i);
                }
            }
        } // if(is_final_probe)
    } // if(last_probe_state != nullptr)
    else {
        if(is_final_probe) {
            // auto &bit_map = probe_state->bit_map;
            // int size_map = bit_map.size();
            // for(int i = 0; i < size_map; i++) {
            //     if(bit_map[i] == 0) {
            //         AppendNullRowToValueVec(probe_state->left_result, schema);
            //         match.emplace_back(i);
            //     }
            // }
            int total_rows = chunk->num_rows();
            for (int i = 0; i < total_rows; ++i) {
                if (!probe_state->GetBit(i)) {
                    AppendNullRowToValueVec(probe_state->left_result, schema);
                    match.emplace_back(i);
                }
            }
        }
    }
    if(match.empty()) {
        return;
    }
    std::vector<std::shared_ptr<arrow::Array>> build_columns;
    ValueVecToArrayVecWithNull(probe_state->left_result, build_columns);
    std::shared_ptr<arrow::RecordBatch> rbR;
    Util::RecordBatchSlice2(chunk, rbR, match);
    int col_num_L = probe_state->left_result.size();
    int col_num_R = rbR->num_columns();
    std::vector<std::shared_ptr<arrow::Array>> columns;
    for(int i = 0; i < col_num_L; i++) {
        auto &col = build_columns[i];
        columns.emplace_back(col);
    }
    for(int i = 0; i < col_num_R; i++) {
        auto col = rbR->column(i);
        columns.emplace_back(col);
    }
    std::shared_ptr<arrow::Schema> &leftS = schema;
    std::shared_ptr<arrow::Schema> rightS = rbR->schema();
    std::shared_ptr<arrow::Schema> schemaM = Util::MergeTwoSchema(leftS, rightS);
    result = arrow::RecordBatch::Make(schemaM, rbR->num_rows(), columns);
}

template<>
void JoinHashTable::GetJoinResult<JoinTypes::OUTER>(std::shared_ptr<arrow::RecordBatch> &chunk, std::shared_ptr<arrow::RecordBatch> &result, std::shared_ptr<ProbeState> &probe_state, std::shared_ptr<ProbeState> &last_probe_state, bool is_final_probe) {
    throw std::runtime_error("Unknown JoinType");
}

// 内部表为Build表，返回Probe为true的外部表字段
template<>
void JoinHashTable::GetJoinResult<JoinTypes::SEMI>(std::shared_ptr<arrow::RecordBatch> &chunk, std::shared_ptr<arrow::RecordBatch> &result, std::shared_ptr<ProbeState> &probe_state, std::shared_ptr<ProbeState> &last_probe_state, bool is_final_probe) {
    // TODO: 去重
    auto &match = probe_state->match_row_idx;
    if(match.empty()) {
        return;
    }
    Util::RecordBatchSlice2(chunk, result, match);
}

// 外部表为Probe表，返回Probe为false的外部表字段
template<>
void JoinHashTable::GetJoinResult<JoinTypes::ANTI>(std::shared_ptr<arrow::RecordBatch> &chunk, std::shared_ptr<arrow::RecordBatch> &result, std::shared_ptr<ProbeState> &probe_state, std::shared_ptr<ProbeState> &last_probe_state, bool is_final_probe) {
    std::vector<int> no_match;
    if(last_probe_state != nullptr) {
        // auto &bit_map = probe_state->bit_map;
        // auto &last_bit_map = last_probe_state->bit_map;
        // int size_map = bit_map.size();
        // if(!last_bit_map.empty()) {
        //     if(bit_map.empty()) {
        //         bit_map = last_bit_map;
        //     } else {
        //         for(int i = 0; i < size_map; i++) {
        //             bit_map[i] |= last_bit_map[i];
        //         }
        //     }
        // }
        auto &bm = probe_state->bit_map_packed;
        auto &lbm = last_probe_state->bit_map_packed;
        size_t min_words = std::min(bm.size(), lbm.size());
        for (size_t i = 0; i < min_words; ++i) {
            bm[i] |= lbm[i];
        }
        if(is_final_probe) {
            int total_rows = chunk->num_rows();
            for (int i = 0; i < total_rows; ++i) {
                if (!probe_state->GetBit(i)) {
                    no_match.emplace_back(i);
                }
            }
            // for(int i = 0; i < bit_map.size(); i++) {
            //     if(bit_map[i] == 0) {
            //         no_match.emplace_back(i);
            //     }
            // }
        } // if(is_final_probe)
    } // if(last_probe_state != nullptr)
    else {
        if(is_final_probe) {
            // auto &bit_map = probe_state->bit_map;
            // int size_map = bit_map.size();
            // for(int i = 0; i < size_map; i++) {
            //     if(bit_map[i] == 0) {
            //         no_match.emplace_back(i);
            //     }
            // }
            int total_rows = chunk->num_rows();
            for (int i = 0; i < total_rows; ++i) {
                if (!probe_state->GetBit(i)) {
                    no_match.emplace_back(i);
                }
            }
        } // if(is_final_probe)
    }
    if(no_match.empty()) {
        return;
    }
    Util::RecordBatchSlice2(chunk, result, no_match);
    // spdlog::info("[{} : {}] ANTI size结果: {}", __FILE__, __LINE__, result->num_rows());
}

void JoinHashTable::GetSingleNullRow() {
    Util::GetSinlgeRowNull(null_row, schema);
}

} // DaseX
