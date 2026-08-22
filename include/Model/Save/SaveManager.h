#ifndef MODEL_SAVE_SAVEMANAGER_H
#define MODEL_SAVE_SAVEMANAGER_H

#include "Model/Save/SaveData.h"
#include <string>

namespace model {

class SaveManager {
public:
    static SaveManager& instance();

    SaveManager(const SaveManager&) = delete;
    SaveManager& operator=(const SaveManager&) = delete;

    static constexpr const char* DefaultSavePath = "assets/data/savegame.json";

    bool hasSaveFile(const std::string& path = DefaultSavePath) const;
    bool save(const GameSaveData& data, const std::string& path = DefaultSavePath) const;
    bool load(GameSaveData& outData, const std::string& path = DefaultSavePath) const;
    bool deleteSave(const std::string& path = DefaultSavePath) const;

private:
    SaveManager() = default;
};

} // namespace model

#endif // MODEL_SAVE_SAVEMANAGER_H
