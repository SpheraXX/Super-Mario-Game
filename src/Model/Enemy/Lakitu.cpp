#include "Model/Enemy/Lakitu.h"

#include "Model/Projectile/SpinyEgg.h"

#include <cmath>
#include <memory>

namespace model {

// 16x23 source artwork at 2x.
Lakitu::Lakitu(Vector2 position)
    : Enemy(position, {32.0f, 46.0f}),
      hoverY(position.y) {
    setGravityScale(0.0f);
    attackCooldown = ThrowInterval;
    attackTimer = ThrowInterval;
}

void Lakitu::updateAI(float /* deltaTime */) {
    if (isStomped) {
        velocity = {0.0f, 0.0f};
        return;
    }

    // Altitude is fixed: it was chosen by whoever placed the Lakitu on the map.
    Vector2 position = getPosition();
    position.y = hoverY;
    setPosition(position);
    velocity.y = 0.0f;

    const Entity* target = findPlayer();
    if (!target) {
        velocity.x = 0.0f;
        return;
    }

    const float targetCentre = target->getPosition().x + target->getSize().x / 2.0f;
    const float selfCentre = getPosition().x + getSize().x / 2.0f;
    const float gap = targetCentre - selfCentre;

    // Drift toward the player, but hold still once roughly overhead — chasing a zero gap
    // makes the cloud twitch left and right every frame.
    if (std::fabs(gap) < DeadZone) {
        velocity.x = 0.0f;
    } else {
        const int towards = (gap > 0.0f) ? 1 : -1;
        velocity.x = TrackSpeed * static_cast<float>(towards);
        setDirection(towards);
    }
}

std::unique_ptr<Projectile> Lakitu::createProjectile() {
    // Throw the egg ahead of where the player is heading rather than dropping it straight
    // down, which a player standing under the cloud could simply walk out of.
    float lead = 0.0f;
    if (const Entity* target = findPlayer()) {
        const float playerVelocityX = target->getVelocity().x;
        if (std::fabs(playerVelocityX) > 1.0f) {
            lead = (playerVelocityX > 0.0f) ? EggLeadSpeed : -EggLeadSpeed;
        }
    }

    Vector2 origin = getPosition();
    origin.y += getSize().y;  // released from under the cloud
    return std::make_unique<SpinyEgg>(origin, this, lead);
}

}
