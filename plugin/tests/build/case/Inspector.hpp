#pragma once

#include "Class.hpp"
#include <iosfwd>

class Inspector {
public:
  void inspect(const Class &classInfo, std::ostream &output) const;
};