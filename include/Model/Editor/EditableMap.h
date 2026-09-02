#ifndef MODEL_EDITOR_EDITABLEMAP_H
#define MODEL_EDITOR_EDITABLEMAP_H

#include "Model/Map/TileMap.h"
#include "Model/World/WorldType.h"

#include <cstddef>
#include <string>
#include <vector>

namespace model {

// In-memory grid mutated by the map editor. Mirrors TileMap's row convention exactly
// (row 0 is the BOTTOM of the world, i.e. the first grid line of a .map file) so the
// saved file loads back through the ordinary Level/TileMap pipeline unchanged. Kept
// separate from TileMap itself: TileMap is a runtime/gameplay class with no mutate or
// serialize API, and giving it one would pull editor concerns into the model every
// level load already depends on.
class EditableMap {
public:
    static constexpr std::size_t Rows = TileMap::Rows;
    static constexpr std::size_t DefaultColumns = 150;

    struct ValidationResult {
        bool ok = true;
        std::string message;
    };

    explicit EditableMap(WorldType world);

    char getTile(std::size_t row, std::size_t column) const;
    void setTile(std::size_t row, std::size_t column, char symbol);

    // Clears any existing 'M' cell before placing the new one: LevelScene only ever
    // honors the first player-spawn marker it finds, so a map can never have more
    // than one anyway (see LevelScene::resetLevel's 'M' case, `if (!playerPtr)`).
    void placePlayerSpawn(std::size_t row, std::size_t column);

    std::size_t columns() const { return m_columns; }
    std::size_t rows() const { return Rows; }

    WorldType getWorldType() const { return worldType; }
    void setWorldType(WorldType type) { worldType = type; }

    const std::string& getLevelName() const { return levelName; }
    void setLevelName(const std::string& name) { levelName = name; }

    // A map with no goal can never be completed, and one with no player spawn can
    // never be entered — both are required exactly once before it can be saved.
    ValidationResult validate() const;

    // Builds a real TileMap from the current grid (via TileMap::loadFromLines), so the
    // editor can draw its live preview with the exact same view::TileMapRenderer the
    // game itself uses.
    TileMap toTileMap() const;

    // Header + 16 grid lines, in the minimal single-area format Level::loadFromFile
    // accepts (no '; area', '; pipe=', '; slider=' or '; next=' lines).
    std::string serialize() const;

    bool saveToFile(const std::string& dir = "assets/maps/custom/") const;

    // Sorted list of "*.map" file paths under dir. Never throws if dir doesn't exist
    // yet (nothing has been saved) — returns an empty list instead.
    static std::vector<std::string> listCustomMaps(const std::string& dir = "assets/maps/custom/");

private:
    static std::string sanitizeFileName(const std::string& name);

    std::vector<std::vector<char>> tiles;  // tiles[row][column], row 0 = bottom
    std::size_t m_columns = DefaultColumns;
    WorldType worldType;
    std::string levelName;
};

} // namespace model

#endif // MODEL_EDITOR_EDITABLEMAP_H
