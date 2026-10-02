#include "Constructor.hpp"
#include <stdexcept>
#include <utility>

Constructor::Constructor()
    : declaringClass_(nullptr), modifier_(), parameters_(), address_(0) {}

Constructor::Constructor(const Class *declaringClass, Modifier modifier,
                         std::vector<Parameter> parameters, uintptr_t address)
    : declaringClass_(declaringClass), modifier_(modifier),
      parameters_(std::move(parameters)), address_(address) {}

const Class *Constructor::getDeclaringClass() const { return declaringClass_; }

Modifier Constructor::getModifier() const { return modifier_; }

const std::vector<Parameter> &Constructor::getParameters() const {
  return parameters_;
}

void *Constructor::construct(const std::vector<void *> &args) const {
  if (address_ == 0) {
    throw std::logic_error("Constructor has no invoker");
  }

  using Invoker = void *(*)(const std::vector<void *> &);
  return reinterpret_cast<Invoker>(address_)(args);
}

uintptr_t Constructor::getAddress() const { return address_; }