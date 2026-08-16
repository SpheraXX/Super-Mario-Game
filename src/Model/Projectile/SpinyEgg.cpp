#include "Model/Projectile/SpinyEgg.h"

#include "Model/Core/World.h"
#include "Model/Enemy/Spiny.h"

#include <memory>

namespace model {

SpinyEgg::SpinyEgg(Vector2 position, Entity* owner, float throwVelocityX)
    : Projectile(position, {16.0f, 16.0f}, owner) {
    velocity = {throwVelocityX, 0.0f};
    setDirection(throwVelocityX < 0.0f ? -1 : 1);
}

void SpinyEgg::onTileCollision(char /* tile */, CollisionType side) {
    // Landing is the only tile contact that matters; glancing off a wall just keeps it
    // falling, which is what the original's intended (bouncing) behaviour did too.
    if (side == CollisionType::Bottom) {
        hatch();
    }
}

void SpinyEgg::hatch() {
    if (!isActive) return;
    expire();

    if (world) {
        // The Spiny takes over the egg's exact footprint, so it appears where the egg landed
        // rather than snapping to a tile.
        world->spawn(std::make_unique<Spiny>(getPosition()));
    }
}

}
