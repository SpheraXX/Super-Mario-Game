#include "Model/Enemy/CheepCheep.h"

namespace model {

CheepCheep::CheepCheep(Vector2 position, int direction)
    : Enemy(position, {16.0f, 16.0f}) {
    setDirection(direction);
    setFacingRight(direction > 0);
    // Neutrally buoyant: it holds its row instead of sinking. Character::update still
    // integrates the horizontal velocity updateAI sets below.
    setGravityScale(0.0f);
}

void CheepCheep::updateAI(float /* deltaTime */) {
    // No turning rule: with tile collision off there is nothing to bump into that could
    // flip it, so a Cheep Cheep swims off-screen exactly as the original's does and the
    // level bounds reclaim it.
    velocity.x = SwimSpeed * static_cast<float>(getDirection());
    velocity.y = 0.0f;
}

}
