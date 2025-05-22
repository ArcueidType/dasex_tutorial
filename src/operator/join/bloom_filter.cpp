//
// Created by cwl on 11/27/24.
//
#include "bloom_filter.hpp"

namespace DaseX {

void BitSet::Set(int x) {
    if (x > bitCount) return;
    int index = x / 32;// x是第index个整数
    int pos = x % 32;// x是第index个整数的第pos个位
    _bitset[index] |= (1 << pos);
}

// 把x所在的数据在位图的那一位变成0
void BitSet::ReSet(int x) {
    if (x > bitCount) return;
    int index = x / 32;// x是第index个整数
    int pos = x % 32;// x是滴index个整数的第pos个位
    _bitset[index] &= ~(1 << pos);
}

// 判断x是否存在
bool BitSet::IsExist(int x) {
    if (x > bitCount) return false;
    int index = x / 32;// x是第index个整数
    int pos = x % 32;// x是滴index个整数的第pos个位
    return _bitset[index] & (1 << pos);
}

size_t BloomFilter::hashFunction(size_t hash_val, size_t seed) const {
    std::hash<std::string> hashFn;
    return hash_val ^ (seed * 0x5bd1e995);
}

void BloomFilter::Add(const size_t hash_val)
{
    size_t index1 = hashFunction(hash_val, 0) % _N;
    size_t index2 = hashFunction(hash_val, 1) % _N;
    size_t index3 = hashFunction(hash_val, 2) % _N;

    _bs.Set(index1);
    _bs.Set(index2);
    _bs.Set(index3);
}

bool BloomFilter::IsInBloomFilter(const size_t hash_val)
{
    size_t index1 = hashFunction(hash_val, 0) % _N;
    size_t index2 = hashFunction(hash_val, 1) % _N;
    size_t index3 = hashFunction(hash_val, 2) % _N;

    return _bs.IsExist(index1)
           && _bs.IsExist(index2)
           && _bs.IsExist(index3);// 可能会误报，判断在是不准确的，判断不在是准确的
}

} // DaseX