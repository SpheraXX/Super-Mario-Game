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

    std::vector<std::string> rows;
    rows.push_back(line);
    for (std::size_t row = 1; row < Rows; ++row) {
        if (!std::getline(input, line)) {
            throw std::runtime_error("Map file has fewer than 16 rows: " + filePath);
        }
        rows.push_back(line);
    }
    loadFromLines(rows);
}

void TileMap::loadFromLines(const std::vector<std::string>& rows) {
    if (rows.size() < Rows) {
        throw std::runtime_error("Area grid has fewer than 16 rows");
    }

    const std::size_t width = rows[0].size();
    if (width == 0) {
        throw std::runtime_error("Area grid has an empty first row");
    }
    columns = width;
    tiles.assign(Rows, std::vector<char>(columns, '.'));
    spawnPoints.clear();

    for (std::size_t row = 0; row < Rows; ++row) {
        if (rows[row].size() < columns) {
            throw std::runtime_error("Area grid row is shorter than the first row");
        }
        for (std::size_t column = 0; column < columns; ++column) {
            const char symbol = rows[row][column];
            // Digits are enemy markers, not terrain: record where the enemy goes and leave
            // empty space behind, so the marker cannot double as a solid tile.
            if (symbol >= '0' && symbol <= '9') {
                spawnPoints.push_back({symbol - '0', row, column});
                tiles[row][column] = '.';
            } else {
                tiles[row][column] = symbol;
            }
        }
    }
}

const std::vector<SpawnPoint>& TileMap::getSpawnPoints() const {
    return spawnPoints;
}

Vector2 TileMap::tileOrigin(std::size_t row, std::size_t column) {
    return {static_cast<float>(column) * TileWidth,
            static_cast<float>(Rows - 1 - row) * TileHeight};
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

void TileMap::setTile(std::size_t row, std::size_t column, char symbol) {
    tiles.at(row).at(column) = symbol;
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
    // scenery ('O', 'T') is decorative. The painted castle is solid.
    return symbol == 'G' || isCastleSymbol(symbol);
}

bool TileMap::isCastleSymbol(char symbol) {
    for (const char candidate : CastleSymbols) {
        if (candidate == symbol) {
            return true;
        }
    }
    return false;
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
