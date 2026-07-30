#ifndef MODEL_TILEMAP_H
#define MODEL_TILEMAP_H

#include <cstddef>
#include <string>
#include <vector>

namespace model {

class TileMap {
public:
    static constexpr std::size_t Rows = 16;
    static constexpr std::size_t Columns = 32;
    static constexpr unsigned int TileWidth = 32;
    static constexpr unsigned int TileHeight = 32;

    void loadFromFile(const std::string& filePath);

    char getTile(std::size_t row, std::size_t column) const;
    std::size_t getRows() const;
    std::size_t getColumns() const;

private:
    std::vector<std::vector<char>> tiles;
};

}

#endif
