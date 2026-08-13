#ifndef MODEL_TILEMAP_H
#define MODEL_TILEMAP_H

#include "Model/Core/Vector2.h"
#include "Model/World/WorldType.h"

#include <cstddef>
#include <string>
#include <vector>

namespace model {

// One enemy the level author placed, as read off the map. The map is the only source of
// initial enemy placement — nothing in the game code positions an enemy.
struct SpawnPoint {
    int id = 0;             // enemy id, matching EnemyFactory's enumeration
    std::size_t row = 0;    // grid row as stored (row 0 is the TOP line of the file)
    std::size_t column = 0;
};

class TileMap {
public:
    static constexpr std::size_t Rows = 16;
    static constexpr unsigned int TileWidth = 16;
    static constexpr unsigned int TileHeight = 16;

    // Background/decorative symbols: rendered like any other tile but passable
    // (excluded from tile collision).
    static constexpr char CloudSymbol = 'O';
    static constexpr char SmallTreeSymbol = 'T';

    // The goal castle is painted into the map's completion zone from its dedicated
    // 21-tile sheet. Each symbol must not collide with any symbol the map or the
    // renderer already uses: 'G' (ground), 'O'/'T' (scenery), 'M'/'E'/'K'/'C'/'B'/'#'
    // (spawns) and 'P'/'Q'/'p'/'q' (pipes) are all taken, so the castle uses the
    // remaining letters of the alphabet (row-major over the 5x5 silhouette).
    //
    // NOTE: the 9th entry is 'r', not 'Q'. 'Q' is the pipe mouth's right-hand cell, and
    // both the castle painter and the tile renderer index THIS array, so a shared symbol
    // would have made one of them draw the other's artwork.
    static constexpr std::size_t CastleTiles = 21;
    static constexpr char CastleSymbols[CastleTiles] =
        {'A', 'D', 'F', 'H', 'I', 'J', 'L', 'N', 'r', 'R', 'S',
         'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd'};

    static bool isCastleSymbol(char symbol);

    void loadFromFile(const std::string& filePath);

    // Load a pre-split grid (no header/metadata lines) of exactly Rows lines. Used by
    // Level to assemble the per-area grids it parsed out of a multi-area map file.
    void loadFromLines(const std::vector<std::string>& rows);

    // Enemy placements found in the map, in file order. The digits themselves are stripped
    // to empty tiles during load, so a spawn marker is never solid ground.
    const std::vector<SpawnPoint>& getSpawnPoints() const;

    // World-space top-left corner of a grid cell. Rows are stored top-down in the file,
    // so this is the one place that flip is written down.
    static Vector2 tileOrigin(std::size_t row, std::size_t column);

    // Append empty columns for the procedural level-completion zone (flagpole +
    // castle). Every new column mirrors the leftmost column's ground symbol ('G') so
    // the floor strip carries across the bonus area; everything else pads as air.
    void padRight(std::size_t extraColumns);

    // Rewrite one cell (used by the controller to paint the castle into the grid).
    void setTile(std::size_t row, std::size_t column, char symbol);

    char getTile(std::size_t row, std::size_t column) const;
    std::size_t getRows() const;
    std::size_t getColumns() const;

    // Optional metadata header (lines starting with ';' before the grid rows).
    const std::string& getLevelName() const;
    WorldType getWorldType() const;
    const std::string& getNextMapPath() const;
    bool hasNextMap() const;

    static bool isSolidTile(char symbol);
    // True for the four pipe cells ('P'/'Q' mouth, 'p'/'q' shaft). Pipes are solid terrain
    // so that enemies collide with them, not just the player.
    static bool isPipeSymbol(char symbol);

private:
    void parseHeader(const std::string& line);

    std::vector<std::vector<char>> tiles;
    std::vector<SpawnPoint> spawnPoints;
    std::size_t columns = 0;  // map width in tiles, read from the file

    std::string levelName;
    WorldType worldType = WorldType::Overworld;
    std::string nextMapPath;
};

}

#endif
