#include "like.hpp"
#include "string_functions.hpp"
#include "common.hpp"
#include "bound_constant_expression.hpp"

namespace DaseX {

template <class UNSIGNED, int NEEDLE_SIZE>
static idx_t ContainsUnaligned(const unsigned char *haystack, idx_t haystack_size, const unsigned char *needle,
                               idx_t base_offset) {
    if (NEEDLE_SIZE > haystack_size) {
        // needle is bigger than haystack: haystack cannot contain needle
        return DConstants::INVALID_INDEX;
    }
    UNSIGNED needle_entry = 0;
    UNSIGNED haystack_entry = 0;
    const UNSIGNED start = (sizeof(UNSIGNED) * 8) - 8;
    const UNSIGNED shift = (sizeof(UNSIGNED) - NEEDLE_SIZE) * 8;
    for (int i = 0; i < NEEDLE_SIZE; i++) {
        needle_entry |= UNSIGNED(needle[i]) << UNSIGNED(start - i * 8);
        haystack_entry |= UNSIGNED(haystack[i]) << UNSIGNED(start - i * 8);
    }
    // now we perform the actual search
    for (idx_t offset = NEEDLE_SIZE; offset < haystack_size; offset++) {
        // for this position we first compare the haystack with the needle
        if (haystack_entry == needle_entry) {
            return base_offset + offset - NEEDLE_SIZE;
        }
        haystack_entry = (haystack_entry << 8) | ((UNSIGNED(haystack[offset])) << shift);
    }
    if (haystack_entry == needle_entry) {
        return base_offset + haystack_size - NEEDLE_SIZE;
    }
    return DConstants::INVALID_INDEX;
}

template <class UNSIGNED>
static idx_t ContainsAligned(const unsigned char *haystack, idx_t haystack_size, const unsigned char *needle,
                             idx_t base_offset) {
    if (sizeof(UNSIGNED) > haystack_size) {
        // needle is bigger than haystack: haystack cannot contain needle
        return DConstants::INVALID_INDEX;
    }
    // contains for a small needle aligned with unsigned integer (2/4/8)
    // similar to ContainsUnaligned, but simpler because we only need to do a reinterpret cast
    auto needle_entry = Load<UNSIGNED>(needle);
    for (idx_t offset = 0; offset <= haystack_size - sizeof(UNSIGNED); offset++) {
        // for this position we first compare the haystack with the needle
        auto haystack_entry = Load<UNSIGNED>(haystack + offset);
        if (needle_entry == haystack_entry) {
            return base_offset + offset;
        }
    }
    return DConstants::INVALID_INDEX;
}

idx_t ContainsGeneric(const unsigned char *haystack, idx_t haystack_size, const unsigned char *needle,
                      idx_t needle_size, idx_t base_offset) {
    if (needle_size > haystack_size) {
        // needle is bigger than haystack: haystack cannot contain needle
        return DConstants::INVALID_INDEX;
    }
    uint32_t sums_diff = 0;
    for (idx_t i = 0; i < needle_size; i++) {
        sums_diff += haystack[i];
        sums_diff -= needle[i];
    }
    idx_t offset = 0;
    while (true) {
        if (sums_diff == 0 && haystack[offset] == needle[0]) {
            if (memcmp(haystack + offset, needle, needle_size) == 0) {
                return base_offset + offset;
            }
        }
        if (offset >= haystack_size - needle_size) {
            return DConstants::INVALID_INDEX;
        }
        sums_diff -= haystack[offset];
        sums_diff += haystack[offset + needle_size];
        offset++;
    }
}

idx_t ContainsFun::Find(const unsigned char *haystack, idx_t haystack_size, const unsigned char *needle, idx_t needle_size) {
    // start off by performing a memchr to find the first character of the
    auto location = memchr(haystack, needle[0], haystack_size);
    if (location == nullptr) {
        return DConstants::INVALID_INDEX;
    }
    idx_t base_offset = const_uchar_ptr_cast(location) - haystack;
    haystack_size -= base_offset;
    haystack = const_uchar_ptr_cast(location);
    // switch algorithm depending on needle size
    switch (needle_size) {
        case 1:
            return base_offset;
        case 2:
            return ContainsAligned<uint16_t>(haystack, haystack_size, needle, base_offset);
        case 3:
            return ContainsUnaligned<uint32_t, 3>(haystack, haystack_size, needle, base_offset);
        case 4:
            return ContainsAligned<uint32_t>(haystack, haystack_size, needle, base_offset);
        case 5:
            return ContainsUnaligned<uint64_t, 5>(haystack, haystack_size, needle, base_offset);
        case 6:
            return ContainsUnaligned<uint64_t, 6>(haystack, haystack_size, needle, base_offset);
        case 7:
            return ContainsUnaligned<uint64_t, 7>(haystack, haystack_size, needle, base_offset);
        case 8:
            return ContainsAligned<uint64_t>(haystack, haystack_size, needle, base_offset);
        default:
            return ContainsGeneric(haystack, haystack_size, needle, needle_size, base_offset);
    }
}

// TODO: 这里的执行器是逐元素操作，应该实现一个向量化的版本，即直接对数组进行操作，否则会大量重复调用Match方法，后续需要优化
bool LikeMatcher::Match(std::string &str) {
    auto str_data = const_uchar_ptr_cast(str.c_str());
    auto str_len = str.size();
    idx_t segment_idx = 0;
    idx_t end_idx = segments.size() - 1;
    if (!has_start_percentage) {
        // no start sample_size: match the first part of the string directly
        auto &segment = segments[0];
        if (str_len < segment.pattern.size()) {
            return false;
        }
        if (memcmp(str_data, segment.pattern.c_str(), segment.pattern.size()) != 0) {
            return false;
        }
        str_data += segment.pattern.size();
        str_len -= segment.pattern.size();
        segment_idx++;
        if (segments.size() == 1) {
            // only one segment, and it matches
            // we have a match if there is an end sample_size, OR if the memcmp was an exact match (remaining str is
            // empty)
            return has_end_percentage || str_len == 0;
        }
    }
    // main match loop: for every segment in the middle, use Contains to find the needle in the haystack
    for (; segment_idx < end_idx; segment_idx++) {
        auto &segment = segments[segment_idx];
        // find the pattern of the current segment
        idx_t next_offset = ContainsFun::Find(str_data, str_len, const_uchar_ptr_cast(segment.pattern.c_str()),
                                              segment.pattern.size());
        if (next_offset == DConstants::INVALID_INDEX) {
            // could not find this pattern in the string: no match
            return false;
        }
        idx_t offset = next_offset + segment.pattern.size();
        str_data += offset;
        str_len -= offset;
    }
    if (!has_end_percentage) {
        end_idx--;
        // no end sample_size: match the final segment now
        auto &segment = segments.back();
        if (str_len < segment.pattern.size()) {
            return false;
        }
        if (memcmp(str_data + str_len - segment.pattern.size(), segment.pattern.c_str(), segment.pattern.size()) != 0) {
            return false;
        }
        return true;
    } else {
        auto &segment = segments.back();
        // find the pattern of the current segment
        idx_t next_offset = ContainsFun::Find(str_data, str_len, const_uchar_ptr_cast(segment.pattern.c_str()),
                                              segment.pattern.size());
        return next_offset != DConstants::INVALID_INDEX;
    }
} // Match

std::shared_ptr<LikeMatcher> LikeMatcher::CreateLikeMatcher(std::string like_pattern, char escape) {
    std::vector<LikeSegment> segments;
    int last_non_pattern = 0;
    bool has_start_percentage = false;
    bool has_end_percentage = false;
    for (int i = 0; i < like_pattern.size(); i++) {
        auto ch = like_pattern[i];
        if (ch == escape || ch == '%' || ch == '_') {
            if (i > last_non_pattern) {
                segments.emplace_back(like_pattern.substr(last_non_pattern, i - last_non_pattern));
            }
            last_non_pattern = i + 1;
            if (ch == escape || ch == '_') {
                return nullptr;
            } else {
                // sample_size
                if (i == 0) {
                    has_start_percentage = true;
                }
                if (i + 1 == like_pattern.size()) {
                    has_end_percentage = true;
                }
            }
        }
    }
    if (last_non_pattern < like_pattern.size()) {
        segments.emplace_back(like_pattern.substr(last_non_pattern, like_pattern.size() - last_non_pattern));
    }
    if (segments.empty()) {
        return nullptr;
    }
    return std::make_shared<LikeMatcher>(std::move(like_pattern), std::move(segments), has_start_percentage, has_end_percentage);
} // CreateLikeMatcher

std::shared_ptr<FunctionData> LikeMatcher::Copy() const {
    return std::make_shared<LikeMatcher>(like_pattern, segments, has_start_percentage, has_end_percentage);
}

// TODO: 从表达式中获取pattern，并根据pattern创建LikeMatcher
static std::shared_ptr<FunctionData> LikeBindFunction(ScalarFunction &bound_function, std::vector<std::shared_ptr<Expression>> &arguments) {
    // TODO:这里需要判断表达式类型，以及参数，不然容易出错，后续优化
    auto &expression = (*(arguments[1])).Cast<BoundConstantExpression>();
    Value pattern_str = expression.value;
    if (pattern_str.is_null) {
        return nullptr;
    }
    return LikeMatcher::CreateLikeMatcher(pattern_str.GetValueUnsafe<std::string>());
}

ScalarFunction LikeFun::GetLikeFunction() {
    return ScalarFunction("~~", {LogicalType::STRING, LogicalType::STRING}, LogicalType::BOOL, RegularLikeFunction<false>, LikeBindFunction);
}

ScalarFunction NotLikeFun::GetNotLikeFunction() {
    return ScalarFunction("!~~", {LogicalType::STRING, LogicalType::STRING}, LogicalType::BOOL, RegularLikeFunction<true>, LikeBindFunction);
}

} // DaseX

