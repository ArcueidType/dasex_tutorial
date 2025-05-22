#include "execute_context.hpp"
#include "pipeline.hpp"

namespace DaseX
{

    ExecuteContext::ExecuteContext() : context_name("ExecuteContext") {}


void ExecuteContext::set_pipeline(std::shared_ptr<Pipeline> &pipeline) {
  pipeline->execute_context = this->getptr();
  this->pipeline = pipeline;
}

std::weak_ptr<Pipeline> ExecuteContext::get_pipeline() {
  return this->pipeline;
}

} // namespace DaseX