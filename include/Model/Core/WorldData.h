#ifndef MODEL_CORE_WORLDDATA_H
#define MODEL_CORE_WORLDDATA_H

#include <string>
#include <vector>

namespace model {

struct LevelData {
    std::string id;
    std::string mapPath;
};

struct WorldData {
    std::string id;
    std::string title;
    std::string previewImage;
    std::string bgaImage;
    std::vector<LevelData> levels;
};

} // namespace model

#endif // MODEL_CORE_WORLDDATA_H
