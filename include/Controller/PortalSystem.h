#ifndef CONTROLLER_PORTALSYSTEM_H
#define CONTROLLER_PORTALSYSTEM_H

#include <cstddef>
#include <memory>
#include <vector>

namespace model {
class Entity;
class Level;
class Player;
class Portal;
class TileMap;
}

namespace controller {

// Warp-pipe rules. One-way pipes: the pipe the player arrived through is inert for the
// rest of this visit (the columns are cleared whenever an area is (re)built). This owns
// the pipe/portal matching (a portal is bound to a pipe by its anchor column) and the
// re-emergence placement (on the cap of the destination pipe, else on the ground).
class PortalSystem {
public:
    // Forget every inert column: a fresh visit to an area reactivates all its pipes.
    void clear();

    // Mark the column as inert — the player arrived through this pipe and cannot re-enter.
    void markInert(std::size_t column);

    // The portal the player may enter right now: holding Down while standing on a pipe's
    // cap with a portal bound to that pipe's column. Returns nullptr when the player
    // cannot enter any pipe.
    const model::Portal* findEntryPortal(
        const model::Player& player,
        const model::Level& level,
        std::size_t currentArea,
        const std::vector<std::unique_ptr<model::Entity>>& entities) const;

    // Where the player re-emerges: on the cap of the pipe at the destination column, or
    // on the ground when the column has no pipe. playerHeight is subtracted so the feet
    // land exactly on the surface.
    float landingY(
        const model::TileMap& map,
        const std::vector<std::unique_ptr<model::Entity>>& entities,
        std::size_t column,
        float playerHeight) const;

private:
    std::vector<std::size_t> inertPipeColumns;
};

}

#endif
