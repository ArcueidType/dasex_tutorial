#pragma once

namespace DaseX {

enum class OperatorResultType {
  NO_MORE_OUTPUT,
  NEED_MORE_INPUT,
  HAVE_MORE_OUTPUT,
  FINISHED,
  BLOCKED
};

enum class SinkFinalizeType { READY, NO_OUTPUT_POSSIBLE, BLOCKED };

} // namespace DaseX
