#include "map_renderer.h"

#include <algorithm>
#include <cmath>

namespace {

static constexpr int ITEM_CELL = 32;       // each cell in all three sheets is 32×32 px
static constexpr int ITEM_COLS_512  = 16;  // 512px sheets → 16 columns
static constexpr int ITEM_COLS_1024 = 32;  // 1024px sheet  → 32 columns
static constexpr int ITEM_DRAW_SIZE = 40;  // render size on screen (scaled up from 32px)

const char* itemSheetPath(uint8_t sheetId) {
    switch (sheetId) {
        case 0:  return "/Pantallas/Items_recolectables.png";
        case 1:  return "/Pantallas/Items_recolectables_2.png";
        case 2:  return "/Pantallas/Items_recolectables_3.png";
        default: return nullptr;
    }
}

// Devuelve el path del sprite para NPCs de ciudad (obstáculos fijos en el mapa).
// Retorna nullptr si el tipo no es un NPC de ciudad.
const char* cityNpcTexturePath(ObstacleType type) {
    switch (type) {
        case ObstacleType::NPC:
        case ObstacleType::NPC_PRIEST:
        case ObstacleType::NPC_MERCHANT:
        case ObstacleType::NPC_BANKER:
            return "/Skins/NPC/Sacerdote.png";
        default:
            return nullptr;
    }
}

// Devuelve el path del sprite para criaturas NPC dinámicas.
const char* npcEntityTexturePath(NpcType type) {
    switch (type) {
        case NpcType::SPIDER:   return "/Skins/NPC/araña.png";
        case NpcType::SKELETON: return "/Skins/NPC/Esqueleto.png";
        case NpcType::ZOMBIE:   return "/Skins/NPC/Goblin.png";
        case NpcType::GOBLIN:   return "/Skins/NPC/Goblin.png";
        case NpcType::ORC:      return "/Skins/NPC/Orc.png";
        case NpcType::GOLEM:    return "/Skins/NPC/Golem.png";
        default:                return "/Skins/NPC/araña.png";
    }
}

const char* obstacleTexturePath(ObstacleType type) {
    switch (type) {
        case ObstacleType::ROCK:       return "/Obstaculos/roca_01_ajustada.png";
        case ObstacleType::ROCK_SMALL: return "/Obstaculos/roca_03_ajustada.png";
        //case ObstacleType::ROCK_LARGE: return "/Obstaculos/roca_01_ajustada.png";
        case ObstacleType::LAMP:       return "/Obstaculos/lampara_corregida.png";
        case ObstacleType::WOOD:       return "/Obstaculos/maderas_apiladas_corregida.png";
        case ObstacleType::CART:       return "/Obstaculos/segunda_carretilla_primera_fila.png";
        case ObstacleType::MILL:       return "/Obstaculos/molino_recortado.png";
        case ObstacleType::CACTUS:     return "/Obstaculos/cactus_arriba_derecha_128x128.png";
        default:                       return nullptr;
    }
}


}  // namespace


MapRenderer::MapRenderer(SDL2pp::Renderer& renderer, TextureCache& cache):
        renderer(renderer), cache(cache) {}

void MapRenderer::render(const GameMap& map, float camX, float camY) {
    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer.Get(), &screenW, &screenH);

    int startX = std::max(0, (int)(camX / TILE_SIZE));
    int startY = std::max(0, (int)(camY / TILE_SIZE));
    int endX = std::min(map.width, startX + screenW / TILE_SIZE + 2);
    int endY = std::min(map.height, startY + screenH / TILE_SIZE + 2);

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            int screenX = (int)(x * TILE_SIZE - camX);
            int screenY = (int)(y * TILE_SIZE - camY);
            drawTile(map.at(x, y), screenX, screenY);
        }
    }

    renderObstacles(map, camX, camY);
}


// Source crop rect for each obstacle image, excluding the drop-shadow overhang
// that extends past the rock body to the lower-right.
// Returns NullOpt to use the full image.
SDL2pp::Optional<SDL2pp::Rect> obstacleSourceCrop(ObstacleType type) {
    switch (type) {
        // roca_01_ajustada.png (437x327): rock body ends ~col 350, row 315
        case ObstacleType::ROCK:       return SDL2pp::Rect(0, 0, 350, 315);
        // roca_03_ajustada.png (168x134): rock body ends ~col 140, row 120
        case ObstacleType::ROCK_SMALL: return SDL2pp::Rect(0, 0, 140, 120);
        default:                       return SDL2pp::NullOpt;
    }
}


void MapRenderer::renderObstacles(const GameMap& map, float camX, float camY) {
    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer.Get(), &screenW, &screenH);

    int startX = std::max(0, (int)(camX / TILE_SIZE));
    int startY = std::max(0, (int)(camY / TILE_SIZE));
    int endX = std::min(map.width, startX + screenW / TILE_SIZE + 2);
    int endY = std::min(map.height, startY + screenH / TILE_SIZE + 2);

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            const TileData& tile = map.at(x, y);
            if (tile.obstacleType == ObstacleType::NONE)
                continue;

            // Solo renderizamos desde la celda ancla (esquina superior-izquierda del grupo).
            const bool leftSame =
                    (x > 0 && map.at(x - 1, y).obstacleType == tile.obstacleType);
            const bool aboveSame =
                    (y > 0 && map.at(x, y - 1).obstacleType == tile.obstacleType);
            if (leftSame || aboveSame)
                continue;

            // Medir el ancho del grupo escaneando hacia la derecha.
            int w = 1;
            while (x + w < map.width && map.at(x + w, y).obstacleType == tile.obstacleType)
                w++;

            // Medir el alto del grupo escaneando hacia abajo.
            int h = 1;
            while (y + h < map.height && map.at(x, y + h).obstacleType == tile.obstacleType)
                h++;

            const char* texPath = obstacleTexturePath(tile.obstacleType);
            if (!texPath)
                continue;

            const int screenX = (int)(x * TILE_SIZE - camX);
            const int screenY = (int)(y * TILE_SIZE - camY);
            SDL2pp::Rect dst(screenX, screenY, w * TILE_SIZE, h * TILE_SIZE);

            try {
               renderer.Copy(cache.get(texPath), obstacleSourceCrop(tile.obstacleType), dst);
            } catch (...) {
                // Textura no disponible — se ignora silenciosamente.
            }
        }
    }
}

void MapRenderer::renderPlayer(const Player& player, float camX, float camY) {
    int screenX = (int)(player.x * TILE_SIZE - camX) + TILE_SIZE / 2 - SPRITE_W / 2;
    int screenY = (int)(player.y * TILE_SIZE - camY) + TILE_SIZE / 2 - SPRITE_H / 2;

    int row = static_cast<int>(player.dir);
    int col = player.moving ? player.animFrame : 0;

    if (player.killed) {
        int ghostRow = row;
        if (player.dir == Direction::LEFT)       ghostRow = static_cast<int>(Direction::RIGHT);
        else if (player.dir == Direction::RIGHT) ghostRow = static_cast<int>(Direction::LEFT);
        SDL2pp::Rect src(col * PHANTOM_SPRITE_W, ghostRow * PHANTOM_SPRITE_H, PHANTOM_SPRITE_W, PHANTOM_SPRITE_H);
        SDL2pp::Rect dst(screenX, screenY, PHANTOM_SPRITE_W, PHANTOM_SPRITE_H);
        renderer.Copy(cache.get("/Skins/NPC/Fantasma.png"), src, dst);
        return;
    }

    

    SDL2pp::Rect src(col * SPRITE_W, row * SPRITE_H, SPRITE_W, SPRITE_H);
    SDL2pp::Rect dst(screenX, screenY, SPRITE_W, SPRITE_H);

    renderer.Copy(cache.get(get_path(player.skin)), src, dst);
}

std::string MapRenderer::get_path(int skin) {
    switch (skin) {
        case 0: return "/Skins/Caballero_blanco.png";
        case 1: return "/Skins/Gladiador_violeta.png";
        case 2: return "/Skins/Gladiador_azul.png";
        case 3: return "/Skins/Hechicero.png";
        default: return "/Skins/skin_default.png";
    }
}

void MapRenderer::drawTile(const TileData& tile, int screenX, int screenY) {
    SDL2pp::Rect dst(screenX, screenY, TILE_SIZE, TILE_SIZE);
    SDL2pp::Rect src(0, 0, TILE_SIZE, TILE_SIZE);

    switch (tile.floor) {
        case TileType::GRASS: {
            static constexpr int VAR_W = 170;
            static constexpr int VAR_H = 128;
            SDL2pp::Rect varSrc(tile.variant * VAR_W, 0, VAR_W, VAR_H);
            renderer.Copy(cache.get("/Mapa/Tiles_pasto.png"), varSrc, dst);
            break;
        }
        case TileType::WATER:
            renderer.Copy(cache.get("/Mapa/Tiles_agua.png"), src, dst);
            break;
        case TileType::DIRT:
            renderer.Copy(cache.get("/Mapa/Tile_tierra.png"), src, dst);
            break;
        case TileType::SAND:
            renderer.Copy(cache.get("/Mapa/Tiles_arena.png"), src, dst);
            break;
        case TileType::INTERIOR:
            renderer.Copy(cache.get("/Mapa/Tiles_interiores.png"), src, dst);
            break;
    }
}

void MapRenderer::renderWeapon(const Player& player, float camX, float camY) {
    if (player.killed || player.weaponId < 0)
        return;

    static const char* weaponFiles[] = {"/Armas/Espada.png", "/Armas/Daga.png", "/Armas/Arco.png",
                                        "/Armas/Baculo.png"};
    if (player.weaponId >= 4)
        return;

    int row = static_cast<int>(player.dir);
    int col = player.moving ? player.animFrame : 0;

    SDL2pp::Rect src(col * SPRITE_W, row * SPRITE_H, SPRITE_W, SPRITE_H);

    // Misma posición base que el cuerpo
    int screenX = (int)(player.x * TILE_SIZE - camX) + TILE_SIZE / 2 - SPRITE_W / 2;
    int screenY = (int)(player.y * TILE_SIZE - camY) + TILE_SIZE / 2 - SPRITE_H / 2;
    SDL2pp::Rect dst(screenX, screenY, SPRITE_W, SPRITE_H);

    renderer.Copy(cache.get(weaponFiles[player.weaponId]), src, dst);
}

void MapRenderer::renderShield(const Player& player, float camX, float camY) {
    if (player.killed || player.shieldId < 0 || player.dir == Direction::UP)
        return;

    static const char* shieldFiles[] = {"/Armas/Escudo.png"};
    if (player.shieldId >= static_cast<int>(std::size(shieldFiles)))
        return;

    int row = static_cast<int>(player.dir);
    int col = player.moving ? player.animFrame : 0;

    SDL2pp::Rect src(col * SPRITE_W, row * SPRITE_H, SPRITE_W, SPRITE_H);

    int screenX = (int)(player.x * TILE_SIZE - camX) + TILE_SIZE / 2 - SPRITE_W / 2;
    int screenY = (int)(player.y * TILE_SIZE - camY) + TILE_SIZE / 2 - SPRITE_H / 2;
    SDL2pp::Rect dst(screenX, screenY, SPRITE_W, SPRITE_H);

    renderer.Copy(cache.get(shieldFiles[player.shieldId]), src, dst);
}

void MapRenderer::renderHead(const Player& player, float camX, float camY) {
    if (player.killed)
        return;
    // Misma posición base que el cuerpo
    int screenX = (int)(player.x * TILE_SIZE - camX) + TILE_SIZE / 2 - SPRITE_W / 2;
    int screenY = (int)(player.y * TILE_SIZE - camY) + TILE_SIZE / 2 - SPRITE_H / 2;

    // La cabeza va centrada en la parte superior del cuerpo
    // Celda de cabeza: 27px ancho, 64px alto
    static constexpr int HEAD_CELL_W = 27;
    static constexpr int HEAD_CELL_H = 64;

    int col = player.headId;                 // qué cabeza
    int row = static_cast<int>(player.dir);  // dirección = fila

    SDL2pp::Rect src(col * HEAD_CELL_W, row * HEAD_CELL_H, HEAD_CELL_W, HEAD_CELL_H);

    // Centrar la cabeza horizontalmente sobre el cuerpo
    int headX = screenX + SPRITE_W / 2 - HEAD_CELL_W / 2;
    int headY = screenY - HEAD_CELL_H / 4 - 3;  // alinear con la parte superior del cuerpo

    SDL2pp::Rect dst(headX, headY, HEAD_CELL_W, HEAD_CELL_H);

    renderer.Copy(cache.get("/Skins/Cabezas.png"), src, dst);
}

void MapRenderer::renderHelmet(const Player& player, float camX, float camY) {
    if (player.killed || player.helmetId < 0)
        return;

    static constexpr int HELMET_CELL_W   = 27;  // 46
    static constexpr int HELMET_CELL_H   = 64;  // 256
    static constexpr int HEAD_CELL_W     = 27;
    static constexpr int HEAD_CELL_H     = 64;

    int col = player.helmetId;
    int row = static_cast<int>(player.dir);  // DOWN=0, UP=1, LEFT=2, RIGHT=3

    SDL2pp::Rect src(col * HELMET_CELL_W, row * HELMET_CELL_H, HELMET_CELL_W, HELMET_CELL_H);

    int screenX = (int)(player.x * TILE_SIZE - camX) + TILE_SIZE / 2 - SPRITE_W / 2;
    int screenY = (int)(player.y * TILE_SIZE - camY) + TILE_SIZE / 2 - SPRITE_H / 2;
    int headX   = screenX + SPRITE_W / 2 - HEAD_CELL_W / 2;
    int headY   = screenY - HEAD_CELL_H / 4 - 3;

    SDL2pp::Rect dst(headX, headY, HEAD_CELL_W, HEAD_CELL_H);

    try {
        renderer.Copy(cache.get("/Skins/Gorros.png"), src, dst);
    } catch (...) {}
}

void MapRenderer::renderCityNpcs(const GameMap& map, float camX, float camY) {
    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer.Get(), &screenW, &screenH);

    int startX = std::max(0, (int)(camX / TILE_SIZE));
    int startY = std::max(0, (int)(camY / TILE_SIZE));
    int endX = std::min(map.width, startX + screenW / TILE_SIZE + 2);
    int endY = std::min(map.height, startY + screenH / TILE_SIZE + 2);

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            const char* tex = cityNpcTexturePath(map.at(x, y).obstacleType);
            if (!tex)
                continue;

            // Centrado en el tile, sprite hacia abajo (row 0), frame idle (col 0)
            int screenX = (int)(x * TILE_SIZE - camX) + TILE_SIZE / 2 - PHANTOM_SPRITE_W / 2;
            int screenY = (int)(y * TILE_SIZE - camY) + TILE_SIZE / 2 - PHANTOM_SPRITE_H / 2;
            SDL2pp::Rect src(0, 0, PHANTOM_SPRITE_W, PHANTOM_SPRITE_H);
            SDL2pp::Rect dst(screenX, screenY, PHANTOM_SPRITE_W, PHANTOM_SPRITE_H);
            try {
                renderer.Copy(cache.get(tex), src, dst);
            } catch (...) {}
        }
    }
}

void MapRenderer::renderDroppedItems(const std::vector<DroppedItem>& items, float camX, float camY) {
    for (const auto& item : items) {
        const char* tex = itemSheetPath(item.sheetId);
        if (!tex)
            continue;

        int cols = (item.sheetId == 2) ? ITEM_COLS_1024 : ITEM_COLS_512;
        int row = item.itemId / cols;
        int col = item.itemId % cols;
        SDL2pp::Rect src(col * ITEM_CELL, row * ITEM_CELL, ITEM_CELL, ITEM_CELL);

        int screenX = (int)(item.x * TILE_SIZE - camX) + TILE_SIZE / 2 - ITEM_DRAW_SIZE / 2;
        int screenY = (int)(item.y * TILE_SIZE - camY) + TILE_SIZE / 2 - ITEM_DRAW_SIZE / 2;
        SDL2pp::Rect dst(screenX, screenY, ITEM_DRAW_SIZE, ITEM_DRAW_SIZE);

        try {
            renderer.Copy(cache.get(tex), src, dst);
        } catch (...) {}
    }
}

void MapRenderer::renderBlood(float x, float y, int texIndex, Uint8 alpha, float camX, float camY) {
    static constexpr int BLOOD_DRAW_SIZE = 32;
    static const char* bloodFiles[] = {
        "/Skins/Sangre_1.png", "/Skins/Sangre_2.png", "/Skins/Sangre_3.png",
        "/Skins/Sangre_4.png", "/Skins/Sangre_5.png"
    };
    if (texIndex < 0 || texIndex >= 5)
        return;

    int screenX = (int)(x * TILE_SIZE - camX) + TILE_SIZE / 2 - BLOOD_DRAW_SIZE / 2;
    int screenY = (int)(y * TILE_SIZE - camY) + TILE_SIZE / 2 - BLOOD_DRAW_SIZE / 2;
    SDL2pp::Rect dst(screenX, screenY, BLOOD_DRAW_SIZE, BLOOD_DRAW_SIZE);

    try {
        SDL2pp::Texture& tex = cache.get(bloodFiles[texIndex]);
        tex.SetAlphaMod(alpha);
        renderer.Copy(tex, SDL2pp::NullOpt, dst);
        tex.SetAlphaMod(255);
    } catch (...) {}
}

void MapRenderer::renderArrows(const std::vector<ArrowProjectile>& arrows, float camX, float camY) {
    // Flechas.png: 512×512, 9 arrow types in a single row at the top.
    // Each cell is 512/9 ≈ 56 px wide. Sprites point upper-right (45° CW from north),
    // so the SDL2 rotation formula is: atan2(vx, -vy) * 180/π − 45.
    static constexpr int ARROW_COLS      = 9;
    static constexpr int ARROW_CELL_W    = 32;
    static constexpr int ARROW_DRAW_SIZE = 32;

    for (const auto& arrow : arrows) {
        int screenX = (int)(arrow.x * TILE_SIZE - camX) - ARROW_DRAW_SIZE / 2;
        int screenY = (int)(arrow.y * TILE_SIZE - camY) - ARROW_DRAW_SIZE / 2;

        int col = std::max(0, std::min(arrow.arrowType, ARROW_COLS - 1));
        SDL_Rect src = {col * ARROW_CELL_W, 0, ARROW_CELL_W, ARROW_CELL_W};
        SDL_Rect dst = {screenX, screenY, ARROW_DRAW_SIZE, ARROW_DRAW_SIZE};

        double angle_deg = std::atan2(arrow.vx, -arrow.vy) * 180.0 / M_PI - 45.0;

        try {
            SDL_RenderCopyEx(renderer.Get(), cache.get("/Armas/Flechas.png").Get(),
                             &src, &dst, angle_deg, nullptr, SDL_FLIP_NONE);
        } catch (...) {}
    }
}

void MapRenderer::renderNpcEntity(const NpcEntity& npc, float camX, float camY) {
    const char* tex = npcEntityTexturePath(npc.type);
    if (!tex)
        return;

    int screenX = (int)(npc.x * TILE_SIZE - camX) + TILE_SIZE / 2 - PHANTOM_SPRITE_W / 2;
    int screenY = (int)(npc.y * TILE_SIZE - camY) + TILE_SIZE / 2 - PHANTOM_SPRITE_H / 2;

    int row = static_cast<int>(npc.dir);
    int col = npc.moving ? npc.animFrame : 0;

    SDL2pp::Rect src(col * PHANTOM_SPRITE_W, row * PHANTOM_SPRITE_H, PHANTOM_SPRITE_W, PHANTOM_SPRITE_H);
    SDL2pp::Rect dst(screenX, screenY, PHANTOM_SPRITE_W, PHANTOM_SPRITE_H);
    try {
        renderer.Copy(cache.get(tex), src, dst);
    } catch (...) {}
}
