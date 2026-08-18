#ifndef MODEL_SETTINGSMANAGER_H
#define MODEL_SETTINGSMANAGER_H

#include "Model/Settings.h"
#include <vector>

namespace model {

class ISettingsObserver {
public:
    virtual ~ISettingsObserver() = default;
    virtual void onSettingsChanged(const Settings& settings) = 0;
};

class SettingsManager {
public:
    static SettingsManager& instance();

    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    const Settings& get() const { return current; }
    void apply(const Settings& next);
    void resetToDefaults();

    static constexpr const char* FilePath = "assets/data/settings.json";

    void addObserver(ISettingsObserver* observer);
    void removeObserver(ISettingsObserver* observer);

private:
    SettingsManager();
    void load();
    void save() const;
    Settings current;
    std::vector<ISettingsObserver*> observers;
};

}  // namespace model

#endif
