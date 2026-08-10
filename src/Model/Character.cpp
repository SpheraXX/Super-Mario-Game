#include "Model/Character.h"

namespace model {

Character::Character(Vector2 position, Vector2 size)
    : Entity(position, size),
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
            // Deliberately full gravity, ignoring gravityScale: the level only reclaims a
            // dying body once it drops past the bottom of the world, so a floating enemy
            // (scale 0) would otherwise hang there forever and never be removed.
            velocity.y += Gravity * deltaTime;
            if (velocity.y > MaxFallSpeed) {
                velocity.y = MaxFallSpeed;
            }
            Vector2 pos = getPosition();
            pos.x += velocity.x * deltaTime;
            pos.y += velocity.y * deltaTime;
            setPosition(pos);
            deathElapsed += deltaTime;
        }
        return;
    }

    applyGravity(deltaTime);

    Vector2 pos = getPosition();
    pos.x += velocity.x * deltaTime;
    pos.y += velocity.y * deltaTime;
    setPosition(pos);
}

void Character::onCollision(Entity* /* other */) {
}

float Character::getWalkSpeed() const {
    return 180.0f;
}

float Character::getRunSpeed() const {
    return 320.0f;
}

float Character::getJumpForce() const {
    return -450.0f;
}

void Character::takeDamage(int amount) {
    health -= amount;
    if (health <= 0) {
        die();
    }
}

void Character::move() {
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
        velocity.y += Gravity * gravityScale * deltaTime;
        if (velocity.y > MaxFallSpeed) {
            velocity.y = MaxFallSpeed;
        }
    }
}

float Character::getGravityScale() const {
    return gravityScale;
}

void Character::setGravityScale(float scale) {
    gravityScale = scale;
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

void Character::clampVelocity() {
    if (velocity.x > 400.0f) velocity.x = 400.0f;
    if (velocity.x < -400.0f) velocity.x = -400.0f;
    if (velocity.y > MaxFallSpeed) velocity.y = MaxFallSpeed;
}
}
