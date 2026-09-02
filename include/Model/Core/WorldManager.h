#ifndef MODEL_CORE_WORLDMANAGER_H
#define MODEL_CORE_WORLDMANAGER_H

#include "Model/Core/WorldData.h"
#include <string>
#include <vector>

namespace model {

class WorldManager {
public:
    static WorldManager& instance();

    void load(const std::string& filepath = "assets/data/worlds.json");
    const std::vector<WorldData>& getWorlds() const;
    const WorldData* getWorld(const std::string& id) const;

private:
    WorldManager() = default;
    ~WorldManager() = default;

    std::vector<WorldData> m_worlds;
};

} // namespace model

#endif // MODEL_CORE_WORLDMANAGER_H
