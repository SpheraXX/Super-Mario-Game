#ifndef MODEL_SETTINGSMANAGER_H
#define MODEL_SETTINGSMANAGER_H

#include "Model/Settings.h"
#include <functional>
#include <vector>

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

    void subscribe(std::function<void(const Settings&)> callback);

private:
    SettingsManager();
    void load();
    void save() const;
    Settings current;
    std::vector<std::function<void(const Settings&)>> subscribers;
};

}  // namespace model

#endif
