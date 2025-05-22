
#pragma once

#include "bound_statement.hpp"
#include "catalog.hpp"
#include <string>
#include <vector>

namespace duckdb_libpgquery {
struct PGCreateStmt;
}

namespace DaseX {

class CreateStatement : public BoundStatement {
public:
  explicit CreateStatement(std::string table, std::vector<Field> columns,
                           std::vector<std::string> primary_key);

  std::string table_;
  std::vector<Field> columns_;
  std::vector<std::string> primary_key_;
};

} // namespace DaseX