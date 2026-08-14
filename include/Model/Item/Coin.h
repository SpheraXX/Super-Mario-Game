#ifndef MODEL_COIN_H
#define MODEL_COIN_H

#include "Model/Item/Item.h"

namespace model {

// The coin that pops out of a bumped block. It is spawned *inside* the block and rises
// out through its top face (drawing behind the block while still inside it), arcs back
// down under gravity, and vanishes the moment it returns to the height it popped from —
// it never falls below its starting position.
//
// Unlike every other Item, this one is *not* collected — the score and the extra-life tally
// are credited at the moment the block is bumped, because the coin is never in doubt. What
// is left is purely the flourish, so it takes no part in collision at all: it is a trigger
// (entity pairs skip it) and it ignores tiles, otherwise it would smack into the underside
// of the very block that produced it.
class Coin : public Item {
public:
    explicit Coin(Vector2 position);

    void update(float deltaTime) override;

    // Already credited at the bump; touching it must not pay twice.
    void onCollect(Entity& collector) override;

    bool usesTileCollision() const override { return false; }

    // The coin lives its whole life in its block's column: drawing in the behind pass
    // lets the block cover it while it is still inside, so it emerges rather than
    // appearing on top.
    bool drawsBehindTerrain() const override { return true; }

private:
    // The height the coin popped from: it disappears when it falls back to it.
    float spawnY;

    // Rise speed for a two-cell pop: the classic one-cell arc used 210, and a pop's
    // height scales with the SQUARE of its speed, so doubling the arc needs ×sqrt(2).
    static constexpr float PopSpeed = -296.98f;
};

}

#endif
