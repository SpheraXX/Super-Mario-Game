#ifndef MODEL_CORE_VERTICALSLIDE_H
#define MODEL_CORE_VERTICALSLIDE_H

#include "Model/Core/Vector2.h"

namespace model {

class Entity;

// One-shot straight-line motion shared by every slide in the game: an item rising out of
// a block, Mario sinking into a vertical pipe and rising back out of it, or sinking into a
// horizontal pipe's face. The owning entity ticks it via advance() every frame while it
// runs; during the slide the entity is inert — the component owns the motion — and the
// owner is drawn behind the terrain so it never overdraws the block or pipe it passes
// through.
class VerticalSlide {
public:
    // Which coordinate the slide drives. Defaults to Vertical: every caller predating the
    // horizontal pipe (item pop, vertical pipe entry) moves along y and never passes this.
    enum class Axis { Vertical, Horizontal };

    // Seeds a slide from start to target at the given speed, along axis; the direction is
    // derived from the two endpoints (positive when the target is past the start). start
    // is the position the slide begins from, which the owner also sets on the entity.
    void begin(float start, float target, float speed, Axis axis = Axis::Vertical);

    // True while the slide has not yet reached its target.
    bool isDone() const;

    // Advances the slide by deltaTime, moving the entity directly along its axis (no
    // physics while the slide runs). Returns false when the target has just been reached,
    // which signals the owner to drop the slide state and resume normal behaviour.
    bool advance(Entity& entity, float deltaTime);

    // The default block-pop pace: one tile (16 world units) in a brisk 0.2s. Tune here,
    // not per user: every slide shares the same feel.
    static constexpr float RiseSpeed = 80.0f;

private:
    float target = 0.0f;
    float speed = RiseSpeed;
    float direction = 0.0f;
    bool done = false;
    Axis axis = Axis::Vertical;
};

}

#endif
