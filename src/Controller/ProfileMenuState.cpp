#include "Controller/ProfileMenuState.h"
#include "Controller/AppEngine.h"
#include "Controller/StateManager.h"
#include "Controller/MainMenuState.h"
#include "Controller/RankingState.h"
#include "Controller/ProfileInputPopupState.h"
#include "Controller/WarningPopupState.h"
#include "Model/Save/ProfileManager.h"
#include "View/AssetManager.h"
#include "View/UI/UIButton.h"
#include "View/UI/UITheme.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

namespace controller {

ProfileMenuState::ProfileMenuState()
    : bgaSprite(view::AssetManager::instance().getTexture("assets/images/bga_mainmenu.png")),
      titleSprite(view::AssetManager::instance().getTexture("assets/images/bga_mainmenu_title.png")) {
}

void ProfileMenuState::onEnter() {
    buildUI();
}

void ProfileMenuState::buildUI() {
    const sf::Font& font = view::AssetManager::instance().getUiFont();
    const float W = static_cast<float>(AppEngine::screenWidth());
    const float H = static_cast<float>(AppEngine::ScreenHeight);

    bgaSprite.setColor(view::ui::theme::BgaDimMenu);

    titleSprite.setOrigin({static_cast<float>(titleSprite.getTexture().getSize().x) / 2.f,
                           static_cast<float>(titleSprite.getTexture().getSize().y) / 2.f});

    menuList = view::ui::UIContainer(view::ui::UIContainer::Layout::Vertical, view::ui::layout::MenuButtonGap);
    const float listX = (W - view::ui::layout::MenuButtonWidth) / 2.f;

    float targetContainerY = H * 0.38f;
    m_menuSlideIn = view::effect::LerpAnimator(H, targetContainerY, 0.6f, view::effect::Easing::OutQuad);
    menuList.setPosition(listX, m_menuSlideIn.value());
    menuList.setSize(view::ui::layout::MenuButtonWidth, 0.f);

    auto makeBtn = [&](const std::string& label, std::function<void()> cmd, bool isActive) {
        auto btn = std::make_unique<view::ui::UIButton>(
            font, label, view::ui::layout::ButtonFontSize, sf::Vector2f{listX, 0.f},
            sf::Vector2f{view::ui::layout::MenuButtonWidth, view::ui::layout::MenuButtonHeight});
        
        if (isActive) {
            btn->setColors(sf::Color::White, sf::Color::White, view::ui::theme::ColorWarningNormal);
        }

        btn->setOnClick(std::move(cmd));
        menuList.add(std::move(btn));
    };

    // Button 1: RANKING
    makeBtn("RANKING", [this]() {
        manager->pushState(std::make_unique<RankingState>());
    }, false);

    // Load profiles from ProfileManager
    model::ProfileManager::instance().load();
    const auto& profiles = model::ProfileManager::instance().getProfiles();
    int activeIdx = model::ProfileManager::instance().getActiveProfileIndex();

    // Buttons 2-5: Profile Slots
    for (int i = 0; i < 4; ++i) {
        bool isActive = (!profiles[i].is_empty && i == activeIdx);
        std::string label = profiles[i].is_empty ? "NEW PROFILE" : profiles[i].name;
        makeBtn(label, [this, i, is_empty = profiles[i].is_empty, name = profiles[i].name, isActive]() {
            if (is_empty) {
                manager->pushState(std::make_unique<ProfileInputPopupState>("EMPTY", [i](const std::string& newName) {
                    model::Profile p;
                    p.name = newName;
                    p.is_empty = false;
                    p.total_score = 0;
                    p.passed_levels = 0;
                    model::ProfileManager::instance().updateProfile(i, p);
                    model::ProfileManager::instance().setActiveProfileIndex(i);
                }));
            } else {
                if (isActive) {
                    manager->pushState(std::make_unique<WarningPopupState>(
                        "Manage Profile: " + name, WarningPopupState::Type::YesNo,
                        [this, i, name]() { // RENAME
                            manager->popState(); // pop WarningPopupState
                            manager->pushState(std::make_unique<ProfileInputPopupState>(name, [i](const std::string& newName) {
                                model::Profile p = model::ProfileManager::instance().getProfiles()[i];
                                p.name = newName;
                                model::ProfileManager::instance().updateProfile(i, p);
                            }));
                        },
                        [this, i]() { // DELETE
                            model::ProfileManager::instance().deleteProfile(i);
                            manager->popState(); // pop WarningPopupState
                        },
                        "RENAME", "DELETE"));
                } else {
                    model::ProfileManager::instance().setActiveProfileIndex(i);
                    menuList.clear();
                    buildUI();
                }
            }
        }, isActive);
    }

    // Add a BACK button just so they can return to MainMenuState
    makeBtn("BACK", [this]() {
        if (manager) manager->popState();
    }, false);

    onDisplayModeChanged();
}

void ProfileMenuState::onDisplayModeChanged() {
    const float W = static_cast<float>(AppEngine::screenWidth());
    const float H = static_cast<float>(AppEngine::ScreenHeight);

    const sf::Texture& tex = bgaSprite.getTexture();
    float scaleX = W / static_cast<float>(tex.getSize().x);
    float scaleY = H / static_cast<float>(tex.getSize().y);
    float scale = std::max(scaleX, scaleY) * view::ui::layout::BgaScaleMultiplier;
    bgaSprite.setScale({scale, scale});
    bgaSprite.setOrigin({static_cast<float>(tex.getSize().x) / 2.f,
                         static_cast<float>(tex.getSize().y) / 2.f});
    bgaSprite.setPosition({W / 2.f, H / 2.f});

    baseTitleY = H * 0.18f;

    float titleScale = (W * view::ui::layout::TitleWidthRatio) / static_cast<float>(titleSprite.getTexture().getSize().x);
    titleSprite.setScale({titleScale, titleScale});
    titleSprite.setPosition({W / 2.f, baseTitleY});

    const float listX = (W - view::ui::layout::MenuButtonWidth) / 2.f;
    menuList.setPosition(listX, m_menuSlideIn.value());
    menuList.relayout();
}

void ProfileMenuState::onResume() {
    // Re-build UI to reflect any changes in profiles
    menuList.clear();
    buildUI();
}

void ProfileMenuState::handleEvent(const sf::Event& event) {
    menuList.handleEvent(event);
    
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Escape) {
            if (manager) manager->popState();
        }
    }
}

void ProfileMenuState::update(float deltaTime) {
    m_menuSlideIn.update(deltaTime);
    menuList.setPosition(menuList.getPosition().x, m_menuSlideIn.value());
    menuList.update(deltaTime);
}

void ProfileMenuState::render(sf::RenderTarget& target) {
    target.draw(bgaSprite);
    target.draw(titleSprite);
    menuList.render(target);
}

} // namespace controller
