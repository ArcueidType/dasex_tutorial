#pragma once

#include <sstream>
#include <string>
#include <cstdint>

namespace DaseX {

enum class PhysicalOperatorType {
  INVALID,
  ORDER_BY,
  LIMIT,
  STREAMING_LIMIT,
  LIMIT_PERCENT,
  TOP_N,
  WINDOW,
  UNNEST,
  UNGROUPED_AGGREGATE,
  HASH_GROUP_BY,
  MULTI_FIELD_HASH_GROUP_BY,
  PERFECT_HASH_GROUP_BY,
  FILTER,
  PROJECTION,
  COPY_TO_FILE,
  BATCH_COPY_TO_FILE,
  FIXED_BATCH_COPY_TO_FILE,
  RESERVOIR_SAMPLE,
  STREAMING_SAMPLE,
  STREAMING_WINDOW,
  PIVOT,

  // -----------------------------
  // Scans
  // -----------------------------
  TABLE_SCAN,
  DUMMY_SCAN,
  COLUMN_DATA_SCAN,
  CHUNK_SCAN,
  RECURSIVE_CTE_SCAN,
  CTE_SCAN,
  DELIM_SCAN,
  EXPRESSION_SCAN,
  POSITIONAL_SCAN,
  // -----------------------------
  // Joins
  // -----------------------------
  BLOCKWISE_NL_JOIN,
  NESTED_LOOP_JOIN,
  HASH_JOIN,
  CROSS_PRODUCT,
  PIECEWISE_MERGE_JOIN,
  IE_JOIN,
  DELIM_JOIN,
  INDEX_JOIN,
  POSITIONAL_JOIN,
  ASOF_JOIN,
  RADIX_JOIN,
  // -----------------------------
  // SetOps
  // -----------------------------
  UNION,
  RECURSIVE_CTE,
  CTE,

  // -----------------------------
  // Updates
  // -----------------------------
  INSERT,
  BATCH_INSERT,
  DELETE_OPERATOR,
  UPDATE,

  // -----------------------------
  // Schema
  // -----------------------------
  CREATE_TABLE,
  CREATE_TABLE_AS,
  BATCH_CREATE_TABLE_AS,
  CREATE_INDEX,
  ALTER,
  CREATE_SEQUENCE,
  CREATE_VIEW,
  CREATE_SCHEMA,
  CREATE_MACRO,
  DROP,
  PRAGMA,
  TRANSACTION,
  CREATE_TYPE,
  ATTACH,
  DETACH,

  // -----------------------------
  // Helpers
  // -----------------------------
  EXPLAIN,
  EXPLAIN_ANALYZE,
  EMPTY_RESULT,
  EXECUTE,
  PREPARE,
  VACUUM,
  EXPORT,
  SET,
  LOAD,
  INOUT_FUNCTION,
  RESULT_COLLECTOR,
  RESET,
  EXTENSION
};

std::string physical_operator_to_string(PhysicalOperatorType type);

enum class SinkResultType { NEED_MORE_INPUT, FINISHED, BLOCKED };

enum class SourceResultType { HAVE_MORE_OUTPUT, FINISHED, BLOCKED };

enum class OperatorType { SOURCE, SINK, OPERATOR, INVALID };

enum class JoinTypes : uint8_t {
    INVALID = 0, // invalid join type
    LEFT = 1,    // left
    RIGHT = 2,   // right
    INNER = 3,   // inner
    OUTER = 4,   // outer
    SEMI = 5,    // SEMI join returns left side row ONLY if it has a join partner, no duplicates
    ANTI = 6,    // ANTI join returns left side row ONLY if it has NO join partner, no duplicates
    MARK = 7,    // MARK join returns marker indicating whether or not there is a join partner (true), there is no join
    // partner (false)
    SINGLE = 8   // SINGLE join is like LEFT OUTER JOIN, BUT returns at most one join partner per entry on the LEFT side
    // (and NULL if no partner is found)
};

} // namespace DaseX
