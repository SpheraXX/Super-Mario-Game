#ifndef MODEL_COINBLOCK_H
#define MODEL_COINBLOCK_H

#include "Model/Block/Block.h"

namespace model {

class CoinBlock : public Block {
public:
    CoinBlock(Vector2 position, Vector2 size);

    bool hasCoin() const;
    void collectCoin();

    // Bumped from below by the player. The first bump spends the block (the renderer then
    // draws the used-block colour) and rolls a reward: a coin by default, or a power-up.
    void onCollision(Entity& other, CollisionType side) override;

    // Reward chances when the block is bumped (they sum to 1.0; the remainder is a coin).
    // Mushroom -> Super state, FireFlower -> Fire state, Starman -> Star state.
    static constexpr float CoinChance = 0.40f;
    static constexpr float MushroomChance = 0.30f;
    static constexpr float FlowerChance = 0.15f;
    static constexpr float StarmanChance = 0.15f;

private:
    bool coinAvailable;
    static constexpr int CoinScore = 200;
};

}

#endif
