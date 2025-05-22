#include "../include/binder/statement/create_statement.hpp"
#include "catalog.hpp"

namespace DaseX {

CreateStatement::CreateStatement(std::string table, std::vector<Field> columns,
                                 std::vector<std::string> primary_key)
    : BoundStatement(StatementType::CREATE_STATEMENT), table_(std::move(table)),
      columns_(std::move(columns)), primary_key_(std::move(primary_key)) {}

} // namespace DaseX