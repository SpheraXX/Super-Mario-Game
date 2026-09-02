#ifndef MODEL_LUIGI_H
#define MODEL_LUIGI_H

#include "Model/Player/Player.h"

namespace model {

class Luigi : public Player {
public:
    Luigi(Vector2 position);

    float getWalkSpeed() const override;
    float getRunSpeed() const override;
    float getMaxJumpSpeed() const override;
    float getJumpAccel() const override;
    float getStompBounceRatio() const override;
    float getStompBounceConstant() const override;

    // The view picks its spritesheet rows off this rather than dynamic_casting the
    // concrete type; without the override Luigi would render with Mario's palette.
    bool isLuigi() const override { return true; }

    // Classic Luigi identity (Mario: 90/180/340/1550): quicker on his feet AND a taller,
    // floatier jump — both the higher ceiling (MaxJumpSpeed) and the stronger boost rate
    // (JumpAccel) contribute, so the extra height reads throughout the arc, not just at
    // the top.
    static constexpr float WalkSpeed = 100.0f;
    static constexpr float RunSpeed = 210.0f;
    static constexpr float MaxJumpSpeed = 400.0f;
    static constexpr float JumpAccel = 1650.0f;

    // Luigi's stomp bounce is springier than Mario's: a higher rebound fraction and a
    // smaller subtraction, matching his floaty, high-jump identity. At the same ~350px/s
    // drop he relaunches at ~295px/s instead of Mario's ~267px/s. Playtest starting point.
    static constexpr float StompBounceRatio = 0.9f;
    static constexpr float StompBounceConstant = 20.0f;
};

}

#endif
