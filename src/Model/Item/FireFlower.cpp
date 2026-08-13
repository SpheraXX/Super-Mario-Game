#include "Model/Item/FireFlower.h"
#include "Model/Player/Player.h"

namespace model {

FireFlower::FireFlower(Vector2 position)
    : Item(position, {16.0f, 16.0f}) {
    velocity = {0.0f, 0.0f};
    // No gravity and no movement: the flower waits on the block for the player.
    setGravityScale(0.0f);
}

void FireFlower::onCollect(Entity& collector) {
    if (auto* player = dynamic_cast<Player*>(&collector)) {
        if (player->getState().isFire() || player->getState().isStar()) {
            player->addScore(1000);
        } else {
            player->becomeFire();
        }
    }
    isActive = false;
}

}
