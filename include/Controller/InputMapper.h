#ifndef CONTROLLER_INPUTMAPPER_H
#define CONTROLLER_INPUTMAPPER_H

#include "Model/Input/IInputMapper.h"
#include "Model/SettingsManager.h"
#include <SFML/Window/Keyboard.hpp>
#include <unordered_map>

namespace controller {

class InputMapper : public model::IInputMapper, public model::ISettingsObserver {
public:
    InputMapper();
    
    bool isActionPressed(model::InputAction action) const override;
    
    void onSettingsChanged(const model::Settings& settings) override;

    // Convert a raw key code (as stored in Settings) to a human-readable string.
    // Lives here rather than in OptionsState so any future UI widget that needs to
    // display key names can call this without duplicating the mapping.
    static std::string getKeyName(int keyCode);

private:
    std::unordered_map<model::InputAction, sf::Keyboard::Key> bindings;
};

} // namespace controller

#endif
