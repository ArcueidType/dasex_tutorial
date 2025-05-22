#pragma once

namespace DaseX {

enum class RC {
  SUCCESS,
  FAILED,
  NO_RESULT,

  // table
  TABLE_ALREADY_EXISTS,
  TABLE_NOT_EXISTS,
  INVALID_DATA,

  // data
  UNSUPPORTED_DATA_TYPE,
  INCONSISTENT_DATA_TYPE,
  INCONSISTENT_SCHEMA_LENGTH,

  // io
  IO_ERROR,
};

} // namespace DaseX
