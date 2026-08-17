#ifndef CONTROLLER_PAUSESTATE_H
#define CONTROLLER_PAUSESTATE_H

#include "Controller/GameState.h"
#include "View/UI/UIContainer.h"
#include "View/UI/UILabel.h"
#include "View/UI/UIButton.h"
#include <SFML/Graphics/RectangleShape.hpp>

namespace controller {

class PauseState : public GameState {
public:
    PauseState();

    void update(float dt) override;
    void render(sf::RenderTarget& target) override;
    void handleEvent(const sf::Event& event) override;
    void onDisplayModeChanged() override;
    void onResume() override;

    bool isTransparent() const override { return true; }

private:
    sf::RectangleShape overlay;
    view::ui::UILabel titleLabel;
    view::ui::UIContainer menuContainer;
};

} // namespace controller

#endif
