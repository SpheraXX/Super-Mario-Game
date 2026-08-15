#ifndef MODEL_BLOCK_H
#define MODEL_BLOCK_H

#include "Model/Entity.h"
#include "Model/Core/BlockHitEvent.h"

namespace model {

class Block : public Entity {
public:
    Block(Vector2 position, Vector2 size, char tileSymbol);

    void update(float deltaTime) override;

    char getTileSymbol() const;
    bool isSolid() const override;

    // The player bumped this block from below (see BlockHitEvent). Only block
    // subclasses react (bounce / collect a coin); other solid objects like pipes
    // never receive this hook.
    //
    // Returns whether the block actually reacted to the bump. The collision pass
    // only counts the bump as dispatched (reacting with whatever stands on the
    // block's top face) when this returns true — so a spent block behaves like a
    // plain ground tile.
    virtual bool onBlockHit(const BlockHitEvent& event) { (void)event; return false; }

    // Bump reaction: start a short render-side bounce (only bricks and unopened
    // CoinBlocks do this; see the subclasses' onBlockHit overrides).
    void startBounce();
    // Vertical offset (0 when idle) the renderer adds to the draw position — the
    // hitbox never moves, so the bounce has no physics side effects.
    float getBounceOffsetY() const;

protected:
    float bounceElapsed = 0.0f;
    static constexpr float BounceDuration = 0.22f;
    static constexpr float BounceHeight = 3.0f;

private:
    char tileSymbol;
    bool solid;
};

}

#endif
