#ifndef MODEL_MARIOFIREBALL_H
#define MODEL_MARIOFIREBALL_H

#include "Model/Projectile/Projectile.h"

namespace model {

// Mario's bouncing fireball, shot by Fire (or Star-wrapped-Fire) Mario. Rolls forward at a
// constant speed, bounces off the ground instead of resting, is destroyed by side walls, and
// kills any enemy it touches — items pass straight through it. The 7x7 source frame maps 1:1
// onto a 7x7 world box.
//
// Kept separate from Fireball (Bowser's breath, which flies flat through walls): this one
// falls under gravity and resolves against tiles, so it needs its own update and tile hook.
class MarioFireball : public Projectile {
public:
    MarioFireball(Vector2 position, Entity* owner, int direction);

    void update(float deltaTime) override;
    void onCollision(Entity& other, CollisionType side) override;
    void onTileCollision(char tile, CollisionType side) override;
    bool usesTileCollision() const override { return true; }

    // Rolling animation clock for the view; the four frames in atlas::FireballRoll cycle
    // off this, like the player's walk clock.
    float getAnimationClock() const { return animationClock; }

    static constexpr float Width = 7.0f;

private:
    static constexpr float Height = Width;
    static constexpr float TravelSpeed = 120.0f;
    static constexpr float BounceSpeed = 160.0f;

    float animationClock = 0.0f;
};

}

#endif
