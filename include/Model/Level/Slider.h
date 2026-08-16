#ifndef MODEL_LEVEL_SLIDER_H
#define MODEL_LEVEL_SLIDER_H

#include "Model/Entity.h"

namespace model {

// A platform that moves back and forth at a constant speed along one axis, carrying
// whatever stands on top of it. Author-placed as a 2-cell run of TileMap::SliderSymbol
// ('=', giving the platform its fixed shape) bound to its motion by a '; slider=' header
// token keyed to the run's leftmost column — the same scheme '; pipe=' uses to bind a
// Portal to a Pipe entity.
//
// Not a Character: a slider has no gravity, health or death, it moves on a fixed rule of
// its own rather than physics. It is solid so the player collides with it like any other
// block (see CollisionManager::resolveEntityInteraction), and it reports how far it moved
// THIS FRAME through getCarryDelta() so a riding player can be carried horizontally without
// either side knowing about the other's implementation. The vertical case needs no such
// help: pushOutOfBlock already re-snaps the rider to the platform's current top every frame
// contact holds, whichever way the platform is moving.
class Slider : public Entity {
public:
    enum class Axis { Horizontal, Vertical };

    Slider(Vector2 position, Vector2 size, Axis axis, float travelDistance, float speed);

    void update(float deltaTime) override;

    bool isSolid() const override { return true; }
    Vector2 getCarryDelta() const override { return lastDelta; }

private:
    Vector2 origin;
    Axis axis;
    float travelDistance;
    float speed;
    float totalTime = 0.0f;
    Vector2 lastDelta{0.0f, 0.0f};
};

}

#endif
