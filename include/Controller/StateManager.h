#ifndef CONTROLLER_STATEMANAGER_H
#define CONTROLLER_STATEMANAGER_H

#include "Controller/GameState.h"

#include <memory>
#include <vector>

namespace sf {
class RenderTarget;
}

namespace controller {

struct GameContext;

// Owns the stack of game states and drives the active one. Transition requests
// (push/pop/replace/clear) are queued and applied only at the end of the frame via
// applyPending(), so a state may safely mutate the stack from within its own update().
class StateManager {
public:
    // Runs an initial state immediately, skipping the 'pending' queue mechanics.
    StateManager(std::unique_ptr<GameState> initial, GameContext* ctx);

    void pushState(std::unique_ptr<GameState> state);
    void popState();
    void replaceState(std::unique_ptr<GameState> state);
    void clear();

    // Called once per frame by AppEngine, after update/render, to enact queued changes.
    void applyPending();

    void handleEvent(const sf::Event& event);
    void update(float deltaTime);
    void render(sf::RenderTarget& window);

    bool empty() const;
    GameState* activeState();

private:
    enum class Action { Push, Pop, Replace, Clear };

    struct PendingChange {
        Action action;
        std::unique_ptr<GameState> state; // populated for Push / Replace only
    };

    std::vector<std::unique_ptr<GameState>> stack;
    std::vector<PendingChange> pending;
    GameContext* context = nullptr;
};

}

#endif
