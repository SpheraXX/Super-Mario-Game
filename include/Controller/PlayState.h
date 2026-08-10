#ifndef CONTROLLER_PLAYSTATE_H
#define CONTROLLER_PLAYSTATE_H

#include "Controller/GameState.h"
#include "Model/Core/CollisionManager.h"
#include "Model/Core/LevelTimer.h"
#include "Model/Entity.h"
#include "Model/Level/Level.h"
#include "Model/World/WorldType.h"
#include "View/Base/EntityRendererRegistry.h"
#include "View/HitboxRenderer.h"
#include "View/HudData.h"
#include "View/HudRenderer.h"
#include "View/Map/TileMapRenderer.h"

#include <memory>
#include <vector>

namespace model {
class Player;
class FlagPole;
class Castle;
class Pipe;
}

namespace controller {

// The live level. Owns the map, every entity, the collision pass and the level timer,
// and builds the per-frame HudData snapshot.
//
// On flagpole touch a short scripted clear play runs before the completion overlay:
// Mario snaps to the pole, slides to the ground while the pennant drops (SlideToPole),
// then walks to the castle door (WalkToCastle) and stops (ReachedCastle) — only then is
// the transparent LevelCompleteState pushed on top, freezing the finished tableau.
class PlayState : public GameState {
public:
    void onEnter() override;
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& window) override;

private:
    enum class ClearPhase {
        None,           // normal play
        SlideToPole,    // penguin + mario slide down the pole
        WalkToCastle,   // mario auto-walks from the pole to the castle
        ReachedCastle,  // mario stands at the door; push the completion overlay
    };

    void resetLevel();
    void beginLevelClear();
    void updateClearSequence(float deltaTime);
    void finishLevelClear();

    // Load the current area's map into the working `map`, spawn the area and (only on
    // the final area) append the completion zone, then place the player at a column.
    void loadArea(std::size_t areaIndex);
    void teleportToPortal(const model::Portal& portal);

    // The multi-area level behind this playthrough. Only the CURRENT area is instantiated
    // (map + entities); pipes carry the portal that teleports to another area.
    model::Level level;
    std::size_t currentArea = 0;

    // Working copy of the current area's grid (final area gets the completion zone).
    model::TileMap map;
    std::unique_ptr<view::TileMapRenderer> renderer;
    bool mapLoaded = false;

    // Columns whose pipe is inert for this area visit: the pipe the player just exited
    // from cannot be re-entered until the area is revisited (one-way warp pipes).
    std::vector<std::size_t> inertPipeColumns;

    std::unique_ptr<view::EntityRendererRegistry> entityRenderers;
    std::unique_ptr<view::HudRenderer> hudRenderer;

    // Debug hitbox overlay, toggled with H.
    view::HitboxRenderer hitboxRenderer;
    bool showHitboxes = true;

    std::unique_ptr<model::CollisionManager> collisionManager;
    std::vector<std::unique_ptr<model::Entity>> entities;
    model::Player* player = nullptr;  // non-owning: the spawned player entity

    model::WorldType worldType = model::WorldType::Overworld;
    model::LevelTimer timer;
    view::HudData hudData;

    model::FlagPole* flagPole = nullptr;  // non-owning: spawned in resetLevel
    model::Castle* castle = nullptr;      // non-owning: spawned in resetLevel
    bool levelComplete = false;

    // Level-clear in the play state (see updateClearSequence).
    ClearPhase clearPhase = ClearPhase::None;
    float poleElapsed = 0.0f;
    float poleSlideStartY = 0.0f;
    float poleGroundY = 0.0f;
    float castleEntryX = 0.0f;
    bool completionOverlayPushed = false;
};

}

#endif
