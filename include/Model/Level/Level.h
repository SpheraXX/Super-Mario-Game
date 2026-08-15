#ifndef MODEL_LEVEL_H
#define MODEL_LEVEL_H

#include "Model/Map/TileMap.h"
#include "Model/World/WorldType.h"

#include <cstddef>
#include <string>
#include <vector>

namespace model {

// Which end of the pipe the player enters from. Kept for forward compatibility with the
// token syntax; the classic gameplay enters "down" — Mario presses Down while standing
// on the top of the pipe.
enum class PortalDirection { Down, Up };

// One explicit warp connection between two areas of the same level.
struct Portal {
    std::size_t sourceColumn;        // anchor column of the 'P' run in the source area
    PortalDirection direction;       // expected entry direction (informational)
    std::size_t destinationArea;     // 0-based area index
    std::size_t destinationColumn;   // column the player re-emerges at
};

// A multi-area level wrapped in a single .map file. The file is basically a sequence of
// segments, each of the form:
//
//   ; area
//   ; world=overworld
//   <16 grid rows>
//   ; pipe=col:24,enter:down,to:1:4
//   ; area
//   ; world=underwater
//   <16 grid rows>
//   ; pipe=col:6,enter:up,to:2:58
//
// Level-level metadata (`; name`, `; next`) lives before the first area. Playable areas
// are the seed-communicated to PlayState, which teleports the player between them via
// portals. The LAST area must be Overworld: the completion zone (flagpole + castle) is
// only procedural there.
class Level {
public:
    void loadFromFile(const std::string& filePath);

    std::size_t areaCount() const;
    TileMap& areaMap(std::size_t index);
    const TileMap& areaMap(std::size_t index) const;
    WorldType areaWorld(std::size_t index) const;
    const std::vector<Portal>& portals(std::size_t index) const;

    const std::string& getLevelName() const;
    const std::string& getNextMapPath() const;
    bool hasNextMap() const;

private:
    struct Area {
        TileMap map;
        WorldType world = WorldType::Overworld;
        std::vector<Portal> portals;
    };

    std::vector<Area> areas;
    std::string levelName;
    std::string nextMapPath;
};

}

#endif