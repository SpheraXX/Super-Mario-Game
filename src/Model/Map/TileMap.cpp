#include "Model/Map/TileMap.h"

#include <fstream>
#include <stdexcept>

namespace model {

void TileMap::loadFromFile(const std::string& filePath) {
    std::ifstream input(filePath);
    if (!input) {
        throw std::runtime_error("Could not open map file: " + filePath);
    }

    std::string line;
    for (std::size_t row = 0; row < Rows; ++row) {
        if (!std::getline(input, line)) {
            throw std::runtime_error("Map file has fewer than 16 rows: " + filePath);
        }

        if (row == 0) {
            if (line.empty()) {
                throw std::runtime_error("Map file has an empty first row: " + filePath);
            }
            columns = line.size();
            tiles.assign(Rows, std::vector<char>(columns, '.'));
        }

        if (line.size() < columns) {
            throw std::runtime_error("Map row is shorter than the first row: " + filePath);
        }

        for (std::size_t column = 0; column < columns; ++column) {
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
    return columns;
}

bool TileMap::isSolidTile(char symbol) {
    return symbol != '.' && symbol != CloudSymbol && symbol != SmallTreeSymbol;
}

}
