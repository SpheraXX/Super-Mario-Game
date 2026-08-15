#ifndef CONTROLLER_OPTIONSSTATE_H
#define CONTROLLER_OPTIONSSTATE_H

#include "Controller/GameState.h"
#include "Model/Settings.h"
#include "View/UI/UIContainer.h"
#include "View/UI/UILabel.h"
#include "View/UI/UIButton.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <array>

namespace controller {

class OptionsState : public GameState {
public:
    OptionsState();

    void update(float dt) override;
    void render(sf::RenderTarget& target) override;
    void handleEvent(const sf::Event& event) override;

private:
    void switchTab(int index);
    void applySettings();
    void resetSettings();
    void updateUIFromDraft();

    void buildGraphicsTab(const sf::Font& font);
    void buildSoundTab(const sf::Font& font);
    void buildControlsTab(const sf::Font& font);
    void buildLanguageTab(const sf::Font& font);

    model::Settings draft;

    sf::RectangleShape background;

    view::ui::UILabel     titleLabel;
    view::ui::UIContainer tabBar;
    
    std::array<view::ui::UIContainer, 4> tabPanels;
    int currentTab = 0;

    view::ui::UIContainer bottomBar;
};

} // namespace controller

#endif
