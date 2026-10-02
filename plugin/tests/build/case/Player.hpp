#pragma once

#include "Behavior.hpp"

namespace com::engine {

#pragma reflect
class Player : public Behavior {
    friend class __Player;


public:
  int life = 0;

  int attack = 0;

  Player(int life_) : Behavior(), life(life_) {}
};
#pragma reflect
} // namespace com::engine