#ifndef MODEL_TILEMAP_H
#define MODEL_TILEMAP_H

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

    char getTile(std::size_t row, std::size_t column) const;
    std::size_t getRows() const;
    std::size_t getColumns() const;

    static bool isSolidTile(char symbol);

private:
    std::vector<std::vector<char>> tiles;
    std::size_t columns = 0;  // map width in tiles, read from the file
};

}

#endif
