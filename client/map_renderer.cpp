#include "map_renderer.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "../common/common_tiles.h"

#include "item_sprites.h"

namespace {

static constexpr int ITEM_DRAW_SIZE = 40;  // render size on screen (scaled up from 32px)
static constexpr const char* COMMON_ASSET_PATH = "../common/assets/images/";

SDL_Color colorFromHex(const char* hex) {
    unsigned int r = 0;
    unsigned int g = 0;
    unsigned int b = 0;
    if (!hex || std::sscanf(hex, "#%02x%02x%02x", &r, &g, &b) != 3) {
        return {0, 0, 0, 255};
    }
    return {static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), 255};
}

// Devuelve el path del sprite para NPCs de ciudad (obstáculos fijos en el mapa).
// Retorna nullptr si el tipo no es un NPC de ciudad.
const char* cityNpcTexturePath(ObstacleCode type) {
    switch (type) {
        case ObstacleCode::NPC:
        case ObstacleCode::NPC_PRIEST:
        case ObstacleCode::NPC_MERCHANT:
        case ObstacleCode::NPC_BANKER:
            return "/Skins/NPC/Sacerdote.png";
        default:
            return nullptr;
    }
}

// Devuelve el path del sprite para criaturas NPC dinámicas.
const char* npcEntityTexturePath(NpcCode type) {
    switch (type) {
        case NpcCode::SPIDER:
            return "/Skins/NPC/araña.png";
        case NpcCode::SKELETON:
            return "/Skins/NPC/Esqueleto.png";
        case NpcCode::ZOMBIE:
            return "/Skins/NPC/Goblin.png";
        case NpcCode::GOBLIN:
            return "/Skins/NPC/Goblin.png";
        case NpcCode::ORC:
            return "/Skins/NPC/Orc.png";
        case NpcCode::GOLEM:
            return "/Skins/NPC/Golem.png";
        default:
            return "/Skins/NPC/araña.png";
    }
}

// Texturas de obstáculos a tamaño nativo. Viven en common/assets/images; el
// cache del cliente tiene base AO_IMGS, así que se referencian relativo a ella.
const char* obstacleTexturePath(ObstacleCode type) {
    switch (type) {
        case ObstacleCode::ROCK:
            return "../common/assets/images/rock_big.png";
        case ObstacleCode::ROCK_SMALL:
            return "../common/assets/images/rock_medium.png";
        case ObstacleCode::ROCK_LARGE:
            return "../common/assets/images/rock_big.png";
        case ObstacleCode::LAMP:
            return "../common/assets/images/street_lamp.png";
        case ObstacleCode::WOOD:
            return "../common/assets/images/stacked_logs.png";
        case ObstacleCode::CART:
            return "../common/assets/images/cart.png";
        case ObstacleCode::MILL:
            return "/Obstaculos/molino_recortado.png";  // sin etextura
        case ObstacleCode::CACTUS:
            return "../common/assets/images/cactus_big.png";
        case ObstacleCode::BANK:
            return "../common/assets/images/bank.png";
        case ObstacleCode::HOUSE_BLUE:
            return "../common/assets/images/wooden_house_blue.png";
        case ObstacleCode::HOUSE_RED:
            return "../common/assets/images/wooden_house_red.png";
        case ObstacleCode::FENCE:
            return "../common/assets/images/wooden_fence.png";
        case ObstacleCode::TARGET:
            return "../common/assets/images/target.png";
        case ObstacleCode::HAYBALE:
            return "../common/assets/images/haybale.png";
        case ObstacleCode::FOUNTAIN:
            return "../common/assets/images/water_fountain.png";
        case ObstacleCode::BLACKSMITH:
            return "../common/assets/images/blacksmith.png";
        case ObstacleCode::HOTEL:
            return "../common/assets/images/hotel.png";
        case ObstacleCode::CHURCH:
            return "../common/assets/images/church.png";
        case ObstacleCode::TRAINING_DUMMY:
            return "../common/assets/images/training_dummy.png";
        default:
            return nullptr;
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


// Dibuja cada obstáculo a tamaño nativo de su textura, anclado a la esquina
// inferior izquierda de su footprint (el rectángulo que bloquea). El tamaño que
// bloquea es independiente del de la textura: la imagen se coloca tal cual se
// carga (incluida la sombra/voladizo, que sobresale del footprint).
void MapRenderer::renderObstacles(const GameMap& map, float camX, float camY) {
    for (const auto& obs: map.obstacles) {
        const char* texPath = obstacleTexturePath(obs.type);
        if (!texPath)
            continue;

        try {
            SDL2pp::Texture& tex = cache.get(texPath);
            const int texW = tex.GetWidth();
            const int texH = tex.GetHeight();

            // Esquina inferior izquierda del footprint, en coords de pantalla.
            const int leftX = (int)(obs.x * TILE_SIZE - camX);
            const int bottomY = (int)((obs.y + obs.h) * TILE_SIZE - camY);

            SDL2pp::Rect dst(leftX, bottomY - texH, texW, texH);
            renderer.Copy(tex, SDL2pp::NullOpt, dst);
        } catch (...) {
            // Textura no disponible — se ignora silenciosamente.
        }
    }
}

void MapRenderer::renderPlayer(const Player_& player, float camX, float camY) {
    int screenX = (int)(player.x * TILE_SIZE - camX) + TILE_SIZE / 2 - SPRITE_W / 2;
    int screenY = (int)(player.y * TILE_SIZE - camY) + TILE_SIZE / 2 - SPRITE_H / 2;

    int row = static_cast<int>(player.dir);
    int col = player.moving ? player.animFrame : 0;

    if (player.killed) {
        int ghostRow = row;
        if (player.dir == SpriteRow::LEFT)
            ghostRow = static_cast<int>(SpriteRow::RIGHT);
        else if (player.dir == SpriteRow::RIGHT)
            ghostRow = static_cast<int>(SpriteRow::LEFT);
        SDL2pp::Rect src(col * PHANTOM_SPRITE_W, ghostRow * PHANTOM_SPRITE_H, PHANTOM_SPRITE_W,
                         PHANTOM_SPRITE_H);
        SDL2pp::Rect dst(screenX, screenY, PHANTOM_SPRITE_W, PHANTOM_SPRITE_H);
        renderer.Copy(cache.get("/Skins/NPC/Fantasma.png"), src, dst);
        return;
    }


    SDL2pp::Rect src(col * SPRITE_W, row * SPRITE_H, SPRITE_W, SPRITE_H);
    SDL2pp::Rect dst(screenX, screenY, SPRITE_W, SPRITE_H);

    renderer.Copy(cache.get(get_path(player.skin)), src, dst);
}

std::string MapRenderer::get_path(int skin) {
    switch (skin % 5) {
        case 0:
            return "/Skins/Skin_inicial.png";
        case 1:
            return "/Skins/Armadura_de_cuero.png";
        case 2:
            return "/Skins/Gladiador_azul.png";
        case 3:
            return "/Skins/Blue_tunic.png";
        default:
            return "/Skins/skin_default.png";
    }
}

void MapRenderer::drawTile(const TileData& tile, int screenX, int screenY) {
    SDL2pp::Rect dst(screenX, screenY, TILE_SIZE, TILE_SIZE);

    const FloorTile* floorTile = floor_tile_from_grid_value(static_cast<uint8_t>(tile.textureId));
    if (!floorTile) {
        floorTile = floor_tile_from_grid_value(1);
    }

    if (floorTile && floorTile->texture && floorTile->texture[0] != '\0') {
        try {
            renderer.Copy(cache.get(std::string(COMMON_ASSET_PATH) + floorTile->texture),
                          SDL2pp::NullOpt, dst);
            return;
        } catch (...) {
            // Si falta la textura, caemos al color de respaldo del tile.
        }
    }

    const SDL_Color color = floorTile ? colorFromHex(floorTile->color) : SDL_Color{0, 0, 0, 255};
    renderer.SetDrawColor(color.r, color.g, color.b, color.a);
    renderer.FillRect(dst);
}

void MapRenderer::renderWeapon(const Player_& player, float camX, float camY) {
    if (player.killed || player.weaponId < 0)
        return;

    static const char* weaponFiles[] = {"/Armas/Espada.png", "/Armas/Hacha.png", "/Armas/Arco.png", "/Armas/Arco_compuesto.png",
                                        "/Armas/Ash_staff.png", "/Armas/Flauta.png", "/Armas/Martillo.png", "/Armas/Staff_Azul.png",
                                         "/Armas/Staff_rojo.png"};
    if (player.weaponId >= 9)
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

void MapRenderer::renderShield(const Player_& player, float camX, float camY) {
    if (player.killed || player.shieldId < 0 || player.dir == SpriteRow::UP)
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

void MapRenderer::renderHead(const Player_& player, float camX, float camY) {
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

void MapRenderer::renderHelmet(const Player_& player, float camX, float camY) {
    if (player.killed || player.helmetId < 0)
        return;

    // Gorros.png está organizado en bloques de 4 filas (las 4 direcciones
    // Down/Up/Left/Right de un mismo gorro) y 32 columnas (gorros distintos).
    // Celda = 32x64. helmetId viene empaquetado como block*32 + col, donde
    // block = qué "fila de gorros" (grupo de 4 filas). Ver equipment_sprites.cpp.
    static constexpr int HAT_CELL_W = 27;
    static constexpr int HAT_CELL_H = 64;
    static constexpr int HAT_COLS = 32;
    static constexpr int HEAD_CELL_H = 64;

    int col = player.helmetId % HAT_COLS;
    int block = player.helmetId / HAT_COLS;
    int dir = static_cast<int>(player.dir);  // DOWN=0, UP=1, LEFT=2, RIGHT=3
    int atlasRow = block * 4 + dir;

    SDL2pp::Rect src(col * HAT_CELL_W, atlasRow * HAT_CELL_H, HAT_CELL_W, HAT_CELL_H);

    int screenX = (int)(player.x * TILE_SIZE - camX) + TILE_SIZE / 2 - SPRITE_W / 2;
    int screenY = (int)(player.y * TILE_SIZE - camY) + TILE_SIZE / 2 - SPRITE_H / 2;
    // Centrado horizontal sobre el cuerpo (misma referencia que la cabeza), y
    // misma Y que la cabeza para que el gorro quede calzado encima.
    int hatX = screenX + SPRITE_W / 2 - HAT_CELL_W / 2;
    int hatY = screenY - HEAD_CELL_H / 4 - 3;

    int hatNudgeX = 0;
    if (player.helmetId == 38)  
        hatNudgeX = 2;
    hatX -= hatNudgeX;

    SDL2pp::Rect dst(hatX, hatY, HAT_CELL_W, HAT_CELL_H);

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

void MapRenderer::renderDroppedItems(const std::vector<DroppedItem>& items, float camX,
                                     float camY) {
    for (const auto& item: items) {
        // Mismo mapeo que el inventario: el id del item (items.toml) define de
        // qué sheet y celda sale el dibujo. Así el item en el piso coincide.
        ItemSpriteRef ref = itemSpriteFor(item.itemId);
        SDL2pp::Rect src(ref.srcX, ref.srcY, ref.srcW, ref.srcH);

        int screenX = (int)(item.x * TILE_SIZE - camX) + TILE_SIZE / 2 - ITEM_DRAW_SIZE / 2;
        int screenY = (int)(item.y * TILE_SIZE - camY) + TILE_SIZE / 2 - ITEM_DRAW_SIZE / 2;
        SDL2pp::Rect dst(screenX, screenY, ITEM_DRAW_SIZE, ITEM_DRAW_SIZE);

        try {
            renderer.Copy(cache.get(ref.sheetPath), src, dst);
        } catch (...) {}
    }
}

void MapRenderer::renderBlood(float x, float y, int texIndex, Uint8 alpha, float camX, float camY) {
    static constexpr int BLOOD_DRAW_SIZE = 32;
    static const char* bloodFiles[] = {"/Skins/Sangre_1.png", "/Skins/Sangre_2.png",
                                       "/Skins/Sangre_3.png", "/Skins/Sangre_4.png",
                                       "/Skins/Sangre_5.png"};
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
    // Each cell is 32 px wide. Sprites point upper-right (45° CW from north),
    // so the SDL2 rotation formula is: atan2(vx, -vy) * 180/π − 45.
    static constexpr int ARROW_COLS = 9;
    static constexpr int ARROW_CELL_W = 32;
    static constexpr int ARROW_DRAW_SIZE = 32;

    // Explosion.png: tira horizontal de 7 frames (socketed staff). Se anima
    // ciclando según age; este es el período de un ciclo completo.
    static constexpr int EXPLOSION_FRAMES = 7;
    static constexpr float EXPLOSION_FPS = 14.0f;

    for (const auto& arrow: arrows) {
        // Parámetros que dependen del tipo de proyectil.
        const char* texPath = "/Armas/Flechas.png";
        int drawW = ARROW_DRAW_SIZE, drawH = ARROW_DRAW_SIZE;
        bool rotate = true;       // alinear el sprite con la dirección de vuelo
        double extraAngle = 0.0;  // corrección si el sprite no apunta al norte

        switch (arrow.kind) {
            case ProjectileKind::ARROW: extraAngle = -45.0;
             break;
            case ProjectileKind::COMPOSITE_ARROW:
                texPath = "/Armas/Flechas_composite_bow.png";
                extraAngle = -45.0;  // apunta al noreste, como las flechas normales
                break;
            case ProjectileKind::MAGIC_ARROW:
                texPath = "/Armas/Flecha_magica.png";
                drawW = 22; drawH = 26;
                break;
            case ProjectileKind::MISSILE:
                texPath = "/Armas/Misil.png";
                drawW = 18; drawH = 48;  // sprite alto y angosto (63×164)
                break;
            case ProjectileKind::EXPLOSION:
                texPath = "/Armas/Explosion.png";
                drawW = 44; drawH = 44;
                rotate = false;  // la explosión no rota: anima en el lugar
                break;
        }

        int screenX = (int)(arrow.x * TILE_SIZE - camX) - drawW / 2;
        int screenY = (int)(arrow.y * TILE_SIZE - camY) - drawH / 2;
        SDL_Rect dst = {screenX, screenY, drawW, drawH};

        try {
            SDL2pp::Texture& tex = cache.get(texPath);

            // Recorte del sprite dentro de la textura.
            SDL_Rect src;
            if (arrow.kind == ProjectileKind::ARROW) {
                int col = std::max(0, std::min(arrow.arrowType, ARROW_COLS - 1));
                src = {col * ARROW_CELL_W, 0, ARROW_CELL_W, ARROW_CELL_W};
            } else if (arrow.kind == ProjectileKind::EXPLOSION) {
                int texW = tex.GetWidth(), texH = tex.GetHeight();
                float cellW = texW / (float)EXPLOSION_FRAMES;
                int frame = (int)(arrow.age * EXPLOSION_FPS) % EXPLOSION_FRAMES;
                src = {(int)(frame * cellW), 0, (int)cellW, texH};
            } else {
                src = {0, 0, tex.GetWidth(), tex.GetHeight()};
            }

            // Tinte: estos sprites vienen en blanco y negro; colorMod los pinta.
            tex.SetColorMod(arrow.tintR, arrow.tintG, arrow.tintB);

            double angle_deg =
                    rotate ? std::atan2(arrow.vx, -arrow.vy) * 180.0 / M_PI + extraAngle : 0.0;
            SDL_RenderCopyEx(renderer.Get(), tex.Get(), &src, &dst, angle_deg, nullptr,
                             SDL_FLIP_NONE);

            tex.SetColorMod(255, 255, 255);  // restaurar para otros usos del cache
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

    SDL2pp::Rect src(col * PHANTOM_SPRITE_W, row * PHANTOM_SPRITE_H, PHANTOM_SPRITE_W,
                     PHANTOM_SPRITE_H);
    SDL2pp::Rect dst(screenX, screenY, PHANTOM_SPRITE_W, PHANTOM_SPRITE_H);
    try {
        renderer.Copy(cache.get(tex), src, dst);
    } catch (...) {}
}