#include "Model/Core/Hitbox.h"
#include <algorithm>
#include <cmath>

namespace model {

bool Hitbox::intersects(const Hitbox& other, Vector2 myPos, Vector2 otherPos) const {
    float myLeft = myPos.x + offset.x;
    float myRight = myLeft + width;
    float myTop = myPos.y + offset.y;
    float myBottom = myTop + height;

    float otherLeft = otherPos.x + other.offset.x;
    float otherRight = otherLeft + other.width;
    float otherTop = otherPos.y + other.offset.y;
    float otherBottom = otherTop + other.height;

    return (myLeft < otherRight && myRight > otherLeft &&
            myTop < otherBottom && myBottom > otherTop);
}

Vector2 Hitbox::getOverlap(const Hitbox& other, Vector2 myPos, Vector2 otherPos) const {
    float myLeft = myPos.x + offset.x;
    float myRight = myLeft + width;
    float myTop = myPos.y + offset.y;
    float myBottom = myTop + height;

    float otherLeft = otherPos.x + other.offset.x;
    float otherRight = otherLeft + other.width;
    float otherTop = otherPos.y + other.offset.y;
    float otherBottom = otherTop + other.height;

    Vector2 overlap = {0.0f, 0.0f};

    if (intersects(other, myPos, otherPos)) {
        float overlapLeft = myRight - otherLeft;
        float overlapRight = otherRight - myLeft;
        float overlapTop = myBottom - otherTop;
        float overlapBottom = otherBottom - myTop;

        overlap.x = (overlapLeft < overlapRight) ? overlapLeft : -overlapRight;
        overlap.y = (overlapTop < overlapBottom) ? overlapTop : -overlapBottom;
    }

    return overlap;
}

}
