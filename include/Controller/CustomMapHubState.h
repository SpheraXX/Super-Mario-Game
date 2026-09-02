#ifndef CONTROLLER_CUSTOMMAPHUBSTATE_H
#define CONTROLLER_CUSTOMMAPHUBSTATE_H

#include "Controller/GameState.h"
#include "View/UI/UIContainer.h"
#include <SFML/Graphics/Sprite.hpp>

namespace controller {

// Landing screen for the "CUSTOM MAP" main menu entry: choose between creating a new
// map or playing one already saved under assets/maps/custom/.
class CustomMapHubState : public GameState {
public:
    CustomMapHubState();

    void onEnter() override;
    void onDisplayModeChanged() override;
    void onResume() override { onDisplayModeChanged(); }

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
    void buildUI();

    sf::Sprite m_background;
    view::ui::UIContainer m_buttons;
};

} // namespace controller

#endif // CONTROLLER_CUSTOMMAPHUBSTATE_H
