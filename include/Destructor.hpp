#pragma once

#include <cstdint>

class Destructor {
private:
  uintptr_t address_;

public:
  Destructor();
  explicit Destructor(uintptr_t address);

  void destruct(void *object) const noexcept;
  uintptr_t getAddress() const;
};