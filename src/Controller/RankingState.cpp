#include "Controller/RankingState.h"
#include "Controller/AppEngine.h"
#include "Controller/StateManager.h"
#include "Model/Save/ProfileManager.h"
#include "Model/Core/WorldManager.h"
#include "View/AssetManager.h"
#include "View/UI/UIButton.h"
#include "View/UI/UITheme.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <memory>

namespace controller {

RankingState::RankingState()
    : bgaSprite(view::AssetManager::instance().getTexture("assets/images/bga_mainmenu.png")),
      titleSprite(view::AssetManager::instance().getTexture("assets/images/bga_mainmenu_title.png")) {
    overlay.setFillColor(view::ui::theme::ColorOverlay);
}

void RankingState::onEnter() {
    // Count total levels in the game
    m_totalLevels = 0;
    const auto& worlds = model::WorldManager::instance().getWorlds();
    for (const auto& w : worlds) {
        m_totalLevels += w.levels.size();
    }

    // Load profiles and sort them
    model::ProfileManager::instance().load();
    const auto& profiles = model::ProfileManager::instance().getProfiles();
    
    rankedProfiles.clear();
    for (const auto& p : profiles) {
        RankEntry entry;
        entry.name = p.is_empty ? "EMPTY" : p.name;
        entry.score = p.total_score;
        entry.passed = p.passed_levels;
        entry.isEmpty = p.is_empty;
        rankedProfiles.push_back(entry);
    }

    std::sort(rankedProfiles.begin(), rankedProfiles.end(), [](const RankEntry& a, const RankEntry& b) {
        if (a.score != b.score) return a.score > b.score;
        if (a.passed != b.passed) return a.passed > b.passed;
        char charA = a.name.empty() ? '\0' : std::toupper(a.name[0]);
        char charB = b.name.empty() ? '\0' : std::toupper(b.name[0]);
        return charA < charB;
    });

    buildUI();
}

void RankingState::buildUI() {
    const sf::Font& font = view::AssetManager::instance().getUiFont();
    const float W = static_cast<float>(AppEngine::screenWidth());
    
    bgaSprite.setColor(view::ui::theme::BgaDimMenu);
    titleSprite.setOrigin({static_cast<float>(titleSprite.getTexture().getSize().x) / 2.f,
                           static_cast<float>(titleSprite.getTexture().getSize().y) / 2.f});

    // Create Back Button
    const float btnW = view::ui::layout::MenuButtonWidth;
    const float btnH = view::ui::layout::MenuButtonHeight;
    backBtn = std::make_unique<view::ui::UIButton>(
        font, "BACK", view::ui::layout::ButtonFontSize, 
        sf::Vector2f{0.f, 0.f}, // Set in onDisplayModeChanged
        sf::Vector2f{btnW, btnH});
    backBtn->setOnClick([this]() {
        if (manager) manager->popState();
    });

    // Build Table Headers
    headerTexts.clear();
    auto makeHeader = [&](const std::string& str) {
        sf::Text t(font, str, view::ui::layout::ButtonFontSize);
        t.setFillColor(sf::Color::Yellow);
        headerTexts.push_back(t);
    };
    makeHeader("No");
    makeHeader("PLAYER");
    makeHeader("SCORE");
    makeHeader("PASS");

    // Build Table Rows
    rowTexts.clear();
    for (size_t i = 0; i < rankedProfiles.size(); ++i) {
        const auto& rp = rankedProfiles[i];
        
        // No
        sf::Text tNo(font, std::to_string(i + 1), view::ui::layout::ButtonFontSize);
        tNo.setFillColor(sf::Color::White);
        rowTexts.push_back(tNo);

        // Player
        sf::Text tPlayer(font, rp.name, view::ui::layout::ButtonFontSize);
        tPlayer.setFillColor(rp.isEmpty ? sf::Color(150, 150, 150) : sf::Color::White);
        rowTexts.push_back(tPlayer);

        // Score
        sf::Text tScore(font, std::to_string(rp.score), view::ui::layout::ButtonFontSize);
        tScore.setFillColor(sf::Color::White);
        rowTexts.push_back(tScore);

        // Pass
        std::string passStr = std::to_string(rp.passed) + "/" + std::to_string(static_cast<int>(m_totalLevels));
        sf::Text tPass(font, passStr, view::ui::layout::ButtonFontSize);
        tPass.setFillColor(sf::Color::White);
        rowTexts.push_back(tPass);
    }

    onDisplayModeChanged();
}

void RankingState::onDisplayModeChanged() {
    const float W = static_cast<float>(AppEngine::screenWidth());
    const float H = static_cast<float>(AppEngine::ScreenHeight);

    overlay.setSize({W, H});

    const sf::Texture& tex = bgaSprite.getTexture();
    float scaleX = W / static_cast<float>(tex.getSize().x);
    float scaleY = H / static_cast<float>(tex.getSize().y);
    float scale = std::max(scaleX, scaleY) * view::ui::layout::BgaScaleMultiplier;
    bgaSprite.setScale({scale, scale});
    bgaSprite.setOrigin({static_cast<float>(tex.getSize().x) / 2.f,
                         static_cast<float>(tex.getSize().y) / 2.f});
    bgaSprite.setPosition({W / 2.f, H / 2.f});

    float titleScale = (W * view::ui::layout::TitleWidthRatio) / static_cast<float>(titleSprite.getTexture().getSize().x);
    titleSprite.setScale({titleScale, titleScale});
    titleSprite.setPosition({W / 2.f, H * view::ui::layout::TitleYRatio}); // Use TitleYRatio (0.18f)

    // Position table
    const float startY = H * view::ui::layout::RankingStartYRatio;
    const float rowSpacing = view::ui::layout::RankingRowSpacing;
    
    // Column X positions
    const float colNoX = W * view::ui::layout::RankingColNoXRatio;
    const float colPlayerX = W * view::ui::layout::RankingColPlayerXRatio;
    const float colScoreX = W * view::ui::layout::RankingColScoreXRatio;
    const float colPassX = W * view::ui::layout::RankingColPassXRatio;

    auto centerText = [](sf::Text& t, float x, float y) {
        sf::FloatRect bounds = t.getLocalBounds();
        t.setOrigin(sf::Vector2f{std::floor(bounds.position.x + bounds.size.x / 2.f), std::floor(bounds.position.y)});
        t.setPosition(sf::Vector2f{std::floor(x), std::floor(y)});
    };

    // Headers
    if (headerTexts.size() == 4) {
        centerText(headerTexts[0], colNoX, startY);
        centerText(headerTexts[1], colPlayerX, startY);
        centerText(headerTexts[2], colScoreX, startY);
        centerText(headerTexts[3], colPassX, startY);
    }

    // Rows
    for (size_t i = 0; i < rankedProfiles.size(); ++i) {
        float y = startY + (i + 1) * rowSpacing;
        size_t baseIdx = i * 4;
        if (baseIdx + 3 < rowTexts.size()) {
            centerText(rowTexts[baseIdx + 0], colNoX, y);
            centerText(rowTexts[baseIdx + 1], colPlayerX, y);
            centerText(rowTexts[baseIdx + 2], colScoreX, y);
            centerText(rowTexts[baseIdx + 3], colPassX, y);
        }
    }

    // Position Back button at bottom left
    if (backBtn) {
        float btnH = view::ui::layout::MenuButtonHeight;
        float gap  = view::ui::layout::MenuButtonGap;
        backBtn->setPosition(gap, H - btnH - gap);
    }
}

void RankingState::handleEvent(const sf::Event& event) {
    if (backBtn) {
        backBtn->handleEvent(event);
    }

    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Escape || key->code == sf::Keyboard::Key::Enter) {
            if (manager) manager->popState();
        }
    }
}

void RankingState::update(float deltaTime) {
    if (backBtn) {
        backBtn->update(deltaTime);
    }
}

void RankingState::render(sf::RenderTarget& target) {
    target.draw(bgaSprite);
    target.draw(overlay);
    target.draw(titleSprite);

    for (const auto& t : headerTexts) target.draw(t);
    for (const auto& t : rowTexts) target.draw(t);

    if (backBtn) {
        backBtn->render(target);
    }
}

} // namespace controller
