#include "Model/Block/Block.h"

#include <cmath>

namespace model {

Block::Block(Vector2 position, Vector2 size, char tileSymbol)
    : Entity(position, size), tileSymbol(tileSymbol), solid(tileSymbol != '.') {
}

void Block::update(float deltaTime) {
    if (bounceElapsed > 0.0f) {
        bounceElapsed -= deltaTime;
        if (bounceElapsed < 0.0f) {
            bounceElapsed = 0.0f;
        }
    }
}

char Block::getTileSymbol() const {
    return tileSymbol;
}

bool Block::isSolid() const {
    return solid;
}

void Block::startBounce() {
    bounceElapsed = BounceDuration;
}

float Block::getBounceOffsetY() const {
    if (bounceElapsed <= 0.0f) {
        return 0.0f;
    }
    // Pop up and settle: sin over the first half of the duration, 0 at both ends.
    const float progress = 1.0f - bounceElapsed / BounceDuration;
    return std::sin(progress * 3.14159265f) * BounceHeight;
}

}
