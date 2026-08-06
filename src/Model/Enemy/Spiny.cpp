#include "Model/Enemy/Spiny.h"

namespace model {

Spiny::Spiny(Vector2 position)
    : Enemy(position, {32.0f, 32.0f}) {
    velocity.x = -WalkSpeed;
    setDirection(-1);
}

void Spiny::updateAI(float /* deltaTime */) {
    velocity.x = WalkSpeed * getDirection();
}

void Spiny::onTileCollision(char /* tile */, CollisionType side) {
    if (side == CollisionType::Left || side == CollisionType::Right) {
        setDirection(-getDirection());
        velocity.x = WalkSpeed * getDirection();
    }
}

}
