#ifndef MODEL_TILEMAP_H
#define MODEL_TILEMAP_H

#include "Model/Core/Vector2.h"

#include <cstddef>
#include <string>
#include <vector>

namespace model {

// One enemy the level author placed, as read off the map. The map is the only source of
// initial enemy placement — nothing in the game code positions an enemy.
struct SpawnPoint {
    int id = 0;             // enemy id, matching EnemyFactory's enumeration
    std::size_t row = 0;    // tile row, 0 = bottom of the world
    std::size_t column = 0;
};

class TileMap {
public:
    static constexpr std::size_t Rows = 16;
    static constexpr unsigned int TileWidth = 32;
    static constexpr unsigned int TileHeight = 32;

    // Background/decorative symbols: rendered like any other tile but passable
    // (excluded from tile collision).
    static constexpr char CloudSymbol = 'O';
    static constexpr char SmallTreeSymbol = 'T';

    // Coin blocks: a reward-carrying block (see CoinBlock). Treated like enemy markers —
    // stripped to empty tiles at load and re-created as entities by the state.
    static constexpr char CoinBlockSymbol = 'C';

    void loadFromFile(const std::string& filePath);

    char getTile(std::size_t row, std::size_t column) const;
    std::size_t getRows() const;
    std::size_t getColumns() const;

    // Enemy placements found in the map, in file order. The digits themselves are stripped
    // to empty tiles during load, so a spawn marker is never solid ground.
    const std::vector<SpawnPoint>& getSpawnPoints() const;

    // Coin-block placements found in the map (symbol 'C'), also in file order and likewise
    // stripped to empty tiles. The level author decides where blocks live, same as enemies.
    const std::vector<SpawnPoint>& getCoinBlockSpawns() const;

    // World-space top-left corner of a tile. Rows are stored bottom-up (row 0 is the
    // ground line), so this is the one place that flip is written down.
    static Vector2 tileOrigin(std::size_t row, std::size_t column);

    static bool isSolidTile(char symbol);

private:
    std::vector<std::vector<char>> tiles;
    std::vector<SpawnPoint> spawnPoints;
    std::vector<SpawnPoint> coinBlockSpawns;
    std::size_t columns = 0;  // map width in tiles, read from the file
};

}

#endif
