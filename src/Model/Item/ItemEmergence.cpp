#include "Model/Item/ItemEmergence.h"
#include "Model/Item/Item.h"

namespace model {

void ItemEmergence::begin(Vector2 blockPosition, Vector2 blockSize) {
    // Full clearance: the item settles exactly on top of the block's top face.
    targetY = blockPosition.y - blockSize.y;
    done = false;
}

bool ItemEmergence::isDone() const {
    return done;
}

bool ItemEmergence::advance(Item& item, float deltaTime) {
    if (done) return false;

    const Vector2 position = item.getPosition();
    const float nextY = position.y - RiseSpeed * deltaTime;
    if (nextY <= targetY) {
        item.setPosition({position.x, targetY});
        done = true;
        return false;
    }
    item.setPosition({position.x, nextY});
    return true;
}

}