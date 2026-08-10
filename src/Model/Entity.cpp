#include "Model/Entity.h"

namespace model {

Entity::Entity(Vector2 position, Vector2 size)
    : hitbox({0.0f, 0.0f}, size.x, size.y, false, CollisionLayer::Environment),
      velocity({0.0f, 0.0f}),
      isActive(true),
      isDormant(false),
      isGrounded(false),
      position(position),
      size(size) {
}

void Entity::update(float deltaTime) {
    (void)deltaTime;
}

void Entity::onCollision(Entity& other, CollisionType side) {
    (void)other;
    (void)side;
}

void Entity::onTileCollision(char tile, CollisionType side) {
    (void)tile;
    (void)side;
}

Vector2 Entity::getPosition() const {
    return position;
}

Vector2 Entity::getSize() const {
    return size;
}

void Entity::setPosition(Vector2 newPosition) {
    position = newPosition;
}

void Entity::setSize(Vector2 newSize) {
    size = newSize;
}

Vector2 Entity::getVelocity() const {
    return velocity;
}

void Entity::setVelocity(Vector2 v) {
    velocity = v;
}

}
