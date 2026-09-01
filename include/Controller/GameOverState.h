#ifndef CONTROLLER_GAMEOVERSTATE_H
#define CONTROLLER_GAMEOVERSTATE_H

#include "Controller/GameState.h"
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include "View/UI/UIContainer.h"
#include "View/UI/UILabel.h"
#include "View/Effect/LerpAnimator.h"
#include <functional>

namespace controller {

// Shown when the run ends. Displays the final score and returns to the menu on Enter,
// resetting progress for the next run.
class GameOverState : public GameState {
public:
    GameOverState(std::function<void()> onRestartCallback);

    void onEnter() override;
    void onDisplayModeChanged() override;
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& window) override;

private:
    void buildUI();

    std::function<void()> m_onRestartCallback;
    
    view::ui::UILabel m_scoreLabel;
    view::ui::UIContainer m_buttonList;
    view::effect::LerpAnimator m_slideIn;
    sf::Sprite m_background;
    
    bool fontLoaded = false;
};

}

#endif
