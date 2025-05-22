#pragma once

#include "../bound_table_ref.h"
#include "catalog.hpp"
#include <optional>
#include <string>
#include <utility>

namespace DaseX {

class SelectStatement;

/**
 * A CTE. e.g., `WITH (select 1) x SELECT * FROM x`, where `x` is `BoundCTERef`.
 */
class BoundCTERef : public BoundTableRef {
public:
  explicit BoundCTERef(std::string cte_name, std::string alias)
      : BoundTableRef(TableReferenceType::CTE), cte_name_(std::move(cte_name)),
        alias_(std::move(alias)) {}

  /** CTE name. */
  std::string cte_name_;

  /** Alias. */
  std::string alias_;
};
} // namespace DaseX
