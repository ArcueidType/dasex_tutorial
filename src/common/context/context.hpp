#pragma once

#include <string>

namespace DaseX {

class Context {
public:
  Context() = default;
  virtual std::string print_context_name() const = 0;
  virtual ~Context() {}
};

} // namespace DaseX
