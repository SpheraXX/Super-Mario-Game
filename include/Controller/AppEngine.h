#ifndef CONTROLLER_APPENGINE_H
#define CONTROLLER_APPENGINE_H

#include "Controller/StateManager.h"
#include "Model/Map/TileMap.h"

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

#include <cstddef>

namespace controller {

// Top-level application object: owns the window and the state stack, and runs the core
// game loop (process input -> fixed-step update -> render) until the window closes or the
// state stack empties.
class AppEngine {
public:
    AppEngine();
    void run();

    // The logical frame, in world units. One world unit is one pixel of source art, and one
    // tile is TileMap::TileWidth of them, so the logical frame is the true pixel-art
    // resolution and every magnification below is a whole-number blow-up of it.
    //
    // The play area is always exactly TileMap::Rows tiles tall whatever size the player
    // picks, so the logical HEIGHT is a compile-time constant. The logical WIDTH is not:
    // the magnification is derived from the height alone, and the width is however many
    // columns the window's aspect ratio leaves room for — so a wider window sees further
    // along the level instead of showing the same view stretched. Screen-space code
    // therefore has to ask for the width rather than read a constant.
    static constexpr unsigned int ScreenHeight =
        model::TileMap::Rows * model::TileMap::TileHeight;
    static unsigned int screenWidth();

    // One entry in the size menu. A window is logicalWidth*scale by ScreenHeight*scale
    // pixels, so every size offered is an exact integer multiple of the logical frame:
    // there is no fractional scaling anywhere, and each source pixel lands on a whole
    // square block of screen pixels. Both dimensions of each entry are multiples of 16.
    //
    // logicalWidth must be >= ScreenHeight (the width >= height assumption): the HUD and
    // the camera are laid out for a frame at least as wide as it is tall.
    struct DisplayOption {
        unsigned int logicalWidth;  // world units across
        unsigned int scale;         // integer magnification
    };
    static constexpr DisplayOption SizeOptions[] = {
        {384, 2},  //  768 x 512  — 24 columns
        {448, 3},  // 1344 x 768  — 28 columns
        {480, 4},  // 1920 x 1024 — 30 columns
    };
    static constexpr std::size_t SizeOptionCount =
        sizeof(SizeOptions) / sizeof(SizeOptions[0]);

private:
    void processInput();
    void update(float deltaTime);
    void render();

    // (Re)create the window, the offscreen target and both views for the current
    // selection. Runs once at start-up and again whenever the size is cycled.
    void applyDisplayMode();
    // Advance the selection: the windowed sizes in order, then fullscreen, then back.
    void cycleDisplayMode();

    // Physics/update run at a fixed rate so collision (Issue 3) stays deterministic.
    static constexpr float TimeStep = 1.0f / 60.0f;

    // Height of the window chrome (title bar + borders), so a windowed size that would
    // otherwise exactly fill the desktop still leaves its whole client area on screen.
    static constexpr unsigned int WindowChrome = 64;

    sf::RenderWindow window;
    StateManager states;

    // Current selection. `fullscreen` overrides the index rather than being a fourth entry
    // in SizeOptions, because its logical width is measured off the display, not listed.
    std::size_t sizeIndex = 0;
    bool fullscreen = false;
    // Set when F2 is seen and acted on after the event queue is drained: recreating the
    // window in the middle of polling it would pull the queue out from under the loop.
    bool displayChangePending = false;

    // Logical width of the current mode (ScreenHeight is fixed). Static so the HUD, the
    // menus and the level camera can reach it the same way they already reached
    // ScreenHeight, without threading an engine pointer through the state stack.
    static unsigned int logicalWidth;

    // Everything is drawn into this offscreen target at the logical resolution, then blitted
    // to the window once. Compositing at 1:1 keeps every tile on an exact pixel (no seams)
    // and lets the camera move in whole logical pixels (even scroll); the single scaled blit
    // afterwards cannot introduce seams between tiles the way scaling each sprite would.
    sf::RenderTexture scene;
    sf::View sceneView;    // inside the offscreen target: the whole logical frame
    sf::View presentView;  // blitting to the window: centred, integer-scaled, letterboxed
};

}

#endif
