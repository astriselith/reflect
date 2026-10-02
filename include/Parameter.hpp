#pragma once
#include "Modifier.hpp"
#include <cstdint>
#include <string>

class Class;

class Parameter {
public:
  Parameter();
  Parameter(std::string name, const Class *type, Modifier modifier);

  const std::string &getName() const;
  const Class *getType() const;
  Modifier getModifier() const;

private:
  Modifier modifier_;
  const Class *type_;
  std::string name_;
};