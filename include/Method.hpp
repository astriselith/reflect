#pragma once

#include "Modifier.hpp"
#include "Parameter.hpp"
#include <cstdint>
#include <string>
#include <vector>

class Class;

class Method {
private:
  const Class *declaringClass_;

  Modifier modifier_;
  const Class *returnType_;
  std::string name_;
  std::vector<Parameter> parameters_;

  uintptr_t address_;

public:
  Method();
  Method(const Class *declaringClass, Modifier modifier,
         const Class *returnType, std::string name,
         std::vector<Parameter> parameters, uintptr_t address);

  const Class *getDeclaringClass() const;

  Modifier getModifier() const;
  const Class *getReturnType() const;
  const std::string &getName() const;
  const std::vector<Parameter> &getParameters() const;

  void *invoke(void *object, const std::vector<void *> &args) const;
  void *invokeStatic(const std::vector<void *> &args) const;

  uintptr_t getAddress() const;
};