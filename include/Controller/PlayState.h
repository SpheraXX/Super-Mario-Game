#ifndef CONTROLLER_PLAYSTATE_H
#define CONTROLLER_PLAYSTATE_H

#include "Controller/GameState.h"
#include "Model/TileMap.h"
#include "View/TileMapRenderer.h"

#include <SFML/Graphics/Font.hpp>

#include <memory>

namespace controller {

// Active gameplay screen. Phase 1 loads and renders the level tilemap as proof of life.
//
// SEAM (Issues 3/4/5): this state will own a `World` (entities + physics). Issue 4's
// LevelLoader/EntityFactory will populate it in onEnter() for the current level; update()
// will step physics and delegate input to the Player; when GameManager reports game over
// the state transitions to GameOverState.
class PlayState : public GameState {
public:
    void onEnter() override;
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    model::TileMap map;
    std::unique_ptr<view::TileMapRenderer> renderer; // built in onEnter (may fail to load)
    bool mapLoaded = false;

    sf::Font font;
    bool fontLoaded = false;
};

}

#endif
