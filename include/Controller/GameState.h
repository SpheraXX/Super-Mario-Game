#ifndef CONTROLLER_GAMESTATE_H
#define CONTROLLER_GAMESTATE_H

#include <SFML/Window/Event.hpp>

namespace sf {
class RenderTarget;
}

namespace controller {

class IAudioManager;
class StateManager;

struct GameContext {
    IAudioManager* audio = nullptr;
};

// Abstract base of the State Pattern. Each concrete screen (menu, play, game over)
// derives from this and is owned by the StateManager stack.
class GameState {
public:
    virtual ~GameState() = default;

    // Lifecycle hooks fired by the StateManager when a state is added/removed.
    virtual void onEnter() {}
    virtual void onExit() {}

    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render(sf::RenderTarget& window) = 0;

    // When true, the state directly below this one is still rendered underneath
    // (used for overlay screens such as a future PauseState).
    virtual bool isTransparent() const { return false; }

protected:
    GameContext* context = nullptr;

    // Back-pointer used to request transitions from inside a state. Assigned by the
    // StateManager the moment the state is pushed; never null while the state is live.
    StateManager* manager = nullptr;

    friend class StateManager;
};

}

#endif
