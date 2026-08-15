#ifndef MODEL_SETTINGSMANAGER_H
#define MODEL_SETTINGSMANAGER_H

#include "Model/Settings.h"

namespace model {

class SettingsManager {
public:
    static SettingsManager& instance();

    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    const Settings& get() const { return current; }
    void apply(const Settings& next);
    void resetToDefaults();

    static constexpr const char* FilePath = "assets/data/settings.json";

private:
    SettingsManager();
    void load();
    void save() const;
    Settings current;
};

}  // namespace model

#endif
