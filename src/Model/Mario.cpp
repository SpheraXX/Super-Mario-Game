#include "Model/Mario.h"

namespace model {

Mario::Mario(Vector2 position)
    : Player(position, {16.0f, 16.0f}) {
}

float Mario::getWalkSpeed() const {
    return WalkSpeed;
}

float Mario::getRunSpeed() const {
    return RunSpeed;
}

float Mario::getJumpForce() const {
    return JumpForce;
}

}
