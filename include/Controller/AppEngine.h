#ifndef CONTROLLER_APPENGINE_H
#define CONTROLLER_APPENGINE_H

#include "Controller/StateManager.h"
#include "Model/Map/TileMap.h"

#include <SFML/Graphics/RenderTexture.hpp>
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

    // The window is fixed (non-resizable) at the logical resolution scaled by this factor;
    // F2 cycles through the scale options at runtime (the offscreen scene never changes).
    static constexpr float DefaultWindowScale = 1.5f;
    static constexpr unsigned int RealScreenWidth = (int)ScreenWidth * DefaultWindowScale;
    static constexpr unsigned int RealScreenHeight = (int)ScreenHeight * DefaultWindowScale;


private:
    void processInput();
    void update(float deltaTime);
    void render();
    void applyWindowScale();

    // Physics/update run at a fixed rate so collision (Issue 3) stays deterministic.
    static constexpr float TimeStep = 1.0f / 60.0f;
    static constexpr float ScaleOptions[3] = {1.0f, 1.5f, 2.0f};

    sf::RenderWindow window;
    StateManager states;
    int scaleIndex = 1;  // starts at 1.5x (DefaultWindowScale)

    // Everything is drawn into this offscreen target at the logical resolution, then blitted
    // to the window once through the fixed view; the window's view transform applies the
    // current scale (1x is a direct 1:1 translation, and larger scales never change the
    // scene itself). Compositing at 1:1 keeps every tile on an exact pixel (no seams) and
    // lets the camera move in whole logical pixels (even scroll), which a fractional scale
    // would otherwise make impossible.
    sf::RenderTexture scene;
    sf::View fixedView;  // always ScreenWidth x ScreenHeight, spans the whole scene target
};

}

#endif
