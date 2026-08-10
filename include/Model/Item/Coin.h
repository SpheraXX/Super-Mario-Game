#ifndef MODEL_COIN_H
#define MODEL_COIN_H

#include "Model/Item/Item.h"

namespace model {

// The coin that pops out of a bumped block: it springs up, arcs back down under gravity,
// and vanishes shortly after.
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

private:
    float lifetime;

    // Tuned so the arc clears roughly one tile above the block and is gone before the
    // player can land on top of the block and wonder why a coin is still sitting there.
    static constexpr float PopSpeed = -420.0f;
    static constexpr float Lifetime = 0.6f;
};

}

#endif
