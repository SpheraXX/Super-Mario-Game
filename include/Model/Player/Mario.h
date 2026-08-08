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

    static constexpr float WalkSpeed = 180.0f;
    // 400 was too hot on land: with the Overworld drag (0.4/s, see WorldSet) the effective
    // top speed settles around 350px/s — fast enough for a snappy sprint, slow enough for
    // the fixed 20-column camera to keep the player visibly ahead.
    static constexpr float RunSpeed = 360.0f;
    // Safety ceiling only: with the hold window and accel below, the boost tops out near
    // 495px/s, so a full-length hold never reaches this cap.
    static constexpr float MaxJumpSpeed = 680.0f;
    // Jump accel. With Gravity 1600 and the 0.16s hold, the net boost rate is
    // (JumpAccel - Gravity) ≈ 1500px/s^2: a full hold ends the ascent boost at ~470px/s,
    // and the total rise (impulse + boost + coast) comes out ≈137px — BARELY over the
    // 128px of a 4-tile wall, clearly short of the 160px of a 5-tile one. The weak graze
    // at the top of that arc is below the bump-speed gate in CollisionManager, so it
    // cannot open a block 5 tiles above the feet. Tap jumps rise much less.
    static constexpr float JumpAccel = 3100.0f;
};

}

#endif
