#include "Model/Block/BrickBlock.h"

#include <fstream>
#include <string>

namespace model {

namespace {
// TEMP trace instrumentation (removed after playtest).
void trace(const std::string& msg) {
    std::ofstream out("trace_log.txt", std::ios::app);
    out << msg << '\n';
}
}

BrickBlock::BrickBlock(Vector2 position, Vector2 size)
    : Block(position, size, '#') {
    // Solid block: same full-size hitbox as every other block.
    hitbox = Hitbox({0.0f, 0.0f}, size.x, size.y, false, CollisionLayer::Environment);
}

void BrickBlock::onBlockHit(const BlockHitEvent& event) {
    (void)event;
    trace("brickBounce");
    startBounce();
}

}
