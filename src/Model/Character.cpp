#include "Model/Character.h"
#include "Model/TileMap.h"

#include <SFML/Graphics/RenderWindow.hpp>

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

void Character::render(sf::RenderWindow& /* window */) {
}

void Character::onCollision(Entity* /* other */) {
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
        velocity.y += Gravity * deltaTime;
        if (velocity.y > MaxFallSpeed) {
            velocity.y = MaxFallSpeed;
        }
    }
}

void Character::setMap(const TileMap* map) {
    mapPtr = map;
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

void Character::resolveTileCollisions() {
    if (!mapPtr) return;

    Vector2 pos = getPosition();
    Vector2 sz = getSize();

    float footY = pos.y + sz.y;
    float headY = pos.y;
    float leftX = pos.x;
    float rightX = pos.x + sz.x;
    float centerY = pos.y + sz.y / 2.0f;
    float centerX = pos.x + sz.x / 2.0f;

    if (velocity.y >= 0.0f) {
        std::size_t col = static_cast<std::size_t>(centerX / TileMap::TileWidth);
        std::size_t row = TileMap::Rows - 1 - static_cast<std::size_t>(footY / TileMap::TileHeight);

        if (col < mapPtr->getColumns() && row < TileMap::Rows) {
            if (mapPtr->getTile(row, col) != '.') {
                pos.y = (TileMap::Rows - 1 - row) * TileMap::TileHeight - sz.y;
                velocity.y = 0.0f;
            }
        }
    }

    if (velocity.y < 0.0f) {
        std::size_t col = static_cast<std::size_t>(centerX / TileMap::TileWidth);
        std::size_t row = TileMap::Rows - 1 - static_cast<std::size_t>(headY / TileMap::TileHeight);

        if (col < mapPtr->getColumns() && row < TileMap::Rows) {
            if (mapPtr->getTile(row, col) != '.') {
                pos.y = (TileMap::Rows - row) * TileMap::TileHeight;
                velocity.y = 0.0f;
            }
        }
    }

    if (velocity.x < 0.0f) {
        std::size_t col = static_cast<std::size_t>(leftX / TileMap::TileWidth);
        std::size_t row = TileMap::Rows - 1 - static_cast<std::size_t>(centerY / TileMap::TileHeight);

        if (col < mapPtr->getColumns() && row < TileMap::Rows) {
            if (mapPtr->getTile(row, col) != '.') {
                pos.x = (col + 1) * TileMap::TileWidth;
                velocity.x = 0.0f;
            }
        }
    }

    if (velocity.x > 0.0f) {
        std::size_t col = static_cast<std::size_t>(rightX / TileMap::TileWidth);
        std::size_t row = TileMap::Rows - 1 - static_cast<std::size_t>(centerY / TileMap::TileHeight);

        if (col < mapPtr->getColumns() && row < TileMap::Rows) {
            if (mapPtr->getTile(row, col) != '.') {
                pos.x = col * TileMap::TileWidth - sz.x;
                velocity.x = 0.0f;
            }
        }
    }

    setPosition(pos);
}

}
