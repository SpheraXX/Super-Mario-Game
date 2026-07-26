#ifndef CONTROLLER_APPENGINE_H
#define CONTROLLER_APPENGINE_H

#include "Controller/StateManager.h"

#include <SFML/Graphics/RenderWindow.hpp>

namespace controller {

// Top-level application object: owns the window and the state stack, and runs the core
// game loop (process input -> fixed-step update -> render) until the window closes or the
// state stack empties.
class AppEngine {
public:
    AppEngine();
    void run();

private:
    void processInput();
    void update(float deltaTime);
    void render();

    // Physics/update run at a fixed rate so collision (Issue 3) stays deterministic.
    static constexpr float TimeStep = 1.0f / 60.0f;

    sf::RenderWindow window;
    StateManager states;
};

}

#endif
