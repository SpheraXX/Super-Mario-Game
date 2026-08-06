#ifndef MODEL_BRICKBLOCK_H
#define MODEL_BRICKBLOCK_H

#include "Model/Block/Block.h"

namespace model {

// A solid brick ('#' / 'B' in the map). Every bump from below triggers a bounce;
// unlike the CoinBlock it never breaks and holds nothing.
class BrickBlock : public Block {
public:
    BrickBlock(Vector2 position, Vector2 size);

    void onBlockHit(const BlockHitEvent& event) override;
};

}

#endif
