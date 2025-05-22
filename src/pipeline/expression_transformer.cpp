#include "expression_transformer.hpp"

#include <memory>

#include "bound_case_expression.hpp"
#include "bound_columnref_expression.hpp"
#include "bound_comparison_expression.hpp"
#include "bound_conjunction_expression.hpp"
#include "bound_constant_expression.hpp"
#include "bound_function_expression.hpp"
#include "calc_expression.h"
#include "case_expression.h"
#include "column_value_expression.h"
#include "compare_expression.h"
#include "constant_value_expression.h"
#include "exception.h"
#include "expression_type.hpp"
#include "like_expression.h"
#include "logic_expression.h"
#include "calc_expression.h"
#include "like_expression.h"
#include "bound_function_expression.hpp"
#include "string_functions.hpp"
#include "llm_expression.h"
#include "llm_call.hpp"

namespace DaseX {

auto ExpressionTransformer::Transform(AbstractExpressionRef expr)
	-> ExpressionRef {
	switch (expr->GetExpressionClass()) {
		/// @todo: code clean up
		case ExpressionClass::COLUMN_REF: {
			auto column_expr = expr->Cast<ColumnValueExpression>();

			return std::make_shared<BoundColumnRefExpression>(
				expr->GetReturnType(), column_expr.GetTupleIdx(),
				column_expr.GetColIdx(),0);
		}
		case ExpressionClass::CONSTANT: {

			auto const_expr = expr->Cast<ConstantValueExpression>();
			return std::make_shared<BoundConstantExpression>(const_expr.val_);
		}
		case ExpressionClass::COMPARISON: {
			auto comp_expr = expr->Cast<ComparisonExpression>();
			auto left = Transform(comp_expr.children_[0]);
			auto right = Transform(comp_expr.children_[1]);
			auto com_type = ExpressionTypes::COMPARE_EQUAL;
			switch (comp_expr.comp_type_) {
				case ComparisonType::Equal: {
					com_type = ExpressionTypes::COMPARE_EQUAL;
					break;
				}
				case ComparisonType::NotEqual: {
					com_type = ExpressionTypes::COMPARE_NOTEQUAL;
					break;
				}
				case ComparisonType::GreaterThan: {
					com_type = ExpressionTypes::COMPARE_GREATERTHAN;
					break;
				}
				case ComparisonType::GreaterThanOrEqual: {
					com_type = ExpressionTypes::COMPARE_GREATERTHANOREQUALTO;
					break;
				}
				case ComparisonType::LessThan: {
					com_type = ExpressionTypes::COMPARE_LESSTHAN;
					break;
				}
				case ComparisonType::LessThanOrEqual: {
					com_type = ExpressionTypes::COMPARE_LESSTHANOREQUALTO;
					break;
				}
			}
			return std::make_shared<BoundComparisonExpression>(com_type, left,
															   right);
		}
		case ExpressionClass::CONJUNCTION: {
			auto logic_expr = expr->Cast<LogicExpression>();
			auto logic_type = logic_expr.logic_type_;
			auto con_type = ExpressionTypes::CONJUNCTION_AND;
			switch (logic_type) {
				case LogicType::And: {
					con_type = ExpressionTypes::CONJUNCTION_AND;
					break;
				}
				case LogicType::Or: {
					con_type = ExpressionTypes::CONJUNCTION_OR;
					break;
				}
			}
			auto left = Transform(logic_expr.children_[0]);
			auto right = Transform(logic_expr.children_[1]);
			return std::make_shared<BoundConjunctionExpression>(con_type, left,
															   right);
		}
        case ExpressionClass::FUNCTION: {
            auto calc_expr = expr->Cast<CalcExpression>();
            std::vector<std::shared_ptr<Expression>> bound_children;

            for (const auto& child : calc_expr.children_) {
                bound_children.push_back(Transform(child));
            }
            switch (calc_expr.compute_type_) {
                case CalcType::Divide:{
                    scalar_function_p function1 = GetDivFunction();
                    std::vector<LogicalType> arguments1 = { LogicalType::FLOAT, LogicalType::FLOAT };
                    ScalarProjectFunction bound_function1("/", arguments1, LogicalType::FLOAT, function1, nullptr);
                    std::shared_ptr<BoundProjectFunctionExpression> bound_expr = std::make_shared<BoundProjectFunctionExpression>(LogicalType::FLOAT, bound_function1, bound_children, nullptr);

                    return bound_expr;
                }
                case CalcType::Multiply:{
                    scalar_function_p function1 = GetMulFunction();
                    std::vector<LogicalType> arguments1 = { LogicalType::FLOAT, LogicalType::FLOAT };
                    ScalarProjectFunction bound_function1("*", arguments1, LogicalType::FLOAT, function1, nullptr);
                    std::shared_ptr<BoundProjectFunctionExpression> bound_expr = std::make_shared<BoundProjectFunctionExpression>(LogicalType::FLOAT, bound_function1, bound_children, nullptr);
                    return bound_expr;
                }
                case CalcType::Minus:{
                    scalar_function_p function1 = GetSubFunction();
                    std::vector<LogicalType> arguments1 = { LogicalType::FLOAT, LogicalType::FLOAT };
                    ScalarProjectFunction bound_function1("-", arguments1, LogicalType::FLOAT, function1, nullptr);
                    std::shared_ptr<BoundProjectFunctionExpression> bound_expr = std::make_shared<BoundProjectFunctionExpression>(LogicalType::FLOAT, bound_function1, bound_children, nullptr);
                    return bound_expr;
                }
                case CalcType::Plus:{
                    scalar_function_p function1= GetAddFunction();
                    std::vector<LogicalType> arguments1 = { LogicalType::FLOAT, LogicalType::FLOAT };
                    ScalarProjectFunction bound_function1("+", arguments1, LogicalType::FLOAT, function1, nullptr);
                    std::shared_ptr<BoundProjectFunctionExpression> bound_expr = std::make_shared<BoundProjectFunctionExpression>(LogicalType::FLOAT, bound_function1, bound_children, nullptr);
                    return bound_expr;
                }
            }

        }

		case ExpressionClass::CASE: {
			auto case_expr = reinterpret_cast<CaseExpression&>(*expr);

			auto when = Transform(case_expr.when_expr[0]);

			auto then = Transform(case_expr.then_expr[0]);

			auto els = Transform(case_expr.else_expr);

			std::shared_ptr<BoundCaseExpression> bound_case_expr = std::make_shared<BoundCaseExpression>();
			bound_case_expr->SetWhenExpr(when);
			bound_case_expr->SetThenExpr(then);
			bound_case_expr->SetElseExpr(els);

			return bound_case_expr;
		}

		case ExpressionClass::LIKE: {
			auto like_expr = reinterpret_cast<LikeExpression&>(*expr);
			auto not_like = like_expr.not_like_;

			auto left = Transform(like_expr.children_[0]);
			auto right = Transform(like_expr.children_[1]);
			std::vector<std::shared_ptr<Expression>> arguments;
			arguments.push_back(std::move(left));
			arguments.push_back(std::move(right));
			if(not_like) {
				ScalarFunction bound_function = NotLikeFun::GetNotLikeFunction();
				std::shared_ptr<BoundFunctionExpression> functionExpression = std::make_shared<BoundFunctionExpression>(LogicalType::BOOL, bound_function, arguments, nullptr);
				functionExpression->bind_info = functionExpression->function.bind(functionExpression->function, functionExpression->children);
				return functionExpression;

			} else {
				ScalarFunction bound_function = LikeFun::GetLikeFunction();
				std::shared_ptr<BoundFunctionExpression> functionExpression = std::make_shared<BoundFunctionExpression>(LogicalType::BOOL, bound_function, arguments, nullptr);
				functionExpression->bind_info = functionExpression->function.bind(functionExpression->function, functionExpression->children);

				return  functionExpression;
			}


		}

		case ExpressionClass::LLM: {

			auto llm_expr = reinterpret_cast<LlmExpression&>(*expr);

			auto prompt = Transform(llm_expr.children_[0]);
			auto left = Transform(llm_expr.children_[1]);
			auto right = Transform(llm_expr.children_[2]);

			std::vector<std::shared_ptr<Expression>> arguments;
			arguments.push_back(left);
			arguments.push_back(right);

			std::vector<std::shared_ptr<Expression>> func_arguments = {prompt};
			ScalarProjectFunction bound_function = LLMCallFun::GetLLMCallFunction();
			std::shared_ptr<BoundProjectFunctionExpression> functionExpression = std::make_shared<BoundProjectFunctionExpression>(LogicalType::STRING, bound_function, arguments, nullptr);
			functionExpression->bind_info = functionExpression->function.bind(functionExpression->function, func_arguments);

			return functionExpression;
		}
		default:
			throw DaseX::Exception("Unsupported transform expression class");
	}
}

}  // namespace DaseX