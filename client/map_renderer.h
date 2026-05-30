#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "../common/DTOs.h"

#include "texture_cache.h"


// ── Datos de un tile del mapa ─────────────────────────────────
struct TileData {
    TileType floor = TileType::DIRT;
    bool blocked = false;
    uint8_t variant = 0;  // 0, 1 o 2 para grass
};

// ── Mapa ──────────────────────────────────────────────────────
struct GameMap {
    int width = 0;
    int height = 0;
    std::vector<TileData> tiles;

    TileData& at(int x, int y) { return tiles[y * width + x]; }
    const TileData& at(int x, int y) const { return tiles[y * width + x]; }

    bool inBounds(int x, int y) const { return x >= 0 && y >= 0 && x < width && y < height; }
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
    TextureCache& cache;

    void drawTile(const TileData& tile, int screenX, int screenY);
};
