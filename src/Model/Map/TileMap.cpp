#include "Model/Map/TileMap.h"

#include <fstream>
#include <stdexcept>

namespace model {

namespace {
std::string trim(const std::string& value) {
    const std::size_t begin = value.find_first_not_of(' ');
    if (begin == std::string::npos) {
        return std::string();
    }
    const std::size_t end = value.find_last_not_of(' ');
    return value.substr(begin, end - begin + 1);
}
}

void TileMap::loadFromFile(const std::string& filePath) {
    std::ifstream input(filePath);
    if (!input) {
        throw std::runtime_error("Could not open map file: " + filePath);
    }

    // Optional metadata header: lines starting with ';' before the 16 grid rows.
    // (Semicolon, because '#' is already the brick tile.) Example:
    //   ; name=World 1-1
    //   ; world=underwater
    //   ; next=assets/maps/debug2.map
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line[0] == ';') {
            parseHeader(line);
            continue;
        }
        break;
    }

    if (line.empty()) {
        throw std::runtime_error("Map file has an empty first row: " + filePath);
    }
    columns = line.size();
    tiles.assign(Rows, std::vector<char>(columns, '.'));

    for (std::size_t row = 0; row < Rows; ++row) {
        if (row > 0) {
            if (!std::getline(input, line)) {
                throw std::runtime_error("Map file has fewer than 16 rows: " + filePath);
            }
        }

        if (line.size() < columns) {
            throw std::runtime_error("Map row is shorter than the first row: " + filePath);
        }

        for (std::size_t column = 0; column < columns; ++column) {
            tiles[row][column] = line[column];
        }
    }
}

void TileMap::padRight(std::size_t extraColumns) {
    if (extraColumns == 0 || columns == 0) {
        return;
    }

    for (std::size_t row = 0; row < Rows; ++row) {
        const char edge = tiles[row][0];
        // Mirror only the ground symbol from the leftmost column; everything else
        // becomes air so no spawn symbols or scenery leak into the bonus zone.
        tiles[row].resize(columns + extraColumns, edge == 'G' ? 'G' : '.');
    }
    columns += extraColumns;
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

const std::string& TileMap::getLevelName() const {
    return levelName;
}

WorldType TileMap::getWorldType() const {
    return worldType;
}

const std::string& TileMap::getNextMapPath() const {
    return nextMapPath;
}

bool TileMap::hasNextMap() const {
    return !nextMapPath.empty();
}

bool TileMap::isSolidTile(char symbol) {
    // Only the ground is static tile geometry now: 'C', 'B', 'M', 'E', 'K' and '#'
    // spawn as entities (blocks and characters) that manage their own collision, and
    // scenery ('O', 'T') is decorative.
    return symbol == 'G';
}

void TileMap::parseHeader(const std::string& line) {
    const std::size_t eq = line.find('=');
    if (eq == std::string::npos) {
        return;
    }

    const std::string key = trim(line.substr(1, eq - 1));
    const std::string value = trim(line.substr(eq + 1));
    if (key == "name" || key == "level") {
        levelName = value;
    } else if (key == "world") {
        worldType = worldTypeFromString(value);
    } else if (key == "next") {
        nextMapPath = value;
    }
}

}
