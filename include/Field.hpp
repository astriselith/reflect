#pragma once
#include "Modifier.hpp"
#include <cstdint>
#include <string>

class Class;

class Field {
private:
  const Class *declaringClass_;

  Modifier modifier_;
  const Class *type_;
  std::string name_;

  uintptr_t address_get_;
  uintptr_t address_set_;

public:
  Field();
  Field(const Class *declaringClass, Modifier modifier, const Class *type,
        std::string name, uintptr_t address_get, uintptr_t address_set);

  const Class *getDeclaringClass() const;

  Modifier getModifier() const;
  const Class *getType() const;
  const std::string &getName() const;

  void get(void *object, void *outValue) const;
  void set(void *object, void *value) const;

  uintptr_t getAddressGet() const;
  uintptr_t getAddressSet() const;
};