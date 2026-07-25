#include "Model/TileMap.h"

#include <fstream>
#include <stdexcept>

namespace model {

void TileMap::loadFromFile(const std::string& filePath) {
    std::ifstream input(filePath);
    if (!input) {
        throw std::runtime_error("Could not open map file: " + filePath);
    }

    tiles.assign(Rows, std::vector<char>(Columns, '.'));

    std::string line;
    for (std::size_t row = 0; row < Rows; ++row) {
        if (!std::getline(input, line)) {
            throw std::runtime_error("Map file has fewer than 16 rows: " + filePath);
        }

        if (line.size() < Columns) {
            throw std::runtime_error("Map row is shorter than 32 columns: " + filePath);
        }

        for (std::size_t column = 0; column < Columns; ++column) {
            tiles[row][column] = line[column];
        }
    }
}

char TileMap::getTile(std::size_t row, std::size_t column) const {
    return tiles.at(row).at(column);
}

std::size_t TileMap::getRows() const {
    return Rows;
}

std::size_t TileMap::getColumns() const {
    return Columns;
}

}
