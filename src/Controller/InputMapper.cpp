#include "Controller/InputMapper.h"
#include "Model/SettingsManager.h"
#include <iostream>
#include <string>
#include <unordered_map>

namespace controller {

InputMapper::InputMapper() {
    // Register to listen to settings changes to keep bindings updated
    model::SettingsManager::instance().addObserver(this);
}

void InputMapper::onSettingsChanged(const model::Settings& s) {
    bindings[model::InputAction::MoveLeft]  = static_cast<sf::Keyboard::Key>(s.keyMoveLeft);
    bindings[model::InputAction::MoveRight] = static_cast<sf::Keyboard::Key>(s.keyMoveRight);
    bindings[model::InputAction::Jump]      = static_cast<sf::Keyboard::Key>(s.keyJump);
    bindings[model::InputAction::Run]       = static_cast<sf::Keyboard::Key>(s.keyRun);
    bindings[model::InputAction::Pause]     = static_cast<sf::Keyboard::Key>(s.keyPause);
    bindings[model::InputAction::Attack]    = static_cast<sf::Keyboard::Key>(s.keyAttack);
    bindings[model::InputAction::Crouch]    = static_cast<sf::Keyboard::Key>(s.keyCrouch);
    bindings[model::InputAction::CycleDisplay] = static_cast<sf::Keyboard::Key>(s.keyCycleDisplay);
}

bool InputMapper::isActionPressed(model::InputAction action) const {
    auto it = bindings.find(action);
    if (it != bindings.end()) {
        sf::Keyboard::Key key = it->second;
        // In SFML 3, Unknown is sf::Keyboard::Key::Unknown
        if (key != sf::Keyboard::Key::Unknown && static_cast<int>(key) != -1) {
            return sf::Keyboard::isKeyPressed(key);
        }
    }
    return false;
}

std::string InputMapper::getKeyName(int keyCode) {
    if (keyCode == -1) return "<NOT BOUND>";
    // Mouse buttons are encoded as keyCode >= 100 by OptionsState convention.
    if (keyCode >= 100) {
        static const std::unordered_map<int, std::string> mouseNames = {
            {100, "Mouse L"}, {101, "Mouse R"}, {102, "Mouse M"},
            {103, "Mouse X1"}, {104, "Mouse X2"}
        };
        auto it = mouseNames.find(keyCode);
        return it != mouseNames.end() ? it->second : "Mouse " + std::to_string(keyCode - 100);
    }
    // Keyboard keys: built once, looked up in O(1).
    static const std::unordered_map<int, std::string> keyNames = {
        {(int)sf::Keyboard::Key::A, "A"}, {(int)sf::Keyboard::Key::B, "B"},
        {(int)sf::Keyboard::Key::C, "C"}, {(int)sf::Keyboard::Key::D, "D"},
        {(int)sf::Keyboard::Key::E, "E"}, {(int)sf::Keyboard::Key::F, "F"},
        {(int)sf::Keyboard::Key::G, "G"}, {(int)sf::Keyboard::Key::H, "H"},
        {(int)sf::Keyboard::Key::I, "I"}, {(int)sf::Keyboard::Key::J, "J"},
        {(int)sf::Keyboard::Key::K, "K"}, {(int)sf::Keyboard::Key::L, "L"},
        {(int)sf::Keyboard::Key::M, "M"}, {(int)sf::Keyboard::Key::N, "N"},
        {(int)sf::Keyboard::Key::O, "O"}, {(int)sf::Keyboard::Key::P, "P"},
        {(int)sf::Keyboard::Key::Q, "Q"}, {(int)sf::Keyboard::Key::R, "R"},
        {(int)sf::Keyboard::Key::S, "S"}, {(int)sf::Keyboard::Key::T, "T"},
        {(int)sf::Keyboard::Key::U, "U"}, {(int)sf::Keyboard::Key::V, "V"},
        {(int)sf::Keyboard::Key::W, "W"}, {(int)sf::Keyboard::Key::X, "X"},
        {(int)sf::Keyboard::Key::Y, "Y"}, {(int)sf::Keyboard::Key::Z, "Z"},
        {(int)sf::Keyboard::Key::Num0, "0"}, {(int)sf::Keyboard::Key::Num1, "1"},
        {(int)sf::Keyboard::Key::Num2, "2"}, {(int)sf::Keyboard::Key::Num3, "3"},
        {(int)sf::Keyboard::Key::Num4, "4"}, {(int)sf::Keyboard::Key::Num5, "5"},
        {(int)sf::Keyboard::Key::Num6, "6"}, {(int)sf::Keyboard::Key::Num7, "7"},
        {(int)sf::Keyboard::Key::Num8, "8"}, {(int)sf::Keyboard::Key::Num9, "9"},
        {(int)sf::Keyboard::Key::Escape,    "Escape"},
        {(int)sf::Keyboard::Key::LControl,  "LCtrl"},
        {(int)sf::Keyboard::Key::LShift,    "LShift"},
        {(int)sf::Keyboard::Key::LAlt,      "LAlt"},
        {(int)sf::Keyboard::Key::RControl,  "RCtrl"},
        {(int)sf::Keyboard::Key::RShift,    "RShift"},
        {(int)sf::Keyboard::Key::RAlt,      "RAlt"},
        {(int)sf::Keyboard::Key::Space,     "Space"},
        {(int)sf::Keyboard::Key::Enter,     "Enter"},
        {(int)sf::Keyboard::Key::Backspace, "Backspace"},
        {(int)sf::Keyboard::Key::Tab,       "Tab"},
        {(int)sf::Keyboard::Key::Up,        "Up"},
        {(int)sf::Keyboard::Key::Down,      "Down"},
        {(int)sf::Keyboard::Key::Left,      "Left"},
        {(int)sf::Keyboard::Key::Right,     "Right"},
        {(int)sf::Keyboard::Key::F1,        "F1"},
        {(int)sf::Keyboard::Key::F2,        "F2"},
        {(int)sf::Keyboard::Key::F3,        "F3"},
        {(int)sf::Keyboard::Key::F4,        "F4"},
        {(int)sf::Keyboard::Key::F5,        "F5"},
        {(int)sf::Keyboard::Key::F6,        "F6"},
        {(int)sf::Keyboard::Key::F7,        "F7"},
        {(int)sf::Keyboard::Key::F8,        "F8"},
        {(int)sf::Keyboard::Key::F9,        "F9"},
        {(int)sf::Keyboard::Key::F10,       "F10"},
        {(int)sf::Keyboard::Key::F11,       "F11"},
        {(int)sf::Keyboard::Key::F12,       "F12"},
    };
    auto it = keyNames.find(keyCode);
    return it != keyNames.end() ? it->second : "Key " + std::to_string(keyCode);
}

} // namespace controller
