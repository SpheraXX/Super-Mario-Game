#ifndef CONTROLLER_LEVELSCENE_H
#define CONTROLLER_LEVELSCENE_H

#include "Controller/LevelCompletion.h"
#include "Controller/PortalSystem.h"
#include "Model/Core/CollisionManager.h"
#include "Model/Core/LevelTimer.h"
#include "Model/Entity.h"
#include "Model/Level/Level.h"
#include "Model/Map/TileMap.h"
#include "Model/World/WorldType.h"
#include "View/Base/EntityRendererRegistry.h"
#include "View/HitboxRenderer.h"
#include "View/Map/TileMapRenderer.h"

#include <SFML/Graphics/RenderTarget.hpp>

#include <memory>
#include <vector>

namespace model {
class FlagPole;
class Player;
class Portal;
}

namespace controller {

// The live level. Owns the map, the entity list, the collision pass, the per-entity
// renderer registry and the level timer, and knows how to build any area of the loaded
// multi-area level into itself (loadArea/resetLevel/teleportToPortal).
//
// update() runs one frame of normal play and reports what happened through an Event so
// the owning state can react without touching the scene's internals: the flagpole was
// touched (ClearTriggered — the owner starts the scripted clear play) or the player's
// death fall finished (RunEnded — the owner decides restart vs. game over). While a
// cinematic runs, the scene is frozen via setCinematicActive(true) and update() does
// nothing — the cinematic drives the player directly.
class LevelScene {
public:
    enum class Event {
        None,            // ordinary frame
        ClearTriggered,  // the flagpole was touched; the owner should start the clear sequence
        RunEnded,        // the player's death fall finished; the owner decides restart/game over
    };

    LevelScene();

    // Load the map at GameManager::instance().getCurrentMapPath(), publish its metadata
    // where the HUD/completion flow reads it, instantiate area 0 and restart the timer.
    // Returns false (leaving the scene safely empty) when the assets cannot be loaded;
    // the owner logs the failure. Re-entrant: every playthrough builds a fresh scene.
    bool loadLevel();

    // Non-owning accessors the owner reads for input, the clear play and the HUD.
    model::Player* player() const;
    model::FlagPole* flagPole() const;

    // HUD time (whole seconds as shown) and the timer pause used by the clear play.
    int getRemainingTime() const;
    void pauseTimer();

    // Freezes update() while the clear cinematic runs; the owner calls this when the
    // sequence starts and clears it when the sequence is over.
    void setCinematicActive(bool active);

    // Debug: toggle the collision-box overlay (H key).
    void toggleHitboxes();

    // X position of the painted castle's door — the walk target of the clear play.
    float castleDoorX() const;

    Event update(float deltaTime);
    void render(sf::RenderTarget& window);

    // Rebuild the whole entity list from the working grid: called by loadArea and by
    // the owner to restart the level after a death. Idempotent (see the castle paint).
    void resetLevel();

private:
    void loadArea(std::size_t areaIndex);
    void teleportToPortal(const model::Portal& portal);

    model::Level level;
    std::size_t currentArea = 0;

    // Working copy of the current area's grid (final area gets the completion zone).
    model::TileMap map;
    std::unique_ptr<view::TileMapRenderer> renderer;
    bool mapLoaded = false;

    // Warp pipes: entry detection, one-way inert columns and re-emergence placement.
    PortalSystem portals;

    // Goal zone: flagpole + painted castle in the padded columns of the final area.
    LevelCompletion completion;

    std::unique_ptr<view::EntityRendererRegistry> entityRenderers;
    std::unique_ptr<model::CollisionManager> collisionManager;

    std::vector<std::unique_ptr<model::Entity>> entities;
    model::Player* playerPtr = nullptr;  // non-owning: spawned by resetLevel

    model::WorldType worldType = model::WorldType::Overworld;
    view::HitboxRenderer hitboxRenderer;
    bool showHitboxes = true;
    bool cinematicActive = false;

    model::LevelTimer timer;
};

}

#endif