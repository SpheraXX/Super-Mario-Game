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

    std::string getActiveSavePath() const;

    bool hasSaveFile(const std::string& path = "") const;
    bool save(const GameSaveData& data, const std::string& path = "") const;
    bool load(GameSaveData& outData, const std::string& path = "") const;
    bool deleteSave(const std::string& path = "") const;

private:
    SaveManager() = default;
};

} // namespace model

#endif // MODEL_SAVE_SAVEMANAGER_H
