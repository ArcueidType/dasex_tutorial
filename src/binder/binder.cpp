#include "../include/binder/binder.h"

namespace DaseX {

Binder::Binder(std::weak_ptr<CataLog> catalog) : catalog_(catalog) {}

void Binder::ParseAndSave(const std::string &query) {

  parser_.Parse(query);

  SaveParseTree(parser_.parse_tree);
}

auto Binder::IsKeyword(const std::string &text) -> bool {
  return duckdb::PostgresParser::IsKeyword(text);
}

} // namespace DaseX