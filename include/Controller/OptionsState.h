#ifndef CONTROLLER_OPTIONSSTATE_H
#define CONTROLLER_OPTIONSSTATE_H

#include "Controller/GameState.h"
#include "Model/Settings.h"
#include "View/UI/UIContainer.h"
#include "View/UI/UIScrollView.h"
#include "View/UI/UILabel.h"
#include "View/UI/UIButton.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Clock.hpp>
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

    void updateKeyButtonsVisuals();
    bool hasKeyConflicts() const;
    void resetSingleKey(int index);

    void buildGraphicsTab(const sf::Font& font);
    void buildSoundTab(const sf::Font& font);
    void buildControlsTab(const sf::Font& font);
    void buildLanguageTab(const sf::Font& font);

    // Layout helper: adds a labelled row (Label on the left, widget on the right)
    // to a scroll-panel and advances the cursor for the next row.
    void addRow(view::ui::UIContainer& parent, const sf::Font& font,
                const std::string& label, std::unique_ptr<view::ui::UIElement> widget,
                float& cursorY);

    model::Settings draft;

    sf::RectangleShape background;

    view::ui::UILabel     titleLabel;
    view::ui::UIContainer tabBar;
    
    std::array<view::ui::UIScrollView, 4> tabPanels;
    int currentTab = 0;

    view::ui::UIContainer bottomBar;

    sf::Clock soundThrottleClock;

    std::array<view::ui::UIButton*, 10> keyButtons = {nullptr};
    int waitingForKeyIndex = -1;
    int pendingPreviousKey = -1;

    struct UIContext {
        view::ui::UIButton* applyBtn = nullptr;
        view::ui::UIButton* doneBtn = nullptr;
        bool forceApply = false;
        bool forceDone = false;
    } uiCtx;
};

} // namespace controller

#endif
