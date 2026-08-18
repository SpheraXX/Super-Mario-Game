#ifndef CONTROLLER_WARNINGPOPUPSTATE_H
#define CONTROLLER_WARNINGPOPUPSTATE_H

#include "Controller/GameState.h"
#include "View/UI/UIButton.h"
#include "View/UI/UILabel.h"
#include "View/UI/UIContainer.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <functional>
#include <string>

namespace controller {

class WarningPopupState : public GameState {
public:
    enum class Type {
        YesNo,
        OkOnly
    };

    WarningPopupState(const std::string& message, Type type, std::function<void()> onPrimary, std::function<void()> onSecondary = nullptr);

    void update(float dt) override;
    void render(sf::RenderTarget& target) override;
    void handleEvent(const sf::Event& event) override;
    void onDisplayModeChanged() override;
    void onResume() override { onDisplayModeChanged(); }
    
    bool isTransparent() const override { return true; }

private:
    void relayout(float screenW, float screenH);
    
    std::string message;
    Type popupType;
    std::function<void()> onPrimaryCallback;
    std::function<void()> onSecondaryCallback;

    sf::RectangleShape overlay;
    view::ui::UILabel titleLabel;
    view::ui::UIContainer buttonContainer;
    
    // Layout constants
    static constexpr float kLabelOffsetY  = -35.f;
    static constexpr float kBtnOffsetY    = 15.f;
    static constexpr float kBtnWidth      = 60.f;
    static constexpr float kBtnGap        = 10.f;
    static constexpr float kYesNoBtnContainerW = kBtnWidth * 2.f + kBtnGap; // 130.f
};

} // namespace controller

#endif
