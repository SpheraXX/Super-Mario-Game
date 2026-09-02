#include "Controller/ProfileInputPopupState.h"
#include "Controller/AppEngine.h"
#include "Controller/StateManager.h"
#include "View/AssetManager.h"
#include "View/UI/UITheme.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <cctype>

namespace controller {

ProfileInputPopupState::ProfileInputPopupState(const std::string& initialName, std::function<void(const std::string&)> onConfirm)
    : currentText(initialName), onConfirmCallback(std::move(onConfirm)) {
    
    if (currentText == "EMPTY") {
        currentText = "";
    }
}

void ProfileInputPopupState::onEnter() {
    onDisplayModeChanged();
}

void ProfileInputPopupState::onDisplayModeChanged() {
    const float W = static_cast<float>(AppEngine::screenWidth());
    const float H = static_cast<float>(AppEngine::ScreenHeight);
    relayout(W, H);
}

void ProfileInputPopupState::relayout(float W, float H) {
    overlay.setSize({W, H});
    overlay.setFillColor(view::ui::theme::PopupBackground);

    const sf::Font& font = view::AssetManager::instance().getUiFont();
    
    titleLabel = view::ui::UILabel(font, "ENTER PROFILE NAME", view::ui::layout::TitleFontSize);
    titleLabel.setColor(sf::Color::Yellow);
    titleLabel.setSize(W, 0.f);
    titleLabel.setPosition(0.f, H / 2.f + view::ui::layout::ProfilePopupTitleOffsetY);

    inputBox.setSize({view::ui::layout::ProfilePopupInputWidth, view::ui::layout::ProfilePopupInputHeight});
    inputBox.setOrigin({view::ui::layout::ProfilePopupInputWidth / 2.f, view::ui::layout::ProfilePopupInputHeight / 2.f});
    inputBox.setPosition({W / 2.f, H / 2.f + view::ui::layout::ProfilePopupInputOffsetY});
    inputBox.setFillColor(sf::Color::White);
    inputBox.setOutlineColor(sf::Color::Black);
    inputBox.setOutlineThickness(1.f);

    inputLabel = view::ui::UILabel(font, "", view::ui::layout::ButtonFontSize);
    inputLabel.setColor(sf::Color::Black);
    inputLabel.setSize(W, 0.f);
    updateInputLabel();

    buttonContainer = view::ui::UIContainer(view::ui::UIContainer::Layout::None, 0.f);
    
    float btnW = view::ui::layout::PopupBtnWidthShort;
    float btnH = view::ui::layout::SmallButtonHeight;
    float gap = view::ui::layout::ProfilePopupBtnGap;

    auto confirmBtn = std::make_unique<view::ui::UIButton>(
        font, "CONFIRM", view::ui::layout::ButtonFontSize, 
        sf::Vector2f{0.f, 0.f}, sf::Vector2f{btnW, btnH});
    confirmBtn->setOnClick([this]() {
        if (onConfirmCallback) onConfirmCallback(currentText);
        if (manager) manager->popState();
    });
    
    auto cancelBtn = std::make_unique<view::ui::UIButton>(
        font, "CANCEL", view::ui::layout::ButtonFontSize, 
        sf::Vector2f{btnW + gap, 0.f}, sf::Vector2f{btnW, btnH});
    cancelBtn->setOnClick([this]() {
        if (manager) manager->popState();
    });

    buttonContainer.add(std::move(confirmBtn));
    buttonContainer.add(std::move(cancelBtn));
    
    float containerW = btnW * 2.f + gap;
    buttonContainer.setPosition(W / 2.f - containerW / 2.f, H / 2.f + view::ui::layout::ProfilePopupBtnOffsetY);
    buttonContainer.relayout();
}

void ProfileInputPopupState::updateInputLabel() {
    std::string display = currentText;
    if (showCursor) {
        display += "|";
    }
    inputLabel.setText(display);
    
    const float W = static_cast<float>(AppEngine::screenWidth());
    const float H = static_cast<float>(AppEngine::ScreenHeight);
    inputLabel.setPosition(0.f, H / 2.f + view::ui::layout::ProfilePopupInputOffsetY);
}

void ProfileInputPopupState::update(float dt) {
    buttonContainer.update(dt);
    
    blinkTimer += dt;
    if (blinkTimer >= view::ui::layout::ProfilePopupCursorBlinkTime) {
        blinkTimer = 0.f;
        showCursor = !showCursor;
        updateInputLabel();
    }
}

void ProfileInputPopupState::handleEvent(const sf::Event& event) {
    if (buttonContainer.handleEvent(event)) return;

    if (const auto* textEv = event.getIf<sf::Event::TextEntered>()) {
        char32_t uc = textEv->unicode;
        if (uc < 128) {
            char c = static_cast<char>(uc);
            if ((std::isalnum(c) || c == ' ') && currentText.size() < 8) {
                currentText += std::toupper(c);
                showCursor = true;
                blinkTimer = 0.f;
                updateInputLabel();
            }
        }
    } else if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Backspace && !currentText.empty()) {
            currentText.pop_back();
            showCursor = true;
            blinkTimer = 0.f;
            updateInputLabel();
        } else if (key->code == sf::Keyboard::Key::Enter) {
            if (onConfirmCallback) onConfirmCallback(currentText);
            if (manager) manager->popState();
        } else if (key->code == sf::Keyboard::Key::Escape) {
            if (manager) manager->popState();
        }
    }
}

void ProfileInputPopupState::render(sf::RenderTarget& target) {
    target.draw(overlay);
    titleLabel.render(target);
    target.draw(inputBox);
    inputLabel.render(target);
    buttonContainer.render(target);
}

} // namespace controller
