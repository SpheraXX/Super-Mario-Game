#ifndef MODEL_COINBLOCK_H
#define MODEL_COINBLOCK_H

#include "Model/Block/Block.h"

namespace model {

// A '?' block. The first bump from below collects the coin (200 points) and starts a
// short bounce; afterwards it renders as an opened (used) block. The coin is delivered
// through the event-driven hook (BlockHitEvent), not via onCollision — bump detection
// lives in CollisionManager.
class CoinBlock : public Block {
public:
    CoinBlock(Vector2 position, Vector2 size);

    // True while the coin is still inside (closed '?' sprite).
    bool isOpened() const;

    // Pop animation: while the coin is bouncing out of the block the renderer draws the
    // coin sprite rising above it (see CoinBlockRenderer). Timer advances via update().
    bool isCoinPopping() const;
    // 0..1, 0 = just bumped, 1 = pop finished (coin faded out).
    float getCoinPopProgress() const;

    void update(float deltaTime) override;

    // Bumped from below by the player: collects the coin exactly once.
    void onBlockHit(const BlockHitEvent& event) override;

private:
    bool coinAvailable;
    float coinPopElapsed;
    static constexpr float CoinPopDuration = 0.7f;

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
