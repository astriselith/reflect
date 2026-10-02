#pragma once

#include "Modifier.hpp"
#include "Parameter.hpp"
#include <cstdint>
#include <vector>

class Class;

class Constructor {
private:
  const Class *declaringClass_;

  Modifier modifier_;
  std::vector<Parameter> parameters_;

  uintptr_t address_;

public:
  Constructor();
  Constructor(const Class *declaringClass, Modifier modifier,
              std::vector<Parameter> parameters, uintptr_t address);

  const Class *getDeclaringClass() const;

  Modifier getModifier() const;
  const std::vector<Parameter> &getParameters() const;

  void *construct(const std::vector<void *> &args) const;

  uintptr_t getAddress() const;
};