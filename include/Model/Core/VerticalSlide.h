#ifndef MODEL_CORE_VERTICALSLIDE_H
#define MODEL_CORE_VERTICALSLIDE_H

#include "Model/Core/Vector2.h"

namespace model {

class Entity;

// One-shot vertical motion shared by every slide in the game: an item rising out of a
// block, Mario sinking into a pipe and rising back out of it. The owning entity ticks
// it via advance() every frame while it runs; during the slide the entity is inert —
// the component owns the motion — and the owner is drawn behind the terrain so it
// never overdraws the block or pipe it passes through.
class VerticalSlide {
public:
    // Seeds a slide from startY to targetY at the given speed; the direction is derived
    // from the two endpoints (down when the target is below the start). startY is the
    // position the slide begins from, which the owner also sets on the entity.
    void begin(float startY, float targetY, float speed);

    // True while the slide has not yet reached its target.
    bool isDone() const;

    // Advances the slide by deltaTime, moving the entity directly (no physics while the
    // slide runs). Returns false when the target has just been reached, which signals
    // the owner to drop the slide state and resume normal behaviour.
    bool advance(Entity& entity, float deltaTime);

    // The default block-pop pace: one tile (16 world units) in a brisk 0.2s. Tune here,
    // not per user: every slide shares the same feel.
    static constexpr float RiseSpeed = 80.0f;

private:
    float targetY = 0.0f;
    float speed = RiseSpeed;
    float direction = 0.0f;
    bool done = false;
};

}

#endif
