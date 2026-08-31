#include "Model/Enemy/Goomba.h"

#include <string>

namespace model {

// World units: one world tile (the 16x16 source frame draws 1:1).
Goomba::Goomba(Vector2 position)
    : Enemy(position, {16.0f, 16.0f}),
      squishTimer(0.0f) {
    velocity.x = -WalkSpeed; // Start walking left
    setDirection(-1);
}

void Goomba::updateAI(float /* deltaTime */) {
    if (isStomped) {
        velocity.x = 0.0f;
        return;
    }
    
    velocity.x = WalkSpeed * getDirection();
}

void Goomba::onStomped(Entity& /* player */) {
    isStomped = true;
    despawnTimer = 0.5f; // Squish sprite shows for 0.5 seconds
    hitbox.isTrigger = true; // Disable solid collision
    velocity = {0.0f, 0.0f};
    awardScore();
}

void Goomba::onTileCollision(char /* tile */, CollisionType side) {
    if (side == CollisionType::Left || side == CollisionType::Right) {
        setDirection(-getDirection());
        velocity.x = WalkSpeed * getDirection();
    }
}

}
