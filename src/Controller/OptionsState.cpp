#include "Controller/OptionsState.h"
#include "Controller/AppEngine.h"
#include "Controller/StateManager.h"
#include "Model/SettingsManager.h"
#include "View/AssetManager.h"
#include "View/UI/UISlider.h"
#include "View/UI/UICycleButton.h"

namespace controller {

namespace {
auto addRow = [](view::ui::UIContainer& parent, const sf::Font& f, const std::string& label, std::unique_ptr<view::ui::UIElement> widget, float& cursorY) {
    auto lbl = std::make_unique<view::ui::UILabel>(f, label, 8);
    lbl->setPosition(20.f, cursorY + 4.f);

    widget->setPosition(160.f, cursorY); 

    parent.add(std::move(lbl));
    parent.add(std::move(widget));
    
    cursorY += 25.f;
};
}

OptionsState::OptionsState() {
    draft = model::SettingsManager::instance().get();

    background.setSize({static_cast<float>(AppEngine::screenWidth()),
                        static_cast<float>(AppEngine::ScreenHeight)});
    background.setFillColor(sf::Color(20, 20, 30));

    const sf::Font& font = view::AssetManager::instance().getUiFont();

    // Title
    titleLabel = view::ui::UILabel(font, "OPTIONS", 16);
    titleLabel.setPosition(10.f, 10.f);

    // Tab Bar Setup (Horizontal via manual layout)
    tabBar = view::ui::UIContainer(view::ui::UIContainer::Layout::None);
    tabBar.setPosition(10.f, 40.f);

    std::vector<std::string> tabNames = {"GRAPHICS", "SOUND", "CONTROLS", "LANGUAGE"};
    float currentX = 10.f;
    for (int i = 0; i < 4; ++i) {
        auto btn = std::make_unique<view::ui::UIButton>(
            font, tabNames[i], 8,
            sf::Vector2f(currentX, 40.f), sf::Vector2f(80.f, 20.f)
        );
        btn->setOnClick([this, i]() { switchTab(i); });
        tabBar.add(std::move(btn));
        currentX += 85.f;
        
        tabPanels[i] = view::ui::UIContainer(view::ui::UIContainer::Layout::None);
        tabPanels[i].setPosition(10.f, 70.f);
        tabPanels[i].setVisible(false);
    }
    tabPanels[0].setVisible(true);

    buildGraphicsTab(font);
    buildSoundTab(font);
    buildControlsTab(font);
    buildLanguageTab(font);

    // Bottom Bar Setup (Horizontal via manual layout)
    bottomBar = view::ui::UIContainer(view::ui::UIContainer::Layout::None);
    bottomBar.setPosition(0.f, 0.f);

    float screenW = static_cast<float>(AppEngine::screenWidth());
    float screenH = static_cast<float>(AppEngine::ScreenHeight);
    float btnY = screenH - 30.f;

    auto btnApply = std::make_unique<view::ui::UIButton>(
        font, "APPLY", 8, sf::Vector2f(screenW - 280.f, btnY), sf::Vector2f(80.f, 20.f));
    btnApply->setOnClick([this]() { applySettings(); });
    
    auto btnReset = std::make_unique<view::ui::UIButton>(
        font, "RESET", 8, sf::Vector2f(screenW - 190.f, btnY), sf::Vector2f(80.f, 20.f));
    btnReset->setOnClick([this]() { resetSettings(); });

    auto btnDone = std::make_unique<view::ui::UIButton>(
        font, "DONE", 8, sf::Vector2f(screenW - 100.f, btnY), sf::Vector2f(80.f, 20.f));
    btnDone->setOnClick([this]() { if (manager) manager->popState(); });

    bottomBar.add(std::move(btnApply));
    bottomBar.add(std::move(btnReset));
    bottomBar.add(std::move(btnDone));
}

void OptionsState::buildGraphicsTab(const sf::Font& font) {
    float cursorY = 80.f;
    auto fsBtn = std::make_unique<view::ui::UICycleButton>(
        font, "", std::vector<std::string>{"Off", "On"}, draft.fullscreen ? 1 : 0,
        sf::Vector2f(0.f,0.f), sf::Vector2f(100.f, 20.f));
    fsBtn->setOnChange([this](int idx) { draft.fullscreen = (idx == 1); });
    addRow(tabPanels[0], font, "Fullscreen", std::move(fsBtn), cursorY);

    std::vector<std::string> resOpts = {"384", "448", "512"};
    int resIdx = 0;
    if (draft.logicalWidth == 448) resIdx = 1;
    if (draft.logicalWidth == 512) resIdx = 2;
    auto resBtn = std::make_unique<view::ui::UICycleButton>(
        font, "", resOpts, resIdx,
        sf::Vector2f(0.f,0.f), sf::Vector2f(100.f, 20.f));
    resBtn->setOnChange([this](int idx) { 
        if (idx == 0) draft.logicalWidth = 384;
        else if (idx == 1) draft.logicalWidth = 448;
        else draft.logicalWidth = 512;
    });
    addRow(tabPanels[0], font, "Logical Width", std::move(resBtn), cursorY);

    std::vector<std::string> qOpts = {"Low", "Medium", "High"};
    auto qBtn = std::make_unique<view::ui::UICycleButton>(
        font, "", qOpts, static_cast<int>(draft.quality),
        sf::Vector2f(0.f,0.f), sf::Vector2f(100.f, 20.f));
    qBtn->setOnChange([this](int idx) { draft.quality = static_cast<model::GraphicsQuality>(idx); });
    addRow(tabPanels[0], font, "Quality", std::move(qBtn), cursorY);
}

void OptionsState::buildSoundTab(const sf::Font& font) {
    float cursorY = 80.f;
    auto master = std::make_unique<view::ui::UISlider>(
        font, "", 0, 100, 5, draft.masterVolume, sf::Vector2f(0,0), sf::Vector2f(150.f, 20.f));
    master->setOnChange([this](int v) { draft.masterVolume = v; });
    addRow(tabPanels[1], font, "Master Volume", std::move(master), cursorY);

    auto music = std::make_unique<view::ui::UISlider>(
        font, "", 0, 100, 5, draft.musicVolume, sf::Vector2f(0,0), sf::Vector2f(150.f, 20.f));
    music->setOnChange([this](int v) { draft.musicVolume = v; });
    addRow(tabPanels[1], font, "Music Volume", std::move(music), cursorY);

    auto sfx = std::make_unique<view::ui::UISlider>(
        font, "", 0, 100, 5, draft.sfxVolume, sf::Vector2f(0,0), sf::Vector2f(150.f, 20.f));
    sfx->setOnChange([this](int v) { draft.sfxVolume = v; });
    addRow(tabPanels[1], font, "SFX Volume", std::move(sfx), cursorY);
}

void OptionsState::buildControlsTab(const sf::Font& font) {
    float cursorY = 80.f;
    // Just mock entries for now, Key binding is complex.
    std::vector<std::string> mock = {"Press Any Key"};
    auto mLeft = std::make_unique<view::ui::UICycleButton>(font, "", mock, 0, sf::Vector2f(0,0), sf::Vector2f(120.f, 20.f));
    addRow(tabPanels[2], font, "Move Left", std::move(mLeft), cursorY);
    
    auto mRight = std::make_unique<view::ui::UICycleButton>(font, "", mock, 0, sf::Vector2f(0,0), sf::Vector2f(120.f, 20.f));
    addRow(tabPanels[2], font, "Move Right", std::move(mRight), cursorY);

    auto mJump = std::make_unique<view::ui::UICycleButton>(font, "", mock, 0, sf::Vector2f(0,0), sf::Vector2f(120.f, 20.f));
    addRow(tabPanels[2], font, "Jump", std::move(mJump), cursorY);
}

void OptionsState::buildLanguageTab(const sf::Font& font) {
    float cursorY = 80.f;
    std::vector<std::string> opts = {"English", "Vietnamese"};
    auto lang = std::make_unique<view::ui::UICycleButton>(
        font, "", opts, static_cast<int>(draft.language), sf::Vector2f(0,0), sf::Vector2f(120.f, 20.f));
    lang->setOnChange([this](int idx) { draft.language = static_cast<model::Language>(idx); });
    addRow(tabPanels[3], font, "Language", std::move(lang), cursorY);
}

void OptionsState::updateUIFromDraft() {
    // Implemented via recreating tabs in resetSettings
}

void OptionsState::switchTab(int index) {
    if (index >= 0 && index < 4) {
        tabPanels[currentTab].setVisible(false);
        currentTab = index;
        tabPanels[currentTab].setVisible(true);
    }
}

void OptionsState::applySettings() {
    model::SettingsManager::instance().apply(draft);
}

void OptionsState::resetSettings() {
    draft = model::Settings::defaults();
    for (int i=0; i<4; ++i) {
        tabPanels[i].clear();
    }
    const sf::Font& font = view::AssetManager::instance().getUiFont();
    buildGraphicsTab(font);
    buildSoundTab(font);
    buildControlsTab(font);
    buildLanguageTab(font);
}

void OptionsState::update(float dt) {
}

void OptionsState::render(sf::RenderTarget& target) {
    target.draw(background);
    titleLabel.render(target);
    tabBar.render(target);
    tabPanels[currentTab].render(target);
    bottomBar.render(target);
}

void OptionsState::handleEvent(const sf::Event& event) {
    if (bottomBar.handleEvent(event)) return;
    if (tabBar.handleEvent(event)) return;
    if (tabPanels[currentTab].handleEvent(event)) return;
}

} // namespace controller
