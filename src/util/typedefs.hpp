#pragma once

#include <cstdint>

namespace DaseX {

//! a saner size_t for loop indices etc
typedef uint64_t idx_t;

//! data pointers
typedef uint8_t data_t;
typedef data_t *data_ptr_t;
typedef const data_t *const_data_ptr_t;

template <class SRC>
data_ptr_t data_ptr_cast(SRC *src) {
    return reinterpret_cast<data_ptr_t>(src);
}

template <class SRC>
const_data_ptr_t const_data_ptr_cast(const SRC *src) {
    return reinterpret_cast<const_data_ptr_t>(src);
}

template <class SRC>
char *char_ptr_cast(SRC *src) {
    return reinterpret_cast<char *>(src);
}

template <class SRC>
const char *const_char_ptr_cast(const SRC *src) {
    return reinterpret_cast<const char *>(src);
}

template <class SRC>
const unsigned char *const_uchar_ptr_cast(const SRC *src) {
    return reinterpret_cast<const unsigned char *>(src);
}

template <class SRC>
uintptr_t CastPointerToValue(SRC *src) {
    return uintptr_t(src);
}

} // DaseX
