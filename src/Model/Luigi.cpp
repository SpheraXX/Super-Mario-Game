#include "Model/Luigi.h"

namespace model {

Luigi::Luigi(Vector2 position)
    : Player(position, {16.0f, 16.0f}) {
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
