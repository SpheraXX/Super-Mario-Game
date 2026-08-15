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
    // All power-up policy lives in Player::applyPowerUp (the size axis vs the ability
    // slot, plus the redundant-power-up points). The item just hands the collect over.
    if (auto* player = dynamic_cast<Player*>(&collector)) {
        player->applyPowerUp(PlayerPowerUp::FireFlower);
    }
    isActive = false;
}

}
