#include "Field.hpp"
#include <stdexcept>

Field::Field()
    : declaringClass_(nullptr), modifier_(0), type_(nullptr), name_(""),
      address_get_(0), address_set_(0) {}

Field::Field(const Class *declaringClass, Modifier modifier, const Class *type,
             std::string name, uintptr_t address_get, uintptr_t address_set)
    : declaringClass_(declaringClass), modifier_(modifier), type_(type),
      name_(std::move(name)), address_get_(address_get),
      address_set_(address_set) {}

const Class *Field::getDeclaringClass() const { return declaringClass_; }

Modifier Field::getModifier() const { return modifier_; }
const Class *Field::getType() const { return type_; }
const std::string &Field::getName() const { return name_; }

void Field::get(void *object, void *outValue) const {
  if (object == nullptr || outValue == nullptr) {
    throw std::invalid_argument("Field::get requires non-null pointers");
  }
  if (address_get_ == 0) {
    throw std::logic_error("Field has no getter");
  }

  using Getter = void (*)(void *, void *);
  reinterpret_cast<Getter>(address_get_)(object, outValue);
}

void Field::set(void *object, void *value) const {
  if (object == nullptr) {
    throw std::invalid_argument("Field::set requires non-null pointers");
  }
  if (address_set_ == 0) {
    throw std::logic_error("Field has no setter");
  }

  using Setter = void (*)(void *, void *);
  reinterpret_cast<Setter>(address_set_)(object, value);
}

uintptr_t Field::getAddressSet() const { return address_set_; }
uintptr_t Field::getAddressGet() const { return address_get_; }