#include "Controller/WarningPopupState.h"
#include "Controller/AppEngine.h"
#include "View/AssetManager.h"
#include "View/UI/UITheme.h"
#include <SFML/Graphics/RenderTarget.hpp>

namespace controller {

WarningPopupState::WarningPopupState(const std::string& msg, Type type,
                                       std::function<void()> onPrimary,
                                       std::function<void()> onSecondary,
                                       const std::string& primaryLabel,
                                       const std::string& secondaryLabel)
    : message(msg), popupType(type), onPrimaryCallback(onPrimary), onSecondaryCallback(onSecondary),
      buttonContainer(view::ui::UIContainer::Layout::None) {
    
    // Dim the background
    overlay.setFillColor(view::ui::theme::ColorOverlay);

    const sf::Font& font = view::AssetManager::instance().getUiFont();

    titleLabel = view::ui::UILabel(font, message, 10);
    titleLabel.setLineSpacing(1.5f);

    primaryBtnText = primaryLabel.empty() ? (popupType == Type::YesNo ? "YES" : "OK") : primaryLabel;
    secondaryBtnText = secondaryLabel.empty() ? "NO" : secondaryLabel;

    if (primaryBtnText.length() > 4 || secondaryBtnText.length() > 4) {
        btnWidth = 75.f;
    } else {
        btnWidth = 60.f;
    }
    
    if (popupType == Type::YesNo) {
        auto btnYes = std::make_unique<view::ui::UIButton>(font, primaryBtnText, 8, sf::Vector2f(0.f, 0.f), sf::Vector2f(btnWidth, 20.f));
        btnYes->setOnClick([this]() {
            if (onPrimaryCallback) onPrimaryCallback();
        });
        
        auto btnNo = std::make_unique<view::ui::UIButton>(font, secondaryBtnText, 8, sf::Vector2f(btnWidth + kBtnGap, 0.f), sf::Vector2f(btnWidth, 20.f));
        btnNo->setOnClick([this]() {
            if (onSecondaryCallback) onSecondaryCallback();
        });
        
        buttonContainer.add(std::move(btnYes));
        buttonContainer.add(std::move(btnNo));
    } else if (popupType == Type::OkOnly) {
        auto btnOk = std::make_unique<view::ui::UIButton>(font, primaryBtnText, 8, sf::Vector2f(0.f, 0.f), sf::Vector2f(btnWidth, 20.f));
        btnOk->setOnClick([this]() {
            if (onPrimaryCallback) onPrimaryCallback();
        });
        
        buttonContainer.add(std::move(btnOk));
    }

    onDisplayModeChanged();
}

void WarningPopupState::onDisplayModeChanged() {
    float screenW = static_cast<float>(AppEngine::screenWidth());
    float screenH = static_cast<float>(AppEngine::ScreenHeight);
    relayout(screenW, screenH);
}

void WarningPopupState::relayout(float screenW, float screenH) {
    overlay.setSize({screenW, screenH});
    
    // Force UILabel to span the full width, so its internal centering handles the text.
    titleLabel.setSize(screenW, 20.f);
    titleLabel.setPosition(0.f, (screenH / 2.f) + kLabelOffsetY);
    
    // Center button container — width depends on popup type.
    float containerW = (popupType == Type::YesNo) ? (btnWidth * 2.f + kBtnGap) : btnWidth;
    buttonContainer.setPosition((screenW - containerW) / 2.f, (screenH / 2.f) + kBtnOffsetY);
}

void WarningPopupState::update(float dt) {
    buttonContainer.update(dt);
}

void WarningPopupState::handleEvent(const sf::Event& event) {
    buttonContainer.handleEvent(event);
}

void WarningPopupState::render(sf::RenderTarget& target) {
    target.draw(overlay);
    titleLabel.render(target);
    buttonContainer.render(target);
}

} // namespace controller
