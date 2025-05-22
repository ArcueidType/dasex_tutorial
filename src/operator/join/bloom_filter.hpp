//
// Created by cwl on 11/27/24.
//

#pragma once

#include "config.hpp"
#include <vector>
#include <string>

namespace DaseX {

class BitSet {
private:
    std::vector<int> _bitset;
    int bitCount;// 开num比特位的空间
public:
    BitSet(size_t bitCount) : bitCount(bitCount) {
        //resize会将vector中的元素初始化为0
        _bitset.resize(bitCount / 32 + 1);
    }
    // 每个字节用0和1记录某个数字存在状态
    // 把x所在的数据在位图的那一位变成1
    void Set(int x);

    // 把x所在的数据在位图的那一位变成0
    void ReSet(int x);

    // 判断x是否存在
    bool IsExist(int x);
};

class BloomFilter {
private:
    BitSet _bs;
    size_t _N;// 能够映射元素个数
public:
    // 布隆过滤器的长度 近似等于4.3~5倍插入元素的个数
    // 这里取 5    2e20 = 1048576
    BloomFilter(size_t size = BLOOM_FILTER_SIZE) :_bs(5 * size), _N(5 * size) {}
    // 哈希函数：使用一个基础哈希函数并添加扰动值
    size_t hashFunction(size_t hash_val, size_t seed) const;
    void Add(const size_t hash_val);
    bool IsInBloomFilter(const size_t hash_val);
};


} // DaseX