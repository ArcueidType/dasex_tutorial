#pragma once

#include "statement_type.hpp"
#include <string>
namespace DaseX {

///  即为 sql statemnet 的基类

class BoundStatement {
public:
  explicit BoundStatement(StatementType type);
  virtual ~BoundStatement() = default;

  StatementType type_;

public:
};

} // namespace DaseX