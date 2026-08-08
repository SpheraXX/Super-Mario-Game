#ifndef CONTROLLER_GAMEOVERSTATE_H
#define CONTROLLER_GAMEOVERSTATE_H

#include "Controller/GameState.h"

#include <SFML/Graphics/Font.hpp>

namespace controller {

// Shown when the run ends. Displays the final score and returns to the menu on Enter,
// resetting progress for the next run.
class GameOverState : public GameState {
public:
    void onEnter() override;
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& window) override;

private:
    sf::Font font;
    bool fontLoaded = false;
    unsigned int titleSize = 56;
    unsigned int scoreSize = 26;
    unsigned int hintSize = 20;
};

}

#endif
