#ifndef MODEL_LEVEL_LAVABUBBLE_H
#define MODEL_LEVEL_LAVABUBBLE_H

#include "Model/Projectile/Projectile.h"

namespace model {

// The big fireball that leaps out of a castle's lava and drops back into it, forever, in
// one fixed column. A hazard on the Projectile layer for the same reasons FirebarBall is:
// contact damage, not stompable, not killable, and it does not expire when it burns you.
//
// The leap is a pure function of elapsed time (a parabola over one period, then a rest at
// the bottom) rather than an integrated launch. That is the same choice Slider makes and
// for the same reason: an entity whose whole job is to repeat a motion exactly cannot be
// allowed to accumulate drift, and there is no "relaunch" edge case to get wrong.
class LavaBubble : public Projectile {
public:
    // `origin` is the bottom of the arc — the cell the author marked, which should sit in
    // the lava. `riseHeight` is how far above it the bubble peaks, in world units.
    LavaBubble(Vector2 origin, float riseHeight, float leapSeconds, float restSeconds,
               float phase = 0.0f);

    void update(float deltaTime) override;

    // Lives in the lava and leaps through open air; the tile pass has nothing useful to
    // say about either, and would evict it from its own pool.
    bool usesTileCollision() const override { return false; }

    void onCollision(Entity& other, CollisionType side) override;

    // True while the bubble is on its way down, so the renderer can flip the flame over.
    bool isFalling() const { return falling; }

private:
    Vector2 origin;
    float riseHeight;
    float leapSeconds;
    float restSeconds;
    float elapsed;
    bool falling = false;
};

}

#endif
