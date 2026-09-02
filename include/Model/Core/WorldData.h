#ifndef MODEL_CORE_WORLDDATA_H
#define MODEL_CORE_WORLDDATA_H

#include <string>
#include <vector>

namespace model {

struct LevelData {
    std::string id;
    std::string mapPath;
    std::string unlockRequires;
};

struct WorldData {
    std::string id;
    std::string title;
    std::string previewImage;
    std::string bgaImage;
    // Audio track ID (key in audio_meta.json) to play while browsing this world's level
    // select screen and during gameplay. Defaults to "overworld" if not specified in JSON.
    std::string musicTrack;
    std::vector<LevelData> levels;
};

} // namespace model

#endif // MODEL_CORE_WORLDDATA_H
