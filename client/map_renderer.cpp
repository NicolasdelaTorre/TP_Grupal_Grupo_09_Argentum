#include "map_renderer.h"


MapRenderer::MapRenderer(SDL2pp::Renderer& renderer, TextureCache& cache)
    : renderer(renderer), cache(cache) {}

void MapRenderer::render(const GameMap& map, float camX, float camY) {
    int screenW, screenH;
        SDL_GetRendererOutputSize(renderer.Get(), &screenW, &screenH);

        int startX = std::max(0, (int)(camX / TILE_SIZE));
        int startY = std::max(0, (int)(camY / TILE_SIZE));
        int endX   = std::min(map.width,  startX + screenW / TILE_SIZE + 2);
        int endY   = std::min(map.height, startY + screenH / TILE_SIZE + 2);

        for (int y = startY; y < endY; y++) {
            for (int x = startX; x < endX; x++) {
                int screenX = (int)(x * TILE_SIZE - camX);
                int screenY = (int)(y * TILE_SIZE - camY);
                drawTile(map.at(x, y).floor, screenX, screenY);
            }
        }
    }

void MapRenderer::renderPlayer(const Player& player, float camX, float camY) {
    // Centrado sobre el tile
    int screenX = (int)(player.x * TILE_SIZE - camX) + TILE_SIZE/2 - SPRITE_W/2;
    int screenY = (int)(player.y * TILE_SIZE - camY) + TILE_SIZE/2 - SPRITE_H/2;

    int row = static_cast<int>(player.dir);
    int col = player.moving ? player.animFrame : 0;

    SDL2pp::Rect src(col * SPRITE_W, row * SPRITE_H, SPRITE_W, SPRITE_H);
    SDL2pp::Rect dst(screenX, screenY, SPRITE_W, SPRITE_H);

    renderer.Copy(cache.get("1027.png"), src, dst);
}

void MapRenderer::drawTile(TileType type, int screenX, int screenY) {
    SDL2pp::Rect dst(screenX, screenY, TILE_SIZE, TILE_SIZE);
    SDL2pp::Rect src(0, 0, TILE_SIZE, TILE_SIZE);

    switch (type) {
        case TileType::GRASS:
            renderer.Copy(cache.get("Tiles_pasto.png"),  src, dst); break;
        case TileType::WATER:
            renderer.Copy(cache.get("Tiles_agua.png"),   src, dst); break;
        case TileType::DIRT:
            renderer.Copy(cache.get("Tiles_tierra.png"), src, dst); break;
        default:
            // Tile vacío, no dibujamos nada
            break;

    }
}

void MapRenderer::renderHead(const Player& player, float camX, float camY) {
    // Misma posición base que el cuerpo
    int screenX = (int)(player.x * TILE_SIZE - camX) + TILE_SIZE/2 - SPRITE_W/2;
    int screenY = (int)(player.y * TILE_SIZE - camY) + TILE_SIZE/2 - SPRITE_H/2;

    // La cabeza va centrada en la parte superior del cuerpo
    // Celda de cabeza: 27px ancho, 64px alto
    static constexpr int HEAD_CELL_W = 27;
    static constexpr int HEAD_CELL_H = 64;

    int col = player.headId;               // qué cabeza
    int row = static_cast<int>(player.dir); // dirección = fila

    SDL2pp::Rect src(col * HEAD_CELL_W, row * HEAD_CELL_H, HEAD_CELL_W, HEAD_CELL_H);

    // Centrar la cabeza horizontalmente sobre el cuerpo
    int headX = screenX + SPRITE_W/2 - HEAD_CELL_W/2;
    int headY = screenY - HEAD_CELL_H/4 - 3;  // alinear con la parte superior del cuerpo

    SDL2pp::Rect dst(headX, headY, HEAD_CELL_W, HEAD_CELL_H);

    renderer.Copy(cache.get("Cabezas.png"), src, dst);
}
