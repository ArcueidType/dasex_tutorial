#pragma once

#include <memory>

#include "bound_expression.h"
#include "compare_expression.h"
#include "select_statement.hpp"
namespace DaseX {

enum class SubqueryType : uint8_t {
	INVALID = 0,
	SCALAR = 1,		 // Regular scalar subquery
	EXISTS = 2,		 // EXISTS (SELECT...)
	NOT_EXISTS = 3,	 // NOT EXISTS(SELECT...)
	ANY = 4,		 // x = ANY(SELECT...) OR x IN (SELECT...)
};

class BoundSubquery : public BoundExpression {
   public:
	BoundSubquery(std::unique_ptr<SelectStatement> subquery,
				  SubqueryType subquery_type,
				  std::unique_ptr<BoundExpression> child,
				  ComparisonType comparison_type)
		: BoundExpression(ExpressionType::SUBQUERY),
		  subquery(std::move(subquery)),
		  subquery_type(subquery_type),
		  child(std::move(child)),
		  comparison_type((comparison_type)) {}

	std::unique_ptr<SelectStatement> subquery;

	SubqueryType subquery_type;

	//! the child expression to compare with (in case of IN, ANY, ALL operators,
	//! empty for EXISTS queries and scalar subquery)
	std::unique_ptr<BoundExpression> child;

	//! The comparison type of the child expression with the subquery (in case
	//! of ANY, ALL operators), empty otherwise
	ComparisonType comparison_type;
};

}  // namespace DaseX