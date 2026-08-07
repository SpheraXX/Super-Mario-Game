#ifndef CONTROLLER_PLAYSTATE_H
#define CONTROLLER_PLAYSTATE_H

#include "Controller/GameState.h"
#include "Model/Core/CollisionManager.h"
#include "Model/Core/LevelTimer.h"
#include "Model/Entity.h"
#include "Model/Map/TileMap.h"
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
}

namespace controller {

// The live level. Owns the map, every entity, the collision pass and the level timer,
// and builds the per-frame HudData snapshot. On flagpole touch it awards the clear
// bonus and pushes the (transparent) LevelCompleteState on top, freezing the level.
class PlayState : public GameState {
public:
    void onEnter() override;
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& window) override;

private:
    void resetLevel();

    model::TileMap map;
    std::unique_ptr<view::TileMapRenderer> renderer;
    bool mapLoaded = false;

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
    bool levelComplete = false;
};

}

#endif
