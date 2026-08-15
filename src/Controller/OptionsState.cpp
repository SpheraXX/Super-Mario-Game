#include "Controller/OptionsState.h"
#include "Controller/AppEngine.h"
#include "Controller/StateManager.h"
#include "Model/SettingsManager.h"
#include "View/AssetManager.h"
#include "Controller/IAudioManager.h"
#include "View/UI/UISlider.h"
#include "View/UI/UICycleButton.h"

namespace controller {

namespace {
int model::Settings::* const KeyFieldMap[10] = {
    &model::Settings::keyMoveLeft,
    &model::Settings::keyMoveRight,
    &model::Settings::keyJump,
    &model::Settings::keyRun,
    &model::Settings::keyPause,
    &model::Settings::keyDash,
    &model::Settings::keyAttack,
    &model::Settings::keyCrouch,
    &model::Settings::keyInteract,
    &model::Settings::keyInventory
};

std::string keyToString(int keyVal) {
    sf::Keyboard::Key key = static_cast<sf::Keyboard::Key>(keyVal);
    switch (key) {
        case sf::Keyboard::Key::A: return "A";
        case sf::Keyboard::Key::B: return "B";
        case sf::Keyboard::Key::C: return "C";
        case sf::Keyboard::Key::D: return "D";
        case sf::Keyboard::Key::E: return "E";
        case sf::Keyboard::Key::F: return "F";
        case sf::Keyboard::Key::G: return "G";
        case sf::Keyboard::Key::H: return "H";
        case sf::Keyboard::Key::I: return "I";
        case sf::Keyboard::Key::J: return "J";
        case sf::Keyboard::Key::K: return "K";
        case sf::Keyboard::Key::L: return "L";
        case sf::Keyboard::Key::M: return "M";
        case sf::Keyboard::Key::N: return "N";
        case sf::Keyboard::Key::O: return "O";
        case sf::Keyboard::Key::P: return "P";
        case sf::Keyboard::Key::Q: return "Q";
        case sf::Keyboard::Key::R: return "R";
        case sf::Keyboard::Key::S: return "S";
        case sf::Keyboard::Key::T: return "T";
        case sf::Keyboard::Key::U: return "U";
        case sf::Keyboard::Key::V: return "V";
        case sf::Keyboard::Key::W: return "W";
        case sf::Keyboard::Key::X: return "X";
        case sf::Keyboard::Key::Y: return "Y";
        case sf::Keyboard::Key::Z: return "Z";
        case sf::Keyboard::Key::Num0: return "0";
        case sf::Keyboard::Key::Num1: return "1";
        case sf::Keyboard::Key::Num2: return "2";
        case sf::Keyboard::Key::Num3: return "3";
        case sf::Keyboard::Key::Num4: return "4";
        case sf::Keyboard::Key::Num5: return "5";
        case sf::Keyboard::Key::Num6: return "6";
        case sf::Keyboard::Key::Num7: return "7";
        case sf::Keyboard::Key::Num8: return "8";
        case sf::Keyboard::Key::Num9: return "9";
        case sf::Keyboard::Key::Escape: return "Escape";
        case sf::Keyboard::Key::LControl: return "LControl";
        case sf::Keyboard::Key::LShift: return "LShift";
        case sf::Keyboard::Key::LAlt: return "LAlt";
        case sf::Keyboard::Key::RControl: return "RControl";
        case sf::Keyboard::Key::RShift: return "RShift";
        case sf::Keyboard::Key::RAlt: return "RAlt";
        case sf::Keyboard::Key::Space: return "Space";
        case sf::Keyboard::Key::Enter: return "Enter";
        case sf::Keyboard::Key::Backspace: return "Backspace";
        case sf::Keyboard::Key::Tab: return "Tab";
        case sf::Keyboard::Key::Up: return "Up";
        case sf::Keyboard::Key::Down: return "Down";
        case sf::Keyboard::Key::Left: return "Left";
        case sf::Keyboard::Key::Right: return "Right";
        default: return "Key " + std::to_string(static_cast<int>(key));
    }
}

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
    
    float screenW = static_cast<float>(AppEngine::screenWidth());
    float screenH = static_cast<float>(AppEngine::ScreenHeight);
    float btnY = screenH - 30.f;

    for (int i = 0; i < 4; ++i) {
        auto btn = std::make_unique<view::ui::UIButton>(
            font, tabNames[i], 8,
            sf::Vector2f(currentX, 40.f), sf::Vector2f(80.f, 20.f)
        );
        btn->setOnClick([this, i]() { switchTab(i); });
        tabBar.add(std::move(btn));
        currentX += 85.f;
        
        tabPanels[i] = view::ui::UIScrollView();
        tabPanels[i].setPosition(10.f, 70.f);
        tabPanels[i].setBounds(sf::FloatRect({10.f, 70.f}, {screenW - 20.f, btnY - 70.f}));
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

    auto btnApply = std::make_unique<view::ui::UIButton>(
        font, "APPLY", 8, sf::Vector2f(screenW - 280.f, btnY), sf::Vector2f(80.f, 20.f));
    uiCtx.applyBtn = btnApply.get();
    btnApply->setOnClick([this]() { 
        if (hasKeyConflicts() && !uiCtx.forceApply) {
            uiCtx.forceApply = true;
            uiCtx.applyBtn->setLabel("SURE?");
            return;
        }
        applySettings(); 
        uiCtx.forceApply = false;
        uiCtx.forceDone = false;
        uiCtx.applyBtn->setLabel("APPLY");
        if (uiCtx.doneBtn) uiCtx.doneBtn->setLabel("DONE");
    });
    
    auto btnReset = std::make_unique<view::ui::UIButton>(
        font, "RESET", 8, sf::Vector2f(screenW - 190.f, btnY), sf::Vector2f(80.f, 20.f));
    btnReset->setOnClick([this]() { 
        resetSettings(); 
        uiCtx.forceApply = false;
        uiCtx.forceDone = false;
        uiCtx.applyBtn->setLabel("APPLY");
        if (uiCtx.doneBtn) uiCtx.doneBtn->setLabel("DONE");
    });

    auto btnDone = std::make_unique<view::ui::UIButton>(
        font, "DONE", 8, sf::Vector2f(screenW - 100.f, btnY), sf::Vector2f(80.f, 20.f));
    uiCtx.doneBtn = btnDone.get();
    btnDone->setOnClick([this]() { 
        if (draft != model::SettingsManager::instance().get() && !uiCtx.forceDone) {
            uiCtx.forceDone = true;
            uiCtx.doneBtn->setLabel("SURE?");
            return;
        }
        if (manager) manager->popState(); 
    });

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
    tabPanels[0].setContentHeight(cursorY - 70.f);
}

void OptionsState::buildSoundTab(const sf::Font& font) {
    float cursorY = 80.f;
    auto master = std::make_unique<view::ui::UISlider>(
        font, "", 0, 100, 5, draft.masterVolume, sf::Vector2f(0,0), sf::Vector2f(150.f, 20.f));
    master->setOnChange([this](int v) { 
        draft.masterVolume = v; 
        if (context && context->audio) {
            context->audio->setMasterVolume(static_cast<float>(v));
            if (soundThrottleClock.getElapsedTime().asSeconds() > 0.15f) {
                context->audio->playSound("02. Beep");
                soundThrottleClock.restart();
            }
        }
    });
    addRow(tabPanels[1], font, "Master Volume", std::move(master), cursorY);

    auto music = std::make_unique<view::ui::UISlider>(
        font, "", 0, 100, 5, draft.musicVolume, sf::Vector2f(0,0), sf::Vector2f(150.f, 20.f));
    music->setOnChange([this](int v) { 
        draft.musicVolume = v; 
        if (context && context->audio) {
            context->audio->setMusicVolume(static_cast<float>(v));
            if (soundThrottleClock.getElapsedTime().asSeconds() > 0.15f) {
                context->audio->playSound("02. Beep");
                soundThrottleClock.restart();
            }
        }
    });
    addRow(tabPanels[1], font, "Music Volume", std::move(music), cursorY);

    auto sfx = std::make_unique<view::ui::UISlider>(
        font, "", 0, 100, 5, draft.sfxVolume, sf::Vector2f(0,0), sf::Vector2f(150.f, 20.f));
    sfx->setOnChange([this](int v) { 
        draft.sfxVolume = v;
        if (context && context->audio) {
            context->audio->setSFXVolume(static_cast<float>(v));
            if (soundThrottleClock.getElapsedTime().asSeconds() > 0.15f) {
                context->audio->playSound("02. Beep");
                soundThrottleClock.restart();
            }
        }
    });
    addRow(tabPanels[1], font, "SFX Volume", std::move(sfx), cursorY);
    tabPanels[1].setContentHeight(cursorY - 70.f);
}

void OptionsState::buildControlsTab(const sf::Font& font) {
    float cursorY = 80.f;
    std::vector<std::string> labels = {"Move Left", "Move Right", "Jump", "Run", "Pause", "Dash", "Attack", "Crouch", "Interact", "Inventory"};

    for (int i = 0; i < 10; ++i) {
        auto rowContainer = std::make_unique<view::ui::UIContainer>(view::ui::UIContainer::Layout::None);
        
        auto btn = std::make_unique<view::ui::UIButton>(
            font, "", 8, sf::Vector2f(0.f, 0.f), sf::Vector2f(100.f, 20.f));
        keyButtons[i] = btn.get();
        
        btn->setOnClick([this, i]() {
            if (waitingForKeyIndex != -1) {
                draft.*(KeyFieldMap[waitingForKeyIndex]) = pendingPreviousKey;
            }
            waitingForKeyIndex = i;
            pendingPreviousKey = draft.*(KeyFieldMap[i]);
            updateKeyButtonsVisuals();
        });

        auto rstBtn = std::make_unique<view::ui::UIButton>(
            font, "Reset", 8, sf::Vector2f(110.f, 0.f), sf::Vector2f(50.f, 20.f));
        rstBtn->setOnClick([this, i]() {
            if (waitingForKeyIndex == i) waitingForKeyIndex = -1;
            resetSingleKey(i);
            updateKeyButtonsVisuals();
        });

        rowContainer->add(std::move(btn));
        rowContainer->add(std::move(rstBtn));

        addRow(tabPanels[2], font, labels[i], std::move(rowContainer), cursorY);
    }
    updateKeyButtonsVisuals();
    tabPanels[2].setContentHeight(cursorY - 70.f);
}

void OptionsState::updateKeyButtonsVisuals() {
    std::array<int, 10> currentKeys;
    for (int i = 0; i < 10; ++i) currentKeys[i] = draft.*(KeyFieldMap[i]);
    
    for (int i = 0; i < 10; ++i) {
        if (!keyButtons[i]) continue;
        
        if (i == waitingForKeyIndex) {
            keyButtons[i]->setLabel("Press Key...");
            keyButtons[i]->setColors(sf::Color(150, 150, 50), sf::Color(180, 180, 80), sf::Color::White); // Yellow
            continue;
        }
        
        int key = currentKeys[i];
        if (key == -1) {
            keyButtons[i]->setLabel("< NOT BOUND >");
        } else {
            keyButtons[i]->setLabel(keyToString(key));
        }
        
        bool conflict = false;
        if (key != -1) {
            for (int j = 0; j < 10; ++j) {
                if (i != j && currentKeys[j] == key) {
                    conflict = true;
                    break;
                }
            }
        }
        
        if (conflict) {
            keyButtons[i]->setColors(sf::Color(150, 50, 50), sf::Color(180, 80, 80), sf::Color::White); // Red
        } else {
            keyButtons[i]->setColors(sf::Color(60, 60, 80), sf::Color(100, 100, 140), sf::Color::White); // Normal
        }
    }
}

bool OptionsState::hasKeyConflicts() const {
    std::array<int, 10> currentKeys;
    for (int i = 0; i < 10; ++i) currentKeys[i] = draft.*(KeyFieldMap[i]);

    for (int i = 0; i < 10; ++i) {
        if (currentKeys[i] == -1) continue;
        for (int j = i + 1; j < 10; ++j) {
            if (currentKeys[i] == currentKeys[j]) return true;
        }
    }
    return false;
}

void OptionsState::resetSingleKey(int index) {
    const auto& recent = model::SettingsManager::instance().get();
    draft.*(KeyFieldMap[index]) = recent.*(KeyFieldMap[index]);
}

void OptionsState::buildLanguageTab(const sf::Font& font) {
    float cursorY = 80.f;
    std::vector<std::string> opts = {"English", "Vietnamese"};
    auto lang = std::make_unique<view::ui::UICycleButton>(
        font, "", opts, static_cast<int>(draft.language), sf::Vector2f(0,0), sf::Vector2f(120.f, 20.f));
    lang->setOnChange([this](int idx) { draft.language = static_cast<model::Language>(idx); });
    addRow(tabPanels[3], font, "Language", std::move(lang), cursorY);
    tabPanels[3].setContentHeight(cursorY - 70.f);
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
    if (uiCtx.applyBtn) {
        if (draft != model::SettingsManager::instance().get()) {
            if (!uiCtx.forceApply) uiCtx.applyBtn->setLabel("APPLY");
            uiCtx.applyBtn->setColors(sf::Color(50, 150, 200), sf::Color(80, 180, 230), sf::Color::White); // Light Blue
        } else {
            uiCtx.forceApply = false;
            uiCtx.forceDone = false;
            uiCtx.applyBtn->setLabel("APPLY");
            if (uiCtx.doneBtn) uiCtx.doneBtn->setLabel("DONE");
            uiCtx.applyBtn->setColors(sf::Color(60, 60, 80), sf::Color(100, 100, 140), sf::Color::White); // Normal
        }
    }

    tabBar.update(dt);
    tabPanels[currentTab].update(dt);
    bottomBar.update(dt);
}

void OptionsState::render(sf::RenderTarget& target) {
    target.draw(background);
    titleLabel.render(target);
    tabBar.render(target);
    tabPanels[currentTab].render(target);
    bottomBar.render(target);
}

void OptionsState::handleEvent(const sf::Event& event) {
    if (waitingForKeyIndex != -1) {
        int keyCode = -1;
        if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
            keyCode = (keyEv->code == sf::Keyboard::Key::Escape) ? -1 : static_cast<int>(keyEv->code);
        } else if (const auto* btnEv = event.getIf<sf::Event::MouseButtonPressed>()) {
            keyCode = static_cast<int>(btnEv->button) + 100;
        }

        if (keyCode != -1 || (event.getIf<sf::Event::KeyPressed>() && event.getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)) {
            draft.*(KeyFieldMap[waitingForKeyIndex]) = keyCode;
            waitingForKeyIndex = -1;
            updateKeyButtonsVisuals();
            return;
        }
    }

    if (bottomBar.handleEvent(event)) return;
    if (tabBar.handleEvent(event)) return;
    if (tabPanels[currentTab].handleEvent(event)) return;
}

} // namespace controller
