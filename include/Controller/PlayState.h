#ifndef CONTROLLER_PLAYSTATE_H
#define CONTROLLER_PLAYSTATE_H

#include "Controller/GameState.h"
#include "Model/Map/TileMap.h"
#include "View/Map/TileMapRenderer.h"
#include "View/Base/EntityRendererRegistry.h"
#include "View/HitboxRenderer.h"
#include "View/HudRenderer.h"
#include "Model/Entity.h"
#include "Model/Core/CollisionManager.h"
#include "Model/Core/World.h"

#include <memory>
#include <vector>

namespace model {
class Player;
}

namespace controller {

// Runs a level. Doubles as the model::World the level's entities see, which is how a
// Hammer Bro gets a hammer into the world without knowing a controller exists.
class PlayState : public GameState, public model::World {
public:
    void onEnter() override;
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& window) override;

    // model::World
    void spawn(std::unique_ptr<model::Entity> entity) override;
    const model::Entity* getPlayer() const override;

private:
    void resetLevel();
    void updateCamera();
    void armDormancy();
    // Take ownership of an entity immediately (level build time, outside the update loop).
    model::Entity* addEntity(std::unique_ptr<model::Entity> entity);

    // How far beyond the right edge of the view an entity wakes. A small lead-in means
    // enemies are already moving by the time they scroll into sight, instead of popping.
    static constexpr float ActivationMargin = 64.0f;

    // How far past the edge of the view a projectile survives before it is reclaimed.
    static constexpr float DespawnMargin = 64.0f;

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

    // Which playable character the level currently runs: toggled with the C key, and the
    // level restarts under the new character (resetLevel reads it when spawning the player).
    bool playAsLuigi = false;
    // Spawned mid-update and spliced in once the loop is over: growing `entities` while
    // iterating it invalidates the iterator.
    std::vector<std::unique_ptr<model::Entity>> pendingEntities;
    model::Player* player = nullptr;  // non-owning: the spawned player entity

    // Camera centre in world space, recomputed at the end of every update so that render()
    // and the next frame's activation check agree on where the view is.
    float cameraX = 0.0f;
    // High-water mark of the camera's right edge: entities to the left of it are awake.
    // Monotonic, so walking back left never re-arms an enemy that has already woken.
    float activationFrontier = 0.0f;
};

}

#endif
