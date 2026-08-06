#ifndef MODEL_COINBLOCK_H
#define MODEL_COINBLOCK_H

#include "Model/Block/Block.h"

namespace model {

class CoinBlock : public Block {
public:
    CoinBlock(Vector2 position, Vector2 size);

    bool hasCoin() const;
    void collectCoin();

    // Bumped from below by the player: collects the coin once.
    void onCollision(Entity& other, CollisionType side) override;

private:
    bool coinAvailable;
};

}

#endif
