#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "../common/DTOs.h"

#include "render_constants.h"
#include "texture_cache.h"
#include "visual_types.h"


// ── Proyectil de flecha (puramente visual, lado cliente) ──────
struct ArrowProjectile {
    float x, y;    // posición en tiles (centro del sprite)
    float vx, vy;  // velocidad en tiles/seg
    float lifetime;
    int arrowType;  // 0–8, columna en Flechas.png
};

// ── Datos de un tile del mapa ─────────────────────────────────
struct TileData {
    TileCode floor = TileCode::GRASS;
    bool blocked = false;
    uint8_t variant = 0;  // 0, 1 o 2 para grass
    ObstacleCode obstacleType = ObstacleCode::NONE;
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


    void renderPlayer(const Player_& player, float camX, float camY);
    void renderWeapon(const Player_& player, float camX, float camY);
    void renderShield(const Player_& player, float camX, float camY);
    void renderHead(const Player_& player, float camX, float camY);
    void renderHelmet(const Player_& player, float camX, float camY);

    // Renderiza los NPCs estáticos de ciudad (tiles con ObstacleCode::NPC_*)
    void renderCityNpcs(const GameMap& map, float camX, float camY);

    // Renderiza una criatura NPC dinámica (araña, esqueleto, etc.)
    void renderNpcEntity(const NpcEntity& npc, float camX, float camY);

    // Renderiza items tirados en el piso, encima de los tiles pero debajo de entidades.
    void renderDroppedItems(const std::vector<DroppedItem>& items, float camX, float camY);

    // Renderiza un efecto de sangre centrado en la posición de tile (x, y).
    // texIndex: 0–4 → Sangre_1.png … Sangre_5.png
    // alpha: 0–255 para fade-out
    void renderBlood(float x, float y, int texIndex, Uint8 alpha, float camX, float camY);

    // Renderiza flechas en vuelo (Armas/Flechas.png), rotadas según su dirección.
    void renderArrows(const std::vector<ArrowProjectile>& arrows, float camX, float camY);

private:
    SDL2pp::Renderer& renderer;
    TextureCache& cache;

    void drawTile(const TileData& tile, int screenX, int screenY);

    std::string get_path(int skin);
};
