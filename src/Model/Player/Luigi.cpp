#include "Model/Player/Luigi.h"

namespace model {

// World units, same as Mario: one world tile tall.
Luigi::Luigi(Vector2 position)
    : Player(position, {32.0f, 32.0f}) {
}

float Luigi::getWalkSpeed() const {
    return WalkSpeed;
}

float Luigi::getRunSpeed() const {
    return RunSpeed;
}

float Luigi::getJumpForce() const {
    return JumpForce;
}

}
