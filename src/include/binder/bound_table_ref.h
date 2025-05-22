#pragma once

#include <memory>
#include <string>

namespace DaseX {

enum class TableReferenceType : uint8_t {
  INVALID = 0,
  BASE_TABLE = 1,
  JOIN = 3,
  CROSS_PRODUCT = 4,
  EXPRESSION_LIST = 5,
  SUBQUERY = 6,
  CTE = 7,
  EMPTY = 8
};

class BoundTableRef {
public:
  explicit BoundTableRef(TableReferenceType type) : type_(type) {}
  BoundTableRef() = default;
  virtual ~BoundTableRef() = default;

  virtual auto ToString() const -> std::string {
    switch (type_) {
    case TableReferenceType::INVALID:
      return "";
    case TableReferenceType::EMPTY:
      return "<empty>";
    default:
      return "not support";
    }
  }

  auto IsInvalid() const -> bool {
    return type_ == TableReferenceType::INVALID;
  }

  TableReferenceType type_{TableReferenceType::INVALID};
};

} // namespace DaseX
