#include "Model/Entity.h"

namespace model {

Entity::Entity(Vector2 position, Vector2 size)
    : position(position), size(size) {
}

void Entity::update(float deltaTime) {
    (void)deltaTime;
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

}
