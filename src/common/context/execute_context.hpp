#pragma once

#include "context.hpp"
#include <memory>
#include <string>

namespace DaseX {

class Pipeline;

class ExecuteContext : public Context,
                       public std::enable_shared_from_this<ExecuteContext> {
public:
  std::string context_name;
  std::weak_ptr<Pipeline> pipeline;
  int operator_idx = -1;

public:
  ExecuteContext();
  std::string print_context_name() const override { return context_name; }
  void set_pipeline(std::shared_ptr<Pipeline> &pipeline);
  std::weak_ptr<Pipeline> get_pipeline();
  std::shared_ptr<ExecuteContext> getptr() { return shared_from_this(); }
};

} // namespace DaseX