#ifndef CONTROLLER_LEVELCOMPLETESTATE_H
#define CONTROLLER_LEVELCOMPLETESTATE_H

#include "Controller/GameState.h"

#include <SFML/Graphics/Font.hpp>

namespace controller {

// Overlay shown the moment Mario touches the flagpole. Transparent: the frozen level
// underneath stays visible (StateManager renders transparent states above their base).
// Enter/Space continues to the next map when the finished map declared one
// ('; next=...'), otherwise it returns to the menu — the screen the game starts at.
class LevelCompleteState : public GameState {
public:
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& window) override;
    bool isTransparent() const override { return true; }

private:
    sf::Font font;
    bool fontLoaded = false;
    unsigned int titleSize = 18;
    unsigned int bonusSize = 10;
    unsigned int hintSize = 8;
};

}

#endif
