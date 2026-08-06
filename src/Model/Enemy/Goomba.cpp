#include "Model/Enemy/Goomba.h"

#include <fstream>
#include <string>

namespace model {

namespace {
// TEMP trace instrumentation (removed after playtest).
void trace(const std::string& msg) {
    std::ofstream out("trace_log.txt", std::ios::app);
    out << msg << '\n';
}
}

// World units: one world tile (the 16x16 source frame is scaled up 2x by the renderer).
Goomba::Goomba(Vector2 position)
    : Enemy(position, {32.0f, 32.0f}),
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
    trace("goombaStomped");
    isStomped = true;
    despawnTimer = 0.5f; // Squish sprite shows for 0.5 seconds
    hitbox.isTrigger = true; // Disable solid collision
    velocity = {0.0f, 0.0f};
}

void Goomba::onTileCollision(char /* tile */, CollisionType side) {
    if (side == CollisionType::Left || side == CollisionType::Right) {
        setDirection(-getDirection());
        velocity.x = WalkSpeed * getDirection();
    }
}

}
