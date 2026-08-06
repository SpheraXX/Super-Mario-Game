#ifndef MODEL_BLOCK_H
#define MODEL_BLOCK_H

#include "Model/Entity.h"

namespace model {

class Block : public Entity {
public:
    Block(Vector2 position, Vector2 size, char tileSymbol);

    void update(float deltaTime) override;

    char getTileSymbol() const;
    bool isSolid() const override;

    // Bump reaction: start a short render-side bounce (only bricks and unopened
    // CoinBlocks do this; see the subclasses' onBlockHit overrides).
    void startBounce();
    // Vertical offset (0 when idle) the renderer adds to the draw position — the
    // hitbox never moves, so the bounce has no physics side effects.
    float getBounceOffsetY() const;

protected:
    float bounceElapsed = 0.0f;
    static constexpr float BounceDuration = 0.22f;
    static constexpr float BounceHeight = 6.0f;

private:
    char tileSymbol;
    bool solid;
};

}

#endif
