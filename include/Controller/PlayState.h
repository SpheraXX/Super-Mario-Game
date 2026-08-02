#ifndef CONTROLLER_PLAYSTATE_H
#define CONTROLLER_PLAYSTATE_H

#include "Controller/GameState.h"
#include "Model/TileMap.h"
#include "View/TileMapRenderer.h"
#include "Model/Entity.h"
#include "Model/CollisionManager.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>

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

    sf::Font font;
    bool fontLoaded = false;

    sf::Texture charsTexture;
    sf::Texture enemiesTexture;
    sf::Texture blocksTexture;  // Bug 5 Fix: used to draw CoinBlock with real art

    std::unique_ptr<model::CollisionManager> collisionManager;
    std::vector<std::unique_ptr<model::Entity>> entities;
    model::Player* player = nullptr;  // non-owning: the spawned player entity
};

}

#endif
