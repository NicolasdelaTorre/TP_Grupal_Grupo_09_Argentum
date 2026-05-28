#pragma once

#include <SDL2pp/SDL2pp.hh>
#include <vector>
#include <string>
#include <unordered_map>
#include "texture_cache.h"
#include "../common/DTOs.h"



// ── Datos de un tile del mapa ─────────────────────────────────
struct TileData {
    TileType floor   = TileType::DIRT;
    bool     blocked = false;
    uint8_t  variant = 0;  // 0, 1 o 2 para grass
};

// ── Mapa ──────────────────────────────────────────────────────
struct GameMap {
    int width  = 0;
    int height = 0;
    std::vector<TileData> tiles;

    TileData& at(int x, int y)             { return tiles[y * width + x]; }
    const TileData& at(int x, int y) const { return tiles[y * width + x]; }

    bool inBounds(int x, int y) const {
        return x >= 0 && y >= 0 && x < width && y < height;
    }
};



class MapRenderer {
public:

    MapRenderer(SDL2pp::Renderer& renderer, TextureCache& cache);
  

    void render(const GameMap& map, float camX, float camY);
        

    void renderPlayer(const Player& player, float camX, float camY);

    void renderWeapon(const Player& player, float camX, float camY);

    void renderHead(const Player& player, float camX, float camY);

private:
    SDL2pp::Renderer& renderer;
    TextureCache&     cache;

    void drawTile(const TileData& tile, int screenX, int screenY);
};

// ── Mapa hardcodeado ──────────────────────────────────────────
inline GameMap makeTestMap() {
    GameMap map;
    map.width  = 200;
    map.height = 200;
    map.tiles.resize(map.width * map.height);

    for (int y = 0; y < map.height; y++) {
        for (int x = 0; x < map.width; x++) {
            TileData& tile = map.at(x, y);
            if (x == 0 || y == 0 || x == map.width-1 || y == map.height-1) {
                tile.floor   = TileType::WATER;
                tile.blocked = true;
            } else if (x >= 5 && x <= 5) {
                if(y == 5){
                    tile.floor   = TileType::DIRT;
                    tile.blocked = true;
                }
                
                
            } else {
                tile.floor   = TileType::DIRT;
                tile.variant = (x * 7 + y * 13) % 3;
            }
        }
    }
    return map;
}