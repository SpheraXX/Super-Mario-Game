#ifndef MODEL_ITEM_EMERGENCE_H
#define MODEL_ITEM_EMERGENCE_H

#include "Model/Core/Vector2.h"

namespace model {

class Item;

// Block-pop animation for an item: it rises straight up out of the block's own cell,
// with no gravity or horizontal motion, until it has fully cleared the block's top
// face. The Item owns one of these while the pop is running and ticks it every frame;
// while the pop runs the item is inert and is drawn behind the terrain so it never
// overdraws the block it is coming out of.
class ItemEmergence {
public:
    // Seeds a fresh pop out of the given block.
    void begin(Vector2 blockPosition, Vector2 blockSize);

    // True while the item has not yet reached its final height.
    bool isDone() const;

    // Advances the rise by deltaTime, moving the item directly (no physics while the
    // pop runs). Returns false when the item has just reached its final height, which
    // signals the owner to drop the pop state and resume normal behaviour.
    bool advance(Item& item, float deltaTime);

private:
    static constexpr float RiseSpeed = 60.0f;

    float targetY;
    bool done = false;
};

}

#endif