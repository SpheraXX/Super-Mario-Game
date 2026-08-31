#ifndef MODEL_WORLD_H
#define MODEL_WORLD_H

#include <cstddef>
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

    // A destructible block has been destroyed: the level erases its cell from the static
    // map, so tile collision (landing on the block's spot) and any tile rendering forget
    // it as well as the entity pass already does.
    virtual void removeTile(std::size_t row, std::size_t column) = 0;

    // Erase EVERY cell carrying `symbol` from the area's map in one go. Distinct from
    // removeTile above, which is the "this one block was just smashed" case: this is for a
    // switch that retires a whole class of terrain at once — the castle axe cutting the
    // bridge — where the trigger knows what to remove but not where any of it is.
    virtual void removeTilesOfType(char symbol) = 0;
};

}

#endif
