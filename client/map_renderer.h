#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "../common/DTOs.h"

#include "texture_cache.h"


// ── Datos de un tile del mapa ─────────────────────────────────
struct TileData {
    TileType floor = TileType::GRASS;
    bool blocked = false;
    uint8_t variant = 0;  // 0, 1 o 2 para grass
    ObstacleType obstacleType = ObstacleType::NONE;
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
    void renderObstacles(const GameMap& map, float camX, float camY);


    void renderPlayer(const Player& player, float camX, float camY);
    void renderWeapon(const Player& player, float camX, float camY);
    void renderHead(const Player& player, float camX, float camY);

    // Renderiza los NPCs estáticos de ciudad (tiles con ObstacleType::NPC_*)
    void renderCityNpcs(const GameMap& map, float camX, float camY);

    // Renderiza una criatura NPC dinámica (araña, esqueleto, etc.)
    void renderNpcEntity(const NpcEntity& npc, float camX, float camY);

    // Renderiza items tirados en el piso, encima de los tiles pero debajo de entidades.
    void renderDroppedItems(const std::vector<DroppedItem>& items, float camX, float camY);

private:
    SDL2pp::Renderer& renderer;
    TextureCache& cache;

    void drawTile(const TileData& tile, int screenX, int screenY);

    std::string get_path(int skin);
};
