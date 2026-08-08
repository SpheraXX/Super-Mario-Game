#ifndef MODEL_TILEMAP_H
#define MODEL_TILEMAP_H

#include "Model/World/WorldType.h"

#include <cstddef>
#include <string>
#include <vector>

namespace model {

class TileMap {
public:
    static constexpr std::size_t Rows = 16;
    static constexpr unsigned int TileWidth = 32;
    static constexpr unsigned int TileHeight = 32;

    // Background/decorative symbols: rendered like any other tile but passable
    // (excluded from tile collision).
    static constexpr char CloudSymbol = 'O';
    static constexpr char SmallTreeSymbol = 'T';

    void loadFromFile(const std::string& filePath);

    // Load a pre-split grid (no header/metadata lines) of exactly Rows lines. Used by
    // Level to assemble the per-area grids it parsed out of a multi-area map file.
    void loadFromLines(const std::vector<std::string>& rows);

    // Append empty columns for the procedural level-completion zone (flagpole +
    // castle). Every new column mirrors the leftmost column's ground symbol ('G') so
    // the floor strip carries across the bonus area; everything else pads as air.
    void padRight(std::size_t extraColumns);

    char getTile(std::size_t row, std::size_t column) const;
    std::size_t getRows() const;
    std::size_t getColumns() const;

    // Optional metadata header (lines starting with ';' before the grid rows).
    const std::string& getLevelName() const;
    WorldType getWorldType() const;
    const std::string& getNextMapPath() const;
    bool hasNextMap() const;

    static bool isSolidTile(char symbol);

private:
    void parseHeader(const std::string& line);

    std::vector<std::vector<char>> tiles;
    std::size_t columns = 0;  // map width in tiles, read from the file

    std::string levelName;
    WorldType worldType = WorldType::Overworld;
    std::string nextMapPath;
};

}

#endif
