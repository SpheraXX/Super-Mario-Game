#include "Model/Enemy/Bowser.h"

#include "Model/Projectile/Fireball.h"

#include <memory>

namespace model {

// Two tiles square, the largest enemy on the sheet: 32x32 source artwork, drawn 1:1.
Bowser::Bowser(Vector2 position)
    : Enemy(position, {32.0f, 32.0f}),
      patrolCentreX(position.x),
      jumpTimer(JumpInterval) {
    health = MaxHealth;
    attackCooldown = FireInterval;
    attackTimer = FireInterval;
    velocity.x = -WalkSpeed;
    setDirection(-1);
}

void Bowser::updateAI(float deltaTime) {
    // Pace back and forth across the bridge.
    const float offset = getPosition().x - patrolCentreX;
    if (offset < -PatrolRange) {
        setDirection(1);
    } else if (offset > PatrolRange) {
        setDirection(-1);
    }
    velocity.x = WalkSpeed * getDirection();

    jumpTimer -= deltaTime;
    if (jumpTimer <= 0.0f && isOnGround()) {
        velocity.y = JumpSpeed;
        jumpTimer = JumpInterval;
    }

    // Turn to face the player so the fire is aimed rather than sprayed at a wall.
    if (const Entity* target = findPlayer()) {
        const float targetCentre = target->getPosition().x + target->getSize().x / 2.0f;
        const float selfCentre = getPosition().x + getSize().x / 2.0f;
        setFacingRight(targetCentre > selfCentre);
    }
}

std::unique_ptr<Projectile> Bowser::createProjectile() {
    const int fireDirection = isFacingRight() ? 1 : -1;

    // Breathed from mouth height, clear of the side he is facing. Firing left has to back off
    // by the fireball's own width, since a position is a top-left corner.
    Vector2 origin = getPosition();
    origin.y += getSize().y * 0.3f;
    origin.x += (fireDirection > 0) ? getSize().x : -FireballWidth;
    return std::make_unique<Fireball>(origin, this, fireDirection);
}

void Bowser::onHit(Entity& /* source */) {
    if (!isAlive() || isDying()) return;

    // Unlike every other enemy, a single hit does not finish him: he has a health pool.
    // Decremented directly rather than through takeDamage(), which calls die() and would
    // clear `alive` before beginDying() could start the death fall.
    health -= 1;
    if (health <= 0) {
        beginDying(true);
        awardScore();  // only the killing blow pays, not each of the five hits
    }
}

}
