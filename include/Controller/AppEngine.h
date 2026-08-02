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

    // Locked logical resolution: the game always renders at this size, so every
    // tile/sprite stays on integer pixels.
    static constexpr unsigned int ScreenWidth = 20 * model::TileMap::TileWidth;
    static constexpr unsigned int ScreenHeight = model::TileMap::Rows * model::TileMap::TileHeight;

    // The window is fixed (non-resizable) at the logical resolution scaled by this factor.
    // Set to 2 on larger monitors to double the size; the logical resolution never changes.
    static constexpr unsigned int WindowScale = 1;

private:
    void processInput();
    void update(float deltaTime);
    void render();

    // Physics/update run at a fixed rate so collision (Issue 3) stays deterministic.
    static constexpr float TimeStep = 1.0f / 60.0f;

    sf::RenderWindow window;
    StateManager states;
    sf::View fixedView;  // always ScreenWidth x ScreenHeight, spans the whole window
};

}

#endif
