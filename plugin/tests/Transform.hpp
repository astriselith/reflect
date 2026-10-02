#include "Behavior.hpp"

namespace com::engine {

#pragma reflect

class Transform : Behavior {

  int x, y, z = 0;

  Transform(int x_, int y_, int z_) : Behavior(), x(x_), y(y_), z(z_) {}
};
#pragma reflect
} // namespace com::engine