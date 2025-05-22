//
// Created by zyy on 24-8-27.
//

#pragma once

#include "../bound_expression.h"
#include "spdlog/fmt/bundled/format.h"
#include <algorithm>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace DaseX {


    struct CaseCheck {
        std::unique_ptr<BoundExpression> when_expr;
        std::unique_ptr<BoundExpression> then_expr;

    };
/**
 * A bound case when expression, e.g., `CASE ... WHEN ... THEN ... ELSE ...` .
 */
    class BoundCase : public BoundExpression {
    public:
    explicit BoundCase() : BoundExpression(ExpressionType::CASE)
                  {}


        std::vector<CaseCheck> case_checks;
        std::unique_ptr<BoundExpression> else_expr;
    };


	template <class T, class BASE>
	static std::string ToString(const T &entry) {
		std::string case_str = "CASE ";
		for (auto &check : entry.case_checks) {
			case_str += " WHEN (" + check.when_expr->ToString() + ")";
			case_str += " THEN (" + check.then_expr->ToString() + ")";
		}
		case_str += " ELSE " + entry.else_expr->ToString();
		case_str += " END";
		return case_str;
	}
} // namespace DaseX
