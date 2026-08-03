#ifndef CONTROLLER_PLAYSTATE_H
#define CONTROLLER_PLAYSTATE_H

#include "Controller/GameState.h"
#include "Model/TileMap.h"
#include "View/TileMapRenderer.h"
#include "View/EntityRendererRegistry.h"
#include "View/HudRenderer.h"
#include "Model/Entity.h"
#include "Model/CollisionManager.h"

#include <memory>
#include <vector>

namespace model {
class Player;
}

namespace controller {

class PlayState : public GameState {
public:
    void onEnter() override;
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    void resetLevel();

    model::TileMap map;
    std::unique_ptr<view::TileMapRenderer> renderer;
    bool mapLoaded = false;

    std::unique_ptr<view::EntityRendererRegistry> entityRenderers;
    std::unique_ptr<view::HudRenderer> hudRenderer;

    std::unique_ptr<model::CollisionManager> collisionManager;
    std::vector<std::unique_ptr<model::Entity>> entities;
    model::Player* player = nullptr;  // non-owning: the spawned player entity
};

}

#endif
