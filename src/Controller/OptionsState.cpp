#include "Controller/OptionsState.h"
#include "Controller/AppEngine.h"
#include "Controller/IAudioManager.h"
#include "Controller/InputMapper.h"
#include "Controller/StateManager.h"
#include "Controller/WarningPopupState.h"
#include "Model/SettingsManager.h"
#include "View/AssetManager.h"
#include "View/UI/UICycleButton.h"
#include "View/UI/UIKeyIcon.h"
#include "View/UI/UIScrollView.h"
#include "View/UI/SolidButtonSkin.h"
#include "View/UI/UILabel.h"
#include "View/UI/UISlider.h"
#include "View/UI/UITheme.h"

namespace controller {

namespace {
int model::Settings::*const KeyFieldMap[11] = {
    &model::Settings::keyMoveLeft,     &model::Settings::keyMoveRight,
    &model::Settings::keyJump,         &model::Settings::keyRun,
    &model::Settings::keyPause,        &model::Settings::keyDash,
    &model::Settings::keyAttack,       &model::Settings::keyCrouch,
    &model::Settings::keyInteract,     &model::Settings::keyInventory,
    &model::Settings::keyCycleDisplay};
}

// Layout constants for addRow
static constexpr float kRowHeight      = 25.f;
static constexpr float kRowPaddingX    = 20.f;
static constexpr float kWidgetOffsetX  = 130.f;

void OptionsState::addRow(view::ui::UIContainer &parent, const sf::Font &font,
                          const std::string &label,
                          std::unique_ptr<view::ui::UIElement> widget,
                          float &cursorY) {
  auto lbl = std::make_unique<view::ui::UILabel>(font, label, view::ui::layout::ButtonFontSize);
  lbl->setPosition(kRowPaddingX, cursorY + 4.f);
  widget->setPosition(kWidgetOffsetX, cursorY);
  parent.add(std::move(lbl));
  parent.add(std::move(widget));
  cursorY += kRowHeight;
}

OptionsState::OptionsState() 
  : draft(model::SettingsManager::instance().get()),
    bgaSprite(view::AssetManager::instance().getTexture("assets/images/bga_options.png")) {

  bgaSprite.setColor(view::ui::theme::BgaDimOptions);

  float W = static_cast<float>(AppEngine::screenWidth());
  float H = static_cast<float>(AppEngine::ScreenHeight);
  float scale = H / static_cast<float>(bgaSprite.getTexture().getSize().y);
  bgaSprite.setScale({scale, scale});
  bgaSprite.setOrigin({static_cast<float>(bgaSprite.getTexture().getSize().x) / 2.f, static_cast<float>(bgaSprite.getTexture().getSize().y) / 2.f});
  bgaSprite.setPosition({W / 2.f, H / 2.f});

  const sf::Font &font = view::AssetManager::instance().getUiFont();

  // Title
  titleLabel = view::ui::UILabel(font, "OPTIONS", view::ui::layout::TitleFontSize);
  titleLabel.setPosition(10.f, 10.f);

  // Tab Bar Setup (Horizontal via manual layout)
  tabBar = view::ui::UIScrollView();
  tabBar.setPosition(10.f, 40.f);

  std::vector<std::string> tabNames = {"GRAPHICS", "SOUND", "CONTROLS",
                                       "LANGUAGE"};
  float currentX = 0.f; // Local to tabBar

  float screenW = static_cast<float>(AppEngine::screenWidth());
  float screenH = static_cast<float>(AppEngine::ScreenHeight);
  float btnY = screenH - 30.f;

  for (int i = 0; i < 4; ++i) {
    auto btn = std::make_unique<view::ui::UIButton>(font, tabNames[i], view::ui::layout::ButtonFontSize,
                                                    sf::Vector2f(currentX, 0.f),
                                                    sf::Vector2f(view::ui::layout::SmallButtonWidth, view::ui::layout::SmallButtonHeight));
    btn->setOnClick([this, i]() { switchTab(i); });
    tabBar.add(std::move(btn));
    currentX += 85.f;

    tabPanels[i] = view::ui::UIScrollView();
    tabPanels[i].setPosition(10.f, 70.f);
    tabPanels[i].setBounds(
        sf::FloatRect({10.f, 70.f}, {screenW - 20.f, btnY - 70.f}));
    tabPanels[i].setVisible(false);
  }
  tabBar.setContentWidth(currentX);
  tabBar.setContentHeight(view::ui::layout::SmallButtonHeight);
  tabBar.setBounds(sf::FloatRect({10.f, 40.f}, {screenW - 20.f, view::ui::layout::SmallButtonHeight + 6.f}));
  tabPanels[0].setVisible(true);

  buildGraphicsTab(font);
  buildSoundTab(font);
  buildControlsTab(font);
  buildLanguageTab(font);

  // Bottom Bar Setup (Horizontal via manual layout)
  bottomBar = view::ui::UIContainer(view::ui::UIContainer::Layout::None);
  bottomBar.setPosition(0.f, 0.f);

  auto btnApply = std::make_unique<view::ui::UIButton>(
      font, "APPLY", view::ui::layout::ButtonFontSize, sf::Vector2f(screenW - 280.f, btnY),
      sf::Vector2f(view::ui::layout::SmallButtonWidth, view::ui::layout::SmallButtonHeight));
  uiCtx.applyBtn = btnApply.get();
  btnApply->setOnClick([this]() {
    if (hasKeyConflicts()) {
      manager->pushState(std::make_unique<WarningPopupState>(
          "Key conflicts detected!\nForce apply anyway?",
          WarningPopupState::Type::YesNo,
          [this]() {
            manager->popState();
            applySettings();
          },
          [this]() { manager->popState(); }));
    } else {
      applySettings();
    }
  });

  auto btnReset = std::make_unique<view::ui::UIButton>(
      font, "RESET", view::ui::layout::ButtonFontSize, sf::Vector2f(screenW - 190.f, btnY),
      sf::Vector2f(view::ui::layout::SmallButtonWidth, view::ui::layout::SmallButtonHeight));
  uiCtx.resetBtn = btnReset.get();
  btnReset->setOnClick([this]() { resetSettings(); });

  auto btnDone = std::make_unique<view::ui::UIButton>(
      font, "DONE", view::ui::layout::ButtonFontSize, sf::Vector2f(screenW - 100.f, btnY),
      sf::Vector2f(view::ui::layout::SmallButtonWidth, view::ui::layout::SmallButtonHeight));
  uiCtx.doneBtn = btnDone.get();
  btnDone->setOnClick([this]() {
    if (draft != model::SettingsManager::instance().get()) {
      manager->pushState(std::make_unique<WarningPopupState>(
          "You have unsaved changes.\nDiscard and exit?",
          WarningPopupState::Type::YesNo,
          [this]() {
            manager->popState(); // pop WarningPopupState
            manager->popState(); // pop OptionsState
          },
          [this]() {
            manager->popState(); // pop WarningPopupState
          }));
    } else {
      manager->popState();
    }
  });

  bottomBar.add(std::move(btnApply));
  bottomBar.add(std::move(btnReset));
  bottomBar.add(std::move(btnDone));
}

void OptionsState::onDisplayModeChanged() {
  float screenW = static_cast<float>(AppEngine::screenWidth());
  relayout(screenW);

  // Rebuild graphics tab to reflect the new screen width/ratio.
  // Do NOT sync draft from SettingsManager here — that would overwrite
  // unsaved user changes (e.g. after pressing NO in ConfirmState).
  // Nullify the raw cache pointer BEFORE clear() destroys the widget it points
  // to.
  resolutionBtn = nullptr;
  tabPanels[0].clear();
  buildGraphicsTab(view::AssetManager::instance().getUiFont());
}

void OptionsState::onResume() {
  // Preserve 'draft' so unsaved changes aren't lost when returning from
  // ConfirmState. Only refresh display settings (in case F2 was pressed
  // globally) and re-apply key visuals (colors may need refresh after state
  // change).
  updateKeyButtonsVisuals();
  onDisplayModeChanged();
}

void OptionsState::relayout(float screenW) {
  float screenH = static_cast<float>(AppEngine::ScreenHeight);
  
  const sf::Texture& tex = bgaSprite.getTexture();
  float scale = screenH / static_cast<float>(tex.getSize().y);
  bgaSprite.setScale({scale, scale});
  bgaSprite.setOrigin({static_cast<float>(tex.getSize().x) / 2.f, static_cast<float>(tex.getSize().y) / 2.f});
  bgaSprite.setPosition({screenW / 2.f, screenH / 2.f});

  tabBar.setBounds(
      sf::FloatRect({10.f, 40.f}, {screenW - 20.f, view::ui::layout::SmallButtonHeight + 6.f}));

  for (int i = 0; i < 4; ++i) {
    tabPanels[i].setBounds(
        sf::FloatRect({10.f, 70.f}, {screenW - 20.f, screenH - 100.f}));
  }

  if (uiCtx.applyBtn) {
    uiCtx.applyBtn->setPosition(screenW - 280.f, screenH - 30.f);
  }
  if (uiCtx.resetBtn) {
    uiCtx.resetBtn->setPosition(screenW - 190.f, screenH - 30.f);
  }
  if (uiCtx.doneBtn) {
    uiCtx.doneBtn->setPosition(screenW - 100.f, screenH - 30.f);
  }
}

void OptionsState::buildGraphicsTab(const sf::Font &font) {
  float cursorY = 10.f;
  auto fsBtn = std::make_unique<view::ui::UICycleButton>(
      font, "", std::vector<std::string>{"Windowed", "Fullscreen"},
      draft.fullscreen ? 1 : 0, sf::Vector2f(0.f, 0.f),
      sf::Vector2f(120.f, view::ui::layout::SmallButtonHeight));
  fsBtn->setOnChange([this](int idx) { draft.fullscreen = (idx == 1); });
  addRow(tabPanels[0], font, "Screen Mode", std::move(fsBtn), cursorY);

  std::vector<std::string> ratioOpts = {"4:3", "16:9"};
  auto ratioBtn = std::make_unique<view::ui::UICycleButton>(
      font, "", ratioOpts, static_cast<int>(draft.ratio),
      sf::Vector2f(0.f, 0.f), sf::Vector2f(120.f, view::ui::layout::SmallButtonHeight));
  ratioBtn->setOnChange([this](int idx) {
    draft.ratio = static_cast<model::AspectRatio>(idx);
    draft.resolutionIndex = 0; // reset to default
    if (resolutionBtn) {
      std::vector<std::string> resOpts;
      if (draft.ratio == model::AspectRatio::Ratio4x3) {
        resOpts = {"800x600", "1024x768"};
      } else {
        resOpts = {"1280x720", "1920x1080"};
      }
      resolutionBtn->setOptions(resOpts, 0);
    }
  });
  addRow(tabPanels[0], font, "Aspect Ratio", std::move(ratioBtn), cursorY);

  std::vector<std::string> resOpts;
  if (draft.ratio == model::AspectRatio::Ratio4x3) {
    resOpts = {"800x600", "1024x768"};
  } else {
    resOpts = {"1280x720", "1920x1080"};
  }

  int resIdx = std::clamp(draft.resolutionIndex, 0,
                          static_cast<int>(resOpts.size() - 1));
  auto resBtn = std::make_unique<view::ui::UICycleButton>(
      font, "", resOpts, resIdx, sf::Vector2f(0.f, 0.f),
      sf::Vector2f(120.f, view::ui::layout::SmallButtonHeight));
  resolutionBtn = resBtn.get();
  resBtn->setOnChange([this](int idx) { draft.resolutionIndex = idx; });
  addRow(tabPanels[0], font, "Resolution", std::move(resBtn), cursorY);

  std::vector<std::string> qOpts = {"Low", "Medium", "High"};
  auto qBtn = std::make_unique<view::ui::UICycleButton>(
      font, "", qOpts, static_cast<int>(draft.quality), sf::Vector2f(0.f, 0.f),
      sf::Vector2f(120.f, view::ui::layout::SmallButtonHeight));
  qBtn->setOnChange([this](int idx) {
    draft.quality = static_cast<model::GraphicsQuality>(idx);
  });
  addRow(tabPanels[0], font, "Quality", std::move(qBtn), cursorY);
  tabPanels[0].setContentHeight(cursorY);
}

void OptionsState::addVolumeRow(view::ui::UIScrollView &panel,
                               const sf::Font &font, const std::string &label,
                               int initialValue,
                               std::function<void(int)> onChange,
                               float &cursorY) {
  auto slider = std::make_unique<view::ui::UISlider>(
      font, "", 0, 100, 5, initialValue, sf::Vector2f(0, 0),
      sf::Vector2f(130.f, view::ui::layout::SmallButtonHeight));
  slider->setOnChange([this, onChange = std::move(onChange)](int v) {
    onChange(v);
    if (context && context->audio) {
      if (soundThrottleClock.getElapsedTime().asSeconds() > 0.15f) {
        context->audio->playSound("02. Beep");
        soundThrottleClock.restart();
      }
    }
  });
  addRow(panel, font, label, std::move(slider), cursorY);
}

void OptionsState::buildSoundTab(const sf::Font &font) {
  float cursorY = 10.f;
  addVolumeRow(tabPanels[1], font, "Master Volume", draft.masterVolume,
               [this](int v) {
                 draft.masterVolume = v;
                 if (context && context->audio)
                   context->audio->setMasterVolume(static_cast<float>(v));
               }, cursorY);
  addVolumeRow(tabPanels[1], font, "Music Volume", draft.musicVolume,
               [this](int v) {
                 draft.musicVolume = v;
                 if (context && context->audio)
                   context->audio->setMusicVolume(static_cast<float>(v));
               }, cursorY);
  addVolumeRow(tabPanels[1], font, "SFX Volume", draft.sfxVolume,
               [this](int v) {
                 draft.sfxVolume = v;
                 if (context && context->audio)
                   context->audio->setSFXVolume(static_cast<float>(v));
               }, cursorY);
  tabPanels[1].setContentHeight(cursorY);
}

void OptionsState::buildControlsTab(const sf::Font &font) {
  float cursorY = 10.f;
  std::vector<std::string> keyLabels = {
      "Move Left", "Move Right", "Jump",     "Run",
      "Pause",     "Dash",       "Attack",   "Crouch",
      "Interact",  "Inventory",  "Cycle Display"};

  for (int i = 0; i < 11; ++i) {
    auto rowContainer = std::make_unique<view::ui::UIContainer>(
        view::ui::UIContainer::Layout::None);

    auto icon = std::make_unique<view::ui::UIKeyIcon>();
    icon->setPosition(0.f, 0.f);
    keyIcons[i] = icon.get();

    auto btn = std::make_unique<view::ui::UIButton>(
        font, "", view::ui::layout::ButtonFontSize, sf::Vector2f(0.f, 0.f), sf::Vector2f(50.f, view::ui::layout::SmallButtonHeight));
    btn->setColors(view::ui::theme::ColorMaskNormal,
                   view::ui::theme::ColorMaskHovered, sf::Color::Transparent);
    btn->setMaskMode(true); // stays transparent even when disabled
    keyButtons[i] = btn.get();

    if (i == 10) {
      btn->setEnabled(false);
    } else {
      btn->setOnClick([this, i]() {
        if (waitingForKeyIndex != -1) {
          draft.*(KeyFieldMap[waitingForKeyIndex]) = pendingPreviousKey;
        }
        waitingForKeyIndex = i;
        pendingPreviousKey = draft.*(KeyFieldMap[i]);
        updateKeyButtonsVisuals();
      });
    }

    auto rstBtn = std::make_unique<view::ui::UIButton>(
        font, "Reset", view::ui::layout::ButtonFontSize, sf::Vector2f(60.f, 0.f), sf::Vector2f(45.f, view::ui::layout::SmallButtonHeight));
    if (i == 10) {
      rstBtn->setEnabled(false);
    } else {
      rstBtn->setOnClick([this, i]() {
        if (waitingForKeyIndex == i)
          waitingForKeyIndex = -1;
        resetSingleKey(i);
        updateKeyButtonsVisuals();
      });
    }

    rowContainer->add(std::move(icon));
    rowContainer->add(std::move(btn));
    rowContainer->add(std::move(rstBtn));

    addRow(tabPanels[2], font, keyLabels[i], std::move(rowContainer), cursorY);
  }
  updateKeyButtonsVisuals();
  tabPanels[2].setContentHeight(cursorY);
}

void OptionsState::updateKeyButtonsVisuals() {
  std::array<int, 11> currentKeys;
  for (int i = 0; i < 11; ++i)
    currentKeys[i] = draft.*(KeyFieldMap[i]);

  for (int i = 0; i < 11; ++i) {
    if (!keyButtons[i] || !keyIcons[i])
      continue;

    if (i == waitingForKeyIndex) {
      keyIcons[i]->setKey(sf::Keyboard::Key::Unknown);
      keyIcons[i]->setColor(view::ui::theme::ColorIconWaiting);
      keyButtons[i]->setSize(keyIcons[i]->getSize().x,
                             keyIcons[i]->getSize().y);
      continue;
    }

    int key = currentKeys[i];
    if (key != -1) {
      keyIcons[i]->setKey(static_cast<sf::Keyboard::Key>(key));
    } else {
      keyIcons[i]->setKey(sf::Keyboard::Key::Unknown);
    }
    keyButtons[i]->setSize(keyIcons[i]->getSize().x, keyIcons[i]->getSize().y);

    bool conflict = false;
    if (key != -1) {
      for (int j = 0; j < 11; ++j) {
        if (i != j && currentKeys[j] == key) {
          conflict = true;
          break;
        }
      }
    }

    if (conflict) {
      keyIcons[i]->setColor(view::ui::theme::ColorIconConflict);
    } else {
      keyIcons[i]->setColor(view::ui::theme::ColorIconNormal);
    }
  }
}

bool OptionsState::hasKeyConflicts() const {
  std::array<int, 11> currentKeys;
  for (int i = 0; i < 11; ++i)
    currentKeys[i] = draft.*(KeyFieldMap[i]);

  for (int i = 0; i < 11; ++i) {
    if (currentKeys[i] == -1)
      continue;
    for (int j = i + 1; j < 11; ++j) {
      if (currentKeys[i] == currentKeys[j])
        return true;
    }
  }
  return false;
}

void OptionsState::resetSingleKey(int index) {
  // Reset to factory default, not last-applied, to be consistent with RESET
  // ALL.
  draft.*(KeyFieldMap[index]) =
      model::Settings::defaults().*(KeyFieldMap[index]);
}

void OptionsState::buildLanguageTab(const sf::Font &font) {
  float cursorY = 10.f;
  std::vector<std::string> opts = {"English", "Vietnamese"};
  auto lang = std::make_unique<view::ui::UICycleButton>(
      font, "", opts, static_cast<int>(draft.language), sf::Vector2f(0, 0),
      sf::Vector2f(120.f, view::ui::layout::SmallButtonHeight));
  lang->setOnChange(
      [this](int idx) { draft.language = static_cast<model::Language>(idx); });
  addRow(tabPanels[3], font, "Language", std::move(lang), cursorY);
  tabPanels[3].setContentHeight(cursorY);
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
  // Nullify raw cache pointers BEFORE clear() destroys the widgets they point
  // to.
  resolutionBtn = nullptr;
  keyButtons.fill(nullptr);
  keyIcons.fill(nullptr);
  for (int i = 0; i < 4; ++i) {
    tabPanels[i].clear();
  }
  const sf::Font &font = view::AssetManager::instance().getUiFont();
  buildGraphicsTab(font);
  buildSoundTab(font);
  buildControlsTab(font);
  buildLanguageTab(font);
}

void OptionsState::update(float dt) {
  if (uiCtx.applyBtn) {
    if (draft != model::SettingsManager::instance().get() && !hasKeyConflicts()) {
      uiCtx.applyBtn->setEnabled(true);
    } else {
      uiCtx.applyBtn->setEnabled(false);
    }
  }

  tabBar.update(dt);
  tabPanels[currentTab].update(dt);
  bottomBar.update(dt);
}

void OptionsState::render(sf::RenderTarget &target) {
  target.draw(bgaSprite);
  titleLabel.render(target);
  tabBar.render(target);
  tabPanels[currentTab].render(target);
  bottomBar.render(target);
}

void OptionsState::handleEvent(const sf::Event &event) {
  if (waitingForKeyIndex != -1) {
    bool inputReceived = false;
    int keyCode = -1;

    if (const auto *keyEv = event.getIf<sf::Event::KeyPressed>()) {
      keyCode = (keyEv->code == sf::Keyboard::Key::Escape)
                    ? -1
                    : static_cast<int>(keyEv->code);
      inputReceived = true;
    } else if (const auto *btnEv =
                   event.getIf<sf::Event::MouseButtonPressed>()) {
      keyCode = static_cast<int>(btnEv->button) + 100;
      inputReceived = true;
    }

    if (inputReceived) {
      draft.*(KeyFieldMap[waitingForKeyIndex]) = keyCode;
      waitingForKeyIndex = -1;
      updateKeyButtonsVisuals();
      return;
    }
  }

  if (bottomBar.handleEvent(event))
    return;
  if (tabBar.handleEvent(event))
    return;
  if (tabPanels[currentTab].handleEvent(event))
    return;
}

} // namespace controller
