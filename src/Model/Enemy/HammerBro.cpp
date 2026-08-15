#include "Model/Enemy/HammerBro.h"

#include "Model/Projectile/Hammer.h"

#include <cmath>
#include <memory>

namespace model {

// 16x23 source artwork, drawn 1:1.
HammerBro::HammerBro(Vector2 position)
    : Enemy(position, {16.0f, 23.0f}),
      patrolCentreX(position.x),
      hopTimer(HopInterval) {
    attackCooldown = ThrowInterval;
    attackTimer = ThrowInterval;
    velocity.x = -WalkSpeed;
    setDirection(-1);
}

void HammerBro::updateAI(float deltaTime) {
    if (isStomped) {
        velocity.x = 0.0f;
        return;
    }

    // Pace around the spawn point rather than walking the whole level.
    const float offset = getPosition().x - patrolCentreX;
    if (offset < -PatrolRange) {
        setDirection(1);
    } else if (offset > PatrolRange) {
        setDirection(-1);
    }
    velocity.x = WalkSpeed * getDirection();

    // Hops constantly, which is what makes the hammer pattern hard to read.
    hopTimer -= deltaTime;
    if (hopTimer <= 0.0f && isOnGround()) {
        velocity.y = HopSpeed;
        hopTimer = HopInterval;
    }

    // Face the player so the hammers are aimed. Patrol still drives movement; only the
    // throwing direction tracks.
    if (const Entity* target = findPlayer()) {
        const float targetCentre = target->getPosition().x + target->getSize().x / 2.0f;
        const float selfCentre = getPosition().x + getSize().x / 2.0f;
        setFacingRight(targetCentre > selfCentre);
    }
}

std::unique_ptr<Projectile> HammerBro::createProjectile() {
    const int throwDirection = isFacingRight() ? 1 : -1;
    // Launch from the top of the sprite so the arc starts above the Bro's head.
    Vector2 origin = getPosition();
    origin.y -= 4.0f;
    return std::make_unique<Hammer>(origin, this, throwDirection);
}

}
