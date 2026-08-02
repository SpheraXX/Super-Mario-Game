#ifndef CONTROLLER_APPENGINE_H
#define CONTROLLER_APPENGINE_H

#include "Controller/StateManager.h"
#include "Model/TileMap.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

namespace controller {

// Top-level application object: owns the window and the state stack, and runs the core
// game loop (process input -> fixed-step update -> render) until the window closes or the
// state stack empties.
class AppEngine {
public:
    AppEngine();
    void run();

    // Locked logical resolution: the game always renders at this size. If the window is
    // resized, the view is letterboxed so the original resolution still fits, centred in
    // the middle of the window. This keeps every tile/sprite on integer pixels.
    static constexpr unsigned int ScreenWidth = 20 * model::TileMap::TileWidth;
    static constexpr unsigned int ScreenHeight = model::TileMap::Rows * model::TileMap::TileHeight;

private:
    void processInput();
    void update(float deltaTime);
    void render();
    void applyLetterbox();

    // Physics/update run at a fixed rate so collision (Issue 3) stays deterministic.
    static constexpr float TimeStep = 1.0f / 60.0f;

    sf::RenderWindow window;
    StateManager states;
    sf::View letterboxView;  // always ScreenWidth x ScreenHeight, viewport recomputed on resize
};

}

#endif
