#ifndef MODEL_MAPCOIN_H
#define MODEL_MAPCOIN_H

#include "Model/Item/Item.h"

namespace model {

// A coin the level author placed in the world, occupying one cell, collected by walking
// through it.
//
// This is a different object from Coin, despite the shared artwork. Coin is the flourish
// that pops out of a bumped block: it is never collected (the bump already paid), takes no
// part in collision, and deletes itself when its arc ends. This one is the opposite in
// every respect — it is a genuine pickup that sits still until the player touches it, so it
// is an ordinary Item and rides the normal collect path (Item::onCollision -> onCollect).
//
// It must NOT be a trigger hitbox: the collision pass routes trigger pairs to
// onTriggerEnter and skips the interaction resolve entirely, which is exactly how Coin
// stays uncollectable. Leaving the default (non-trigger) is what makes this one work.
class MapCoin : public Item {
public:
    explicit MapCoin(Vector2 position);

    // Credit the coin and remove it. GameManager::addCoin already rolls every hundredth
    // coin into an extra life, so that comes for free.
    void onCollect(Entity& collector) override;

    // It hangs exactly where the author placed it: no gravity, and nothing to resolve
    // against terrain. A coin sitting inside a cell would otherwise be pushed by the tile
    // pass or fall out of the level.
    bool usesTileCollision() const override { return false; }

    // Drives the view's frame cycle. Kept on the model, like MarioFireball's roll clock,
    // so the renderer stays stateless.
    float getAnimationClock() const { return animationClock; }

protected:
    // Overridden to do nothing but advance the clock: the base would apply gravity and
    // integrate a velocity this entity is never meant to have.
    void updateBehavior(float deltaTime) override;

private:
    float animationClock = 0.0f;

    // Matches CoinBlock::CoinScore, so a coin is worth the same however it was obtained.
    static constexpr int CoinScore = 200;
};

}

#endif
