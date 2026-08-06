#ifndef MODEL_WORLD_H
#define MODEL_WORLD_H

#include <memory>

namespace model {

class Entity;

// The level, as seen from inside it. Entities never know which controller is running them;
// they only know they can ask the world to take a new entity, or where the player is.
//
// Two needs drive this. Emitters (Hammer Bro, Lakitu, Bowser) must create entities at
// runtime — the level's *initial* population still comes entirely from the map, but a
// thrown hammer cannot. And a few enemies steer by the player's position rather than by
// terrain, which nothing in Entity otherwise exposes.
class World {
public:
    virtual ~World() = default;

    // Hand a newly created entity to the level. Implementations must defer the insertion
    // until the update loop has finished — entities are spawned from inside that loop, and
    // growing the container mid-iteration invalidates it.
    virtual void spawn(std::unique_ptr<Entity> entity) = 0;

    // The player, or nullptr between deaths. Non-owning; never outlives the world.
    virtual const Entity* getPlayer() const = 0;
};

}

#endif
