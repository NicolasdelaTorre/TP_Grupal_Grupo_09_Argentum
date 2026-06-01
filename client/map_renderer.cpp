#include "map_renderer.h"

#include <algorithm>

namespace {

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
    // Centrado sobre el tile
    int screenX = (int)(player.x * TILE_SIZE - camX) + TILE_SIZE / 2 - SPRITE_W / 2;
    int screenY = (int)(player.y * TILE_SIZE - camY) + TILE_SIZE / 2 - SPRITE_H / 2;

    int row = static_cast<int>(player.dir);
    int col = player.moving ? player.animFrame : 0;

    SDL2pp::Rect src(col * SPRITE_W, row * SPRITE_H, SPRITE_W, SPRITE_H);
    SDL2pp::Rect dst(screenX, screenY, SPRITE_W, SPRITE_H);

    renderer.Copy(cache.get("/Skins/Caballero_blanco.png"), src, dst);
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
    if (player.weaponId < 0)
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

void MapRenderer::renderHead(const Player& player, float camX, float camY) {
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
