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
    // Static tile geometry: ground and pipes. 'C', 'B', 'M', 'E' and '#' spawn as entities
    // (blocks and characters) that manage their own collision, and scenery ('O', 'T') --
    // including the castle, which is a backdrop now that 'E' ends the level -- is
    // decorative.
    //
    // Pipes are terrain rather than entities. The entity pass only ever resolves the
    // PLAYER against solid entities (see CollisionManager::resolveEntityInteraction, which
    // returns early unless exactly one side is the player), so a pipe modelled purely as an
    // entity is invisible to enemies and they walk straight through it. As tiles they are
    // resolved by the tile pass, which runs for every Character. A Pipe entity is still
    // spawned for columns that carry a warp portal, since that needs the column linkage.
    // The stair block is terrain for the same reason: it is an unbreakable brick, so it
    // needs no entity of its own and every Character resolves against it in the tile pass.
    return isStandableTerrain(symbol) || isPipeSymbol(symbol);
}

bool TileMap::isStandableTerrain(char symbol) {
    // The chain (bridge deck) is here and not merely in isSolidTile because standing on it
    // is the whole point: it has to hold the player AND Bowser up until the axe cuts it,
    // and it stops doing so the moment ChainTrigger erases those cells, with no special
    // case anywhere. A firebar's marker cell is also the BLOCK the bar is mounted on —
    // ordinary footing in the original — which is why one symbol carries both.
    return symbol == 'G' || symbol == StairSymbol || symbol == ChainSymbol
           || symbol == FirebarSymbol;
}

bool TileMap::isPipeSymbol(char symbol) {
    // 'P'/'Q' are the mouth's left/right cells, 'p'/'q' the shaft below them.
    return symbol == 'P' || symbol == 'Q' || symbol == 'p' || symbol == 'q';
}

bool TileMap::isCastleSymbol(char symbol) {
    return symbol == CastleUpperSymbol || symbol == CastleLowerSymbol;
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
