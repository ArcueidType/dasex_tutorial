#pragma once

#include "../bound_table_ref.h"
#include "catalog.hpp"
#include <optional>
#include <string>
#include <utility>

namespace DaseX {

/**
 * A bound table ref type for single table. e.g., `SELECT x FROM y`, where `y`
 * is `BoundBaseTableRef`.
 */
class BoundBaseTableRef : public BoundTableRef {
public:
  explicit BoundBaseTableRef(std::string table,
                             std::optional<std::string> alias,
                             std::shared_ptr<arrow::Schema> &schema)
      : BoundTableRef(TableReferenceType::BASE_TABLE), table_(std::move(table)),
        alias_(std::move(alias)), schema_(*std::move(schema)) {}

  /**
   *
   * @return  若有返回别名，否则返回表名
   */
  auto GetBoundTableName() const -> std::string {
    if (alias_ != std::nullopt) {
      return *alias_;
    }
    return table_;
  }

  std::string table_;

  std::optional<std::string> alias_;

  arrow::Schema schema_;
};
} // namespace DaseX
