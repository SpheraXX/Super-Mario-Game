#ifndef MODEL_BLOCK_BRICKSHARD_H
#define MODEL_BLOCK_BRICKSHARD_H

#include "Model/Character.h"

namespace model {

// One quarter of a BrickBlock's artwork, tossed loose the moment a big player smashes it
// (see BrickBlock::onBlockHit). Derives from Character purely to reuse gravity + velocity
// integration for the toss-then-fall arc; everything else about a Character is neutralised
// below, since a shard is debris, not a participant in play. LevelScene's own world-bounds
// sweep despawns it once it falls past the map, exactly as it already does for anything
// else that walks off the world.
class BrickShard : public Character {
public:
    // `quadrant` (0=top-left, 1=top-right, 2=bottom-left, 3=bottom-right) selects which
    // 8x8 crop of the source brick the renderer draws, so the piece shown is always the
    // piece of artwork it actually broke off of. `launchVelocity` is the initial toss.
    BrickShard(Vector2 position, int quadrant, Vector2 launchVelocity);

    // Takes no part in collision, the same technique a popped Coin uses: debris must fall
    // through the floor it broke out of rather than land on it.
    bool usesTileCollision() const override { return false; }

    int getQuadrant() const;

private:
    int quadrant;
};

}

#endif
