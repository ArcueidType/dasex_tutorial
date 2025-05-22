
#pragma once

#include <arrow/record_batch.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "catalog.hpp"
#include "logical_type.hpp"
#include "expression_type.hpp"

#define EXPR_CLONE_WITH_CHILDREN(cname)                                        \
  auto CloneWithChildren(std::vector<AbstractExpressionRef> children) const    \
      -> std::unique_ptr<AbstractExpression>                                   \
          override {                                                           \
    auto expr = cname(*this);                                                  \
    expr.children_ = children;                                                 \
    return std::make_unique<cname>(std::move(expr));                           \
  }

namespace DaseX {

class AbstractExpression;
using AbstractExpressionRef = std::shared_ptr<AbstractExpression>;

/**
 * AbstractExpression is the base class of all the expressions in the system.
 * Expressions are modeled as trees, i.e. every expression may have a variable
 * number of children.
 */
class AbstractExpression {
public:
  /**
   * Create a new AbstractExpression with the given children and return type.
   * @param children the children of this abstract expression
   * @param ret_type the return type of this abstract expression when it is
   * evaluated
   */
  AbstractExpression(std::vector<AbstractExpressionRef> children,
                     LogicalType ret_type,ExpressionClass expression_class)
      : children_{std::move(children)}, ret_type_{ret_type}, expression_class_{expression_class} {}

  /** Virtual destructor. */
  virtual ~AbstractExpression() = default;


  /** @return the child_idx'th child of this expression */
  auto GetChildAt(uint32_t child_idx) const -> const AbstractExpressionRef & {
    return children_[child_idx];
  }

  /** @return the children of this expression, ordering may matter */
  [[maybe_unused]] auto GetChildren() const -> const std::vector<AbstractExpressionRef> & {
    return children_;
  }
    ExpressionClass GetExpressionClass() const { return expression_class_; }
  /** @return the type of this expression if it were to be evaluated */
  virtual auto GetReturnType() const -> LogicalType { return ret_type_; }

  /** @return the string representation of the plan node and its children */
  virtual auto ToString() const -> std::string { return "<unknown>"; }

  /** @return a new expression with new children */
  virtual auto
  CloneWithChildren(std::vector<AbstractExpressionRef> children) const
      -> std::unique_ptr<AbstractExpression> = 0;

  /** The children of this expression. Note that the order of appearance of
   * children may matter. */
  std::vector<AbstractExpressionRef> children_;

  ExpressionClass expression_class_;

  template <typename TARGET> TARGET &Cast() {
    return reinterpret_cast<TARGET &>(*this);
  }
  template <typename TARGET> const TARGET &Cast() const {
    return reinterpret_cast<const TARGET &>(*this);
  }
  
private:
  /** The return type of this expression. */
  LogicalType ret_type_;
};

} // namespace DaseX
