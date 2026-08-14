#ifndef MODEL_COINBLOCK_H
#define MODEL_COINBLOCK_H

#include "Model/Block/Block.h"

namespace model {

// A '?' block. The first bump from below collects the reward (weighted heavily towards a
// plain coin) and starts a short bounce; afterwards it renders as an opened (used) block.
// Rewards are delivered through the event-driven hook (BlockHitEvent), not via onCollision
// — bump detection lives in CollisionManager. The coin is a spawned Coin entity that pops
// out of the block's own cell; power-ups are spawned at the cell above it.
class CoinBlock : public Block {
public:
    CoinBlock(Vector2 position, Vector2 size);

    // True while the coin is still inside (closed '?' sprite).
    bool isOpened() const;

    // Bumped from below by the player: collects the reward exactly once.
    void onBlockHit(const BlockHitEvent& event) override;

private:
    bool coinAvailable;

    // Reward table, rolled once per bump. Weighted heavily towards coins: a power-up from
    // every '?' block reads as far too generous for a Mario level, so the item rate is kept
    // low and the star rarest of all. Must sum to 1.0.
    static constexpr float MushroomChance = 0.15f;
    static constexpr float FlowerChance   = 0.05f;
    static constexpr float StarmanChance  = 0.05f;
    // Remainder (0.75) is a plain coin.
    static constexpr int CoinScore = 200;
};

}

#endif
