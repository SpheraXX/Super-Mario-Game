#include "Model/Item/Starman.h"
#include "Model/Player/Player.h"

namespace model {

Starman::Starman(Vector2 position)
    : Item(position, {16.0f, 16.0f}) {
    velocity = {0.0f, 0.0f};
    // No gravity and no movement: the star waits on the block for the player, exactly
    // like the Fire Flower it shares its behaviour with.
    setGravityScale(0.0f);
}

void Starman::onCollect(Entity& collector) {
    if (auto* player = dynamic_cast<Player*>(&collector)) {
        // Already star: an extra one is worth points, matching the other power-ups.
        if (player->isStar()) {
            player->addScore(1000);
        } else {
            player->becomeStar();
        }
    }
    isActive = false;
}

}
