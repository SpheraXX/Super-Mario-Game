#ifndef MODEL_BRICKBLOCK_H
#define MODEL_BRICKBLOCK_H

#include "Model/Block/Block.h"

namespace model {

// A solid brick ('#' / 'B' in the map). A small bumper only makes it bounce; a big
// one (canBreakBricks) smashes it apart on the spot, scoring like the original.
class BrickBlock : public Block {
public:
    BrickBlock(Vector2 position, Vector2 size);

    // A brick always reacts to a bump (break or bounce), so it reports true.
    bool onBlockHit(const BlockHitEvent& event) override;

private:
    // Points for smashing a brick, matching the original.
    static constexpr int BreakScore = 50;

    // Erase the brick's cell from the static map through the world channel.
    void eraseFromMap();
};

}

#endif
