#include "Model/Item/MapCoin.h"

#include "Model/Core/GameManager.h"
#include "Model/Player/Player.h"

namespace model {

MapCoin::MapCoin(Vector2 position)
    : Item(position, {16.0f, 16.0f}) {
    // Static by construction: no pop velocity to seed and no gravity to resist. Belt and
    // braces alongside updateBehavior(), so the coin stays put even if something else ever
    // routes it through Character::update.
    setGravityScale(0.0f);
    velocity = {0.0f, 0.0f};
}

void MapCoin::updateBehavior(float deltaTime) {
    // No physics at all — only the animation clock advances. Deliberately does NOT call
    // Character::update: a placed coin never moves.
    animationClock += deltaTime;
}

void MapCoin::onCollect(Entity& collector) {
    // Guard against a second credit: the player can still overlap the coin on the frame
    // after it was taken, and Item::onCollision fires for every overlapping frame.
    if (!isActive) {
        return;
    }
    
    if (auto* player = dynamic_cast<Player*>(&collector)) {
        player->addCoin();
    } else {
        GameManager::instance().addCoin();
    }
    
    GameManager::instance().addScore(CoinScore);
    isActive = false;
}

}
