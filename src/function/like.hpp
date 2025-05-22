#pragma once

#include "expression_executor_state.hpp"
#include "bound_function_expression.hpp"
#include "unary_executor.hpp"
#include "typedefs.hpp"
#include <memory>


namespace DaseX {

struct DConstants {
    //! The value used to signify an invalid index entry
    static constexpr const idx_t INVALID_INDEX = idx_t(-1);
};

class LikeSegment {
public:
    std::string pattern;
    explicit LikeSegment(std::string pattern) : pattern(std::move(pattern)) {}
}; // LikeSegment

class LikeMatcher : public FunctionData {
public:
    std::string like_pattern;
    std::vector<LikeSegment> segments;
    bool has_start_percentage;
    bool has_end_percentage;
public:
    LikeMatcher(std::string like_pattern_p, std::vector<LikeSegment> segments, bool has_start_percentage, bool has_end_percentage)
            : like_pattern(std::move(like_pattern_p)), segments(std::move(segments)),
              has_start_percentage(has_start_percentage), has_end_percentage(has_end_percentage) {
    }
public:
    bool Match(std::string &str);

    static std::shared_ptr<LikeMatcher> CreateLikeMatcher(std::string like_pattern, char escape = '\0');
    std::shared_ptr<FunctionData> Copy() const override;
}; // LikeMatcher

template <bool INVERT>
static void RegularLikeFunction(std::shared_ptr<arrow::Array> &input, ExpressionState &state, std::vector<int> &result) {
    auto &func_expr = (*(state.expr)).Cast<BoundFunctionExpression>();
    auto &matcher = (*(func_expr.bind_info)).Cast<LikeMatcher>();
    UnaryExecutor::Execute<std::string, bool>(input, result, input->length(), [&](std::string input) {
        return INVERT ? !matcher.Match(input) : matcher.Match(input);
    });
} // RegularLikeFunction

} // DaseX
