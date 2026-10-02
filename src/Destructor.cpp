#include "Destructor.hpp"
#include <exception>

Destructor::Destructor() : address_(0) {}

Destructor::Destructor(uintptr_t address) : address_(address) {}

void Destructor::destruct(void *object) const noexcept {
  if (object == nullptr) {
    return;
  }
  if (address_ == 0) {
    std::terminate();
  }

  using Destroyer = void (*)(void *);
  reinterpret_cast<Destroyer>(address_)(object);
}

uintptr_t Destructor::getAddress() const { return address_; }