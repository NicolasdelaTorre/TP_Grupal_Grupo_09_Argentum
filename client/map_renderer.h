#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "texture_cache.h"

// ── Constantes ────────────────────────────────────────────────
static constexpr int TILE_SIZE = 32;  // tiles son 128x128
static constexpr int SPRITE_W = 27;   // frame del personaje
static constexpr int SPRITE_H = 49;
static constexpr int ANIM_FRAMES = 4;  // columnas del spritesheet

// ── Tipos de tile ─────────────────────────────────────────────
enum class TileType : uint8_t { GRASS = 0, WATER, DIRT };

// ── Dirección del personaje ───────────────────────────────────
enum class Direction : uint8_t { UP = 1, LEFT = 2, DOWN = 0, RIGHT = 3 };

// ── Datos de un tile del mapa ─────────────────────────────────
struct TileData {
    TileType floor = TileType::GRASS;
    bool blocked = false;
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

// ── Estado del jugador ────────────────────────────────────────
struct Player {
    float x = 5.0f, y = 5.0f;  // posición en tiles
    Direction dir = Direction::DOWN;
    bool moving = false;
    int animFrame = 0;
    float animTimer = 0.0f;
    int headId = 4;

    static constexpr float ANIM_SPEED = 0.15f;  // segundos por frame
    static constexpr float MOVE_SPEED = 4.0f;   // tiles por segundo
};


class MapRenderer {
public:
    MapRenderer(SDL2pp::Renderer& renderer, TextureCache& cache);


    void render(const GameMap& map, float camX, float camY);


    void renderPlayer(const Player& player, float camX, float camY);

    void renderHead(const Player& player, float camX, float camY);

private:
    SDL2pp::Renderer& renderer;
    TextureCache& cache;

    void drawTile(TileType type, int screenX, int screenY);
};

// ── Mapa hardcodeado ──────────────────────────────────────────
// Mapa chico (10x10) para testear out-of-bounds rápido contra el server.
// Se reemplaza por el mapa que envía el servidor en sprints siguientes.
inline GameMap makeTestMap() {
    GameMap map;
    map.width = 10;
    map.height = 10;
    map.tiles.resize(map.width * map.height);

    for (int y = 0; y < map.height; y++) {
        for (int x = 0; x < map.width; x++) {
            TileData& tile = map.at(x, y);
            if (x == 0 || y == 0 || x == map.width - 1 || y == map.height - 1) {
                // Borde bloqueado con agua
                tile.floor = TileType::WATER;
                tile.blocked = true;
            } else {
                tile.floor = TileType::GRASS;
            }
        }
    }
    return map;
}
