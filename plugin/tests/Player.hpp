#pragma once

#include "Behavior.hpp"

namespace com::engine {

#pragma reflect
class Player : public Behavior {

public:
  int life = 0;

  int attack = 0;

  Player(int life_) : Behavior(), life(life_) {}
};
#pragma reflect
} // namespace com::engine