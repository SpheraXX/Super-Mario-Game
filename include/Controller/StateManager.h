#ifndef CONTROLLER_STATEMANAGER_H
#define CONTROLLER_STATEMANAGER_H

#include "Controller/GameState.h"

#include <memory>
#include <vector>

namespace sf {
class RenderWindow;
}

namespace controller {

// Owns the stack of game states and drives the active one. Transition requests
// (push/pop/replace/clear) are queued and applied only at the end of the frame via
// applyPending(), so a state may safely mutate the stack from within its own update().
class StateManager {
public:
    void pushState(std::unique_ptr<GameState> state);
    void popState();
    void replaceState(std::unique_ptr<GameState> state);
    void clear();

    // Called once per frame by AppEngine, after update/render, to enact queued changes.
    void applyPending();

    void handleEvent(const sf::Event& event);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);

    bool empty() const;

private:
    enum class Action { Push, Pop, Replace, Clear };

    struct PendingChange {
        Action action;
        std::unique_ptr<GameState> state; // populated for Push / Replace only
    };

    GameState* activeState();

    std::vector<std::unique_ptr<GameState>> stack;
    std::vector<PendingChange> pending;
};

}

#endif
