#include "Method.hpp"
#include <stdexcept>
#include <utility>

Method::Method()
    : declaringClass_(nullptr), modifier_(), returnType_(nullptr), name_(),
      parameters_(), address_(0) {}

Method::Method(const Class *declaringClass, Modifier modifier,
               const Class *returnType, std::string name,
               std::vector<Parameter> parameters, uintptr_t address)
    : declaringClass_(declaringClass), modifier_(modifier),
      returnType_(returnType), name_(std::move(name)),
      parameters_(std::move(parameters)), address_(address) {}

const Class *Method::getDeclaringClass() const { return declaringClass_; }

Modifier Method::getModifier() const { return modifier_; }
const Class *Method::getReturnType() const { return returnType_; }
const std::string &Method::getName() const { return name_; }
const std::vector<Parameter> &Method::getParameters() const {
  return parameters_;
}

void *Method::invoke(void *object, const std::vector<void *> &args) const {
  if (modifier_.isStatic()) {
    throw std::logic_error("Cannot invoke a static method as an instance method");
  }
  if (object == nullptr) {
    throw std::invalid_argument("Method::invoke requires a non-null object");
  }
  if (address_ == 0) {
    throw std::logic_error("Method has no invoker");
  }

  using Invoker = void *(*)(void *, const std::vector<void *> &);
  return reinterpret_cast<Invoker>(address_)(object, args);
}

void *Method::invokeStatic(const std::vector<void *> &args) const {
  if (!modifier_.isStatic()) {
    throw std::logic_error("Cannot invoke an instance method as static");
  }
  if (address_ == 0) {
    throw std::logic_error("Method has no invoker");
  }

  using Invoker = void *(*)(const std::vector<void *> &);
  return reinterpret_cast<Invoker>(address_)(args);
}

uintptr_t Method::getAddress() const { return address_; }