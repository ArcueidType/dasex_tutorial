#pragma once

namespace DaseX {

enum class PipelineResultType {
  NEED_MORE_INPUT,
  HAVE_MORE_OUTPUT,
  FINISHED,
  BLOCKED,
  NO_MORE_OUTPUT
};

} // namespace DaseX
