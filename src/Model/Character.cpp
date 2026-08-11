#include "Model/Character.h"
#include "Model/Map/TileMap.h"
#include "Model/World/WorldTheme.h"

#include <algorithm>

namespace model {

Character::Character(Vector2 position, Vector2 size)
    : Entity(position, size),
      isGrounded(false),
      velocity({0.0f, 0.0f}),
      direction(1),
      health(1),
      alive(true),
      animState(AnimState::Idle),
      facingRight(true) {
}

void Character::update(float deltaTime) {
    if (!alive) {
        if (isDyingFlag) {
            // Death fall: gravity only. The body pops through tiles and never lands.
            velocity.y += getGravity() * deltaTime;
            if (velocity.y > getMaxFallSpeed()) {
                velocity.y = getMaxFallSpeed();
            }
            Vector2 pos = getPosition();
            pos.x += velocity.x * deltaTime;
            pos.y += velocity.y * deltaTime;
            setPosition(pos);
        }
        return;
    }

    // Worlds with horizontal drag (Overworld, Underwater) bleed off lateral velocity so
    // characters settle against their top speed instead of skating; the value is a decay
    // per second (1.2 underwater keeps motion floaty, 0.4 on land trims a hot sprint).
    const float drag = getHorizontalDrag();
    if (drag > 0.0f && velocity.x != 0.0f) {
        velocity.x *= std::max(0.0f, 1.0f - drag * deltaTime);
    }

    applyGravity(deltaTime);

    Vector2 pos = getPosition();
    pos.x += velocity.x * deltaTime;
    pos.y += velocity.y * deltaTime;
    setPosition(pos);
}

float Character::getWalkSpeed() const {
    return 180.0f;
}

float Character::getRunSpeed() const {
    return 320.0f;
}

float Character::getMaxJumpSpeed() const {
    return 500.0f;
}

float Character::getJumpAccel() const {
    return 3400.0f;
}

void Character::takeDamage(int amount) {
    health -= amount;
    if (health <= 0) {
        die();
    }
}

void Character::die() {
    alive = false;
    animState = AnimState::Die;
}

void Character::beginDying(bool bounce) {
    if (!alive || isDyingFlag) return;
    alive = false;
    isDyingFlag = true;
    animState = AnimState::Die;
    velocity.x = 0.0f;
    if (bounce) {
        velocity.y = DeathBounceSpeed;
    }
}

bool Character::isDying() const {
    return isDyingFlag;
}

void Character::applyGravity(float deltaTime) {
    if (!isGrounded) {
        velocity.y += getGravity() * deltaTime;
        if (velocity.y > getMaxFallSpeed()) {
            velocity.y = getMaxFallSpeed();
        }
    }
}

void Character::setMap(const TileMap* map) {
    mapPtr = map;
}

void Character::setWorld(const WorldTheme& world) {
    worldPtr = &world;
}

float Character::getGravity() const {
    return worldPtr ? DefaultGravity * worldPtr->getGravityScale() : DefaultGravity;
}

float Character::getMaxFallSpeed() const {
    return worldPtr ? DefaultMaxFallSpeed * worldPtr->getMaxFallScale() : DefaultMaxFallSpeed;
}

float Character::getHorizontalDrag() const {
    return worldPtr ? worldPtr->getHorizontalDrag() : 0.0f;
}

bool Character::isUnderwater() const {
    return worldPtr != nullptr && worldPtr->getType() == WorldType::Underwater;
}

bool Character::isOnGround() const {
    return isGrounded;
}

int Character::getDirection() const {
    return direction;
}

void Character::setDirection(int d) {
    direction = d;
    facingRight = (d > 0);
}

bool Character::isAlive() const {
    return alive;
}

AnimState Character::getAnimState() const {
    return animState;
}

void Character::setAnimState(AnimState state) {
    animState = state;
}

bool Character::isFacingRight() const {
    return facingRight;
}

void Character::setFacingRight(bool right) {
    facingRight = right;
}

Vector2 Character::getVelocity() const {
    return velocity;
}

void Character::setVelocity(Vector2 v) {
    velocity = v;
}

}
