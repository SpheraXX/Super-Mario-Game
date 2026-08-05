#ifndef CONTROLLER_MENUSTATE_H
#define CONTROLLER_MENUSTATE_H

#include "Controller/GameState.h"

#include <SFML/Graphics/Font.hpp>

namespace controller {

// Opening screen. Enter/Space starts a new game; Escape quits.
class MenuState : public GameState {
public:
    void onEnter() override;
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& window) override;

private:
    sf::Font font;
    bool fontLoaded = false;
};

}

#endif
