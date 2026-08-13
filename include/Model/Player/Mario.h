#ifndef MODEL_MARIO_H
#define MODEL_MARIO_H

#include "Model/Player/Player.h"

namespace model {

class Mario : public Player {
public:
    Mario(Vector2 position);

    float getWalkSpeed() const override;
    float getRunSpeed() const override;
    float getMaxJumpSpeed() const override;
    float getJumpAccel() const override;

    static constexpr float WalkSpeed = 90.0f;
    // 400 (feat's value) was too hot on land for the fixed 20-column camera to keep the
    // player visibly ahead. 360 is now the true top speed: the Overworld's horizontal drag
    // used to bleed this down to ~350, but that drag was removed when the ground feel was
    // retuned (see WorldSet and Player's accel constants), so what is written here is what
    // the player actually reaches.
    static constexpr float RunSpeed = 180.0f;
    // Safety ceiling only: with the hold window and accel below, the boost tops out near
    // 495px/s, so a full-length hold never reaches this cap.
    static constexpr float MaxJumpSpeed = 340.0f;
    // Jump accel. With Gravity 1600 and the 0.16s hold, the net boost rate is
    // (JumpAccel - Gravity) ≈ 1500px/s^2: a full hold ends the ascent boost at ~470px/s,
    // and the total rise (impulse + boost + coast) comes out ≈137px — BARELY over the
    // 128px of a 4-tile wall, clearly short of the 160px of a 5-tile one. The weak graze
    // at the top of that arc is below the bump-speed gate in CollisionManager, so it
    // cannot open a block 5 tiles above the feet. Tap jumps rise much less.
    static constexpr float JumpAccel = 1550.0f;
};

}

#endif
