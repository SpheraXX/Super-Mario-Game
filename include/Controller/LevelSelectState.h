#ifndef CONTROLLER_LEVELSELECTSTATE_H
#define CONTROLLER_LEVELSELECTSTATE_H

#include "Controller/GameState.h"
#include "View/UI/UIContainer.h"
#include "View/UI/UIButton.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <string>

namespace controller {

class LevelSelectState : public GameState {
public:
    explicit LevelSelectState(std::string worldId);
    ~LevelSelectState() override = default;

    void onEnter() override;
    void onDisplayModeChanged() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;

private:
    void buildUI();

    std::string m_worldId;
    sf::Sprite m_background;
    view::ui::UIContainer m_grid;
    view::ui::UIButton m_backButton;
};

} // namespace controller

#endif // CONTROLLER_LEVELSELECTSTATE_H
