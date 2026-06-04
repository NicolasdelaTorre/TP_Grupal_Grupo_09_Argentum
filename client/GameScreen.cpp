#include "GameScreen.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <utility>

namespace {

// ReceivedMap (wire) → GameMap (lo que pinta el MapRenderer).
GameMap convertToGameMap(const ReceivedMap& m) {
    GameMap gm;
    gm.width = m.width;
    gm.height = m.height;
    gm.tiles.resize(m.cells.size());
    for (size_t i = 0; i < m.cells.size(); i++) {
        const auto& cell = m.cells[i];
        if (cell.obstacleId != 0) {
            gm.tiles[i].blocked = true;
            gm.tiles[i].obstacleType = static_cast<ObstacleType>(cell.obstacleId);
        } else if (cell.safeZone) {
            gm.tiles[i].floor = TileType::INTERIOR;
            gm.tiles[i].blocked = false;
        } else {
            gm.tiles[i].floor = TileType::GRASS;
            gm.tiles[i].blocked = false;
        }
    }
    return gm;
}

}  // namespace

// tile del servidor (donde caen los pies) → coords del Player.
// p.x e p.y = tile del jugador, así el sprite queda centrado en la celda.
static void tileToPlayerCoords(int16_t tileX, int16_t tileY, Player& p) {
    p.x = static_cast<float>(tileX);
    p.y = static_cast<float>(tileY);
}

// Mapea la dirección wire (3=TOP, 4=BOTTOM, 5=LEFT, 6=RIGHT) a la Direction
// del cliente (que usa otros valores porque son índices de fila en el spritesheet).
static Direction wireDirToSpriteDir(uint8_t wireDir) {
    switch (wireDir) {
        case 3:
            return Direction::UP;
        case 4:
            return Direction::DOWN;
        case 5:
            return Direction::LEFT;
        case 6:
            return Direction::RIGHT;
        default:
            return Direction::DOWN;
    }
}

GameScreen::GameScreen(SDL2pp::Renderer& renderer, const std::string& assetsPath,
                       Queue<std::string>& events_queue, Queue<std::string>& server_queue,
                       const ReceivedMap& mapData, Position spawn):
        renderer(renderer),
        cache(renderer, assetsPath),
        mapRenderer(renderer, cache),
        map(convertToGameMap(mapData)),
        events_queue(events_queue),
        server_queue(server_queue) {
    tileToPlayerCoords(spawn.x, spawn.y, player);

    lastTileX = (int)(player.x + HEAD_OFFSET);
    lastTileY = (int)(player.y + FEET_OFFSET);
    lastSentDir = player.dir;
}

bool GameScreen::run() {
    Uint32 lastTime = SDL_GetTicks();

    while (true) {
        Uint32 now = SDL_GetTicks();
        float dt = (now - lastTime) / 1000.0f;
        lastTime = now;

        if (!handleEvents(dt))
            return false;
        update(dt);
        render();
        SDL_Delay(16);
    }
}

void GameScreen::render() {
    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer.Get(), &screenW, &screenH);

    // Cámara centrada en el jugadorANIM_SPEED
    float camX = player.x * TILE_SIZE - screenW / 2.0f + TILE_SIZE / 2.0f;
    float camY = player.y * TILE_SIZE - screenH / 2.0f + TILE_SIZE / 2.0f;

    // Clampear cámara al mapa
    camX = std::max(0.0f, std::min(camX, (float)(map.width * TILE_SIZE - screenW)));
    camY = std::max(0.0f, std::min(camY, (float)(map.height * TILE_SIZE - screenH)));

    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();

    mapRenderer.render(map, camX, camY);
    mapRenderer.renderDroppedItems(droppedItems, camX, camY);

    // Otros jugadores primero, el local queda visualmente encima.
    // for (const auto& [id, op: otherPlayers]) {(void)id .....}
    for (const auto& playerEntry: otherPlayers) {
        const auto& op = playerEntry.second;
        mapRenderer.renderPlayer(op.visual, camX, camY);
        mapRenderer.renderWeapon(op.visual, camX, camY);
        mapRenderer.renderHead(op.visual, camX, camY);
    }

    mapRenderer.renderPlayer(player, camX, camY);
    mapRenderer.renderWeapon(player, camX, camY);
    mapRenderer.renderHead(player, camX, camY);

    renderHUD();

    renderer.Present();
}

bool GameScreen::handleEvents(float dt) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT)
            return false;
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
            return false;
    }

    // Movimiento continuo con teclas sostenidas
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    float dx = 0, dy = 0;

    const char* msg = nullptr;

    if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
        dy = -PLAYER_MOVE_SPEED * dt;
        player.dir = Direction::UP;
    } else if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
        dy = PLAYER_MOVE_SPEED * dt;
        player.dir = Direction::DOWN;
    } else if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
        dx = -PLAYER_MOVE_SPEED * dt;
        player.dir = Direction::LEFT;
    } else if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
        dx = PLAYER_MOVE_SPEED * dt;
        player.dir = Direction::RIGHT;
    } else if (keys[SDL_SCANCODE_F1]) {
        msg = "CHEAT_SUICIDE";
    } else if (keys[SDL_SCANCODE_F2]) {
        msg = "CHEAT_GOLD";
    } else if (keys[SDL_SCANCODE_F3]) {
        msg = "CHEAT_EXPERIENCE";
    }

    if (msg) {
        events_queue.push(msg);
    }
    player.moving = (dx != 0 || dy != 0);

    // Mover si el tile destino no está bloqueado

    float newX = player.x + dx;
    float newY = player.y + dy;


    int tileX = (int)(newX + HEAD_OFFSET);
    int tileY = (int)(newY + FEET_OFFSET);
    int curFootY = (int)(player.y + FEET_OFFSET);
    int curHeadX = (int)(player.x + HEAD_OFFSET);

    // Bloqueamos si el server lo rechazaría (bounds, obstáculo, u otro jugador).
    if (map.inBounds(tileX, curFootY) && !map.at(tileX, curFootY).blocked &&
        !isOccupiedByOther(tileX, curFootY))
        player.x = newX;

    if (map.inBounds(curHeadX, tileY) && !map.at(curHeadX, tileY).blocked &&
        !isOccupiedByOther(curHeadX, tileY))
        player.y = newY;

    // Si cambiamos de tile, le avisamos al server
    notifyTileChange();
    notifyDirectionChange();
    return true;
}

void GameScreen::notifyDirectionChange() {
    if (player.dir == lastSentDir)
        return;
    const char* msg = nullptr;
    switch (player.dir) {
        case Direction::UP:
            msg = "TURN_TOP";
            break;
        case Direction::DOWN:
            msg = "TURN_BOTTOM";
            break;
        case Direction::LEFT:
            msg = "TURN_LEFT";
            break;
        case Direction::RIGHT:
            msg = "TURN_RIGHT";
            break;
    }
    if (msg)
        events_queue.push(msg);
    lastSentDir = player.dir;
}

void GameScreen::notifyTileChange() {
    int curTileX = (int)(player.x + HEAD_OFFSET);
    int curTileY = (int)(player.y + FEET_OFFSET);

    if (curTileX != lastTileX || curTileY != lastTileY) {
        std::cout << "Pos: (" << curTileX << ", " << curTileY << ")" << std::endl;
    }

    if (curTileX != lastTileX) {
        events_queue.push(curTileX > lastTileX ? "RIGHT" : "LEFT");
    }
    if (curTileY != lastTileY) {
        events_queue.push(curTileY > lastTileY ? "BOTTOM" : "TOP");
    }

    lastTileX = curTileX;
    lastTileY = curTileY;
}

void GameScreen::update(float dt) {
    // Consumimos eventos del servidor antes de animar.
    consumeServerEvents();

    // Interpolamos a los otros jugadores hacia su tile destino para que se vea
    // un walk fluido en vez de saltos de tile en tile.
    for (auto& entry: otherPlayers) {
        auto& op = entry.second;
        float dx = op.targetX - op.visual.x;
        float dy = op.targetY - op.visual.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        float step = PLAYER_MOVE_SPEED * dt;
        if (dist <= step || dist == 0.0f) {
            op.visual.x = op.targetX;
            op.visual.y = op.targetY;
            op.visual.moving = false;
            op.visual.animFrame = 0;
            op.visual.animTimer = 0;
        } else {
            op.visual.x += (dx / dist) * step;
            op.visual.y += (dy / dist) * step;
            op.visual.moving = true;
            op.visual.animTimer += dt;
            if (op.visual.animTimer >= ANIM_SPEED) {
                op.visual.animTimer -= ANIM_SPEED;
                op.visual.animFrame = (op.visual.animFrame + 1) % ANIM_FRAMES;
            }
        }
    }

    if (!player.moving) {
        player.animFrame = 0;
        player.animTimer = 0;
        return;
    }
    player.animTimer += dt;
    if (player.animTimer >= ANIM_SPEED) {
        player.animTimer -= ANIM_SPEED;
        player.animFrame = (player.animFrame + 1) % ANIM_FRAMES;
    }
}

bool GameScreen::isOccupiedByOther(int tileX, int tileY) const {
    for (const auto& playerEntry: otherPlayers) {
        const auto& op = playerEntry.second;
        int opTileX = (int)(op.visual.x + HEAD_OFFSET);
        int opTileY = (int)(op.visual.y + FEET_OFFSET);
        if (opTileX == tileX && opTileY == tileY)
            return true;
        // También el tile destino: si está caminando hacia (tileX,tileY) no podemos pisarlo.
        int opTargetX = (int)(op.targetX + HEAD_OFFSET);
        int opTargetY = (int)(op.targetY + FEET_OFFSET);
        if (opTargetX == tileX && opTargetY == tileY)
            return true;
    }
    return false;
}

void GameScreen::consumeServerEvents() {
    std::string event;
    while (server_queue.try_pop(event)) {
        if (event.rfind("NEW_PLAYER:", 0) == 0) {
            // NEW_PLAYER:<id>:<x>:<y>:<dir>:<name>
            size_t c1 = event.find(':');
            size_t c2 = event.find(':', c1 + 1);
            size_t c3 = event.find(':', c2 + 1);
            size_t c4 = event.find(':', c3 + 1);
            size_t c5 = event.find(':', c4 + 1);
            if (c5 == std::string::npos)
                continue;
            int id = std::stoi(event.substr(c1 + 1, c2 - c1 - 1));
            int16_t x = static_cast<int16_t>(std::stoi(event.substr(c2 + 1, c3 - c2 - 1)));
            int16_t y = static_cast<int16_t>(std::stoi(event.substr(c3 + 1, c4 - c3 - 1)));
            uint8_t dir = static_cast<uint8_t>(std::stoi(event.substr(c4 + 1, c5 - c4 - 1)));
            OtherPlayer op;
            tileToPlayerCoords(x, y, op.visual);
            op.targetX = static_cast<float>(x);
            op.targetY = static_cast<float>(y);
            op.visual.dir = wireDirToSpriteDir(dir);
            op.name = event.substr(c5 + 1);
            otherPlayers[id] = std::move(op);
        } else if (event.rfind("PLAYER_MOVED:", 0) == 0) {
            // PLAYER_MOVED:<id>:<x>:<y>:<dir>
            size_t c1 = event.find(':');
            size_t c2 = event.find(':', c1 + 1);
            size_t c3 = event.find(':', c2 + 1);
            size_t c4 = event.find(':', c3 + 1);
            if (c4 == std::string::npos)
                continue;
            int id = std::stoi(event.substr(c1 + 1, c2 - c1 - 1));
            int16_t x = static_cast<int16_t>(std::stoi(event.substr(c2 + 1, c3 - c2 - 1)));
            int16_t y = static_cast<int16_t>(std::stoi(event.substr(c3 + 1, c4 - c3 - 1)));
            uint8_t dir = static_cast<uint8_t>(std::stoi(event.substr(c4 + 1)));
            auto it = otherPlayers.find(id);
            if (it != otherPlayers.end()) {
                it->second.targetX = static_cast<float>(x);
                it->second.targetY = static_cast<float>(y);
                it->second.visual.dir = wireDirToSpriteDir(dir);
            }
        } else if (event.rfind("PLAYER_DISCONNECTED:", 0) == 0) {
            // PLAYER_DISCONNECTED:<id>
            size_t c1 = event.find(':');
            int id = std::stoi(event.substr(c1 + 1));
            otherPlayers.erase(id);
        } else if (event.rfind("DROPPED_ITEMS:", 0) == 0) {
            // DROPPED_ITEMS:<count>:<x>:<y>:<sheetId>:<itemId>:...
            droppedItems.clear();
            size_t pos = event.find(':');
            size_t next = event.find(':', pos + 1);
            int count = std::stoi(event.substr(pos + 1, next - pos - 1));
            pos = next;
            for (int i = 0; i < count && pos != std::string::npos; i++) {
                DroppedItem di;
                next = event.find(':', pos + 1);
                di.x = static_cast<int16_t>(std::stoi(event.substr(pos + 1, next - pos - 1)));
                pos = next;
                next = event.find(':', pos + 1);
                di.y = static_cast<int16_t>(std::stoi(event.substr(pos + 1, next - pos - 1)));
                pos = next;
                next = event.find(':', pos + 1);
                di.sheetId = static_cast<uint8_t>(std::stoi(event.substr(pos + 1, next - pos - 1)));
                pos = next;
                next = event.find(':', pos + 1);
                di.itemId = static_cast<uint16_t>(std::stoi(event.substr(
                        pos + 1, next == std::string::npos ? std::string::npos : next - pos - 1)));
                pos = next;
                droppedItems.push_back(di);
            }
        } else if (event.rfind("STATS:", 0) == 0) {
            // STATS:<hp>:<maxHp>:<level>
            size_t c1 = event.find(':');
            size_t c2 = event.find(':', c1 + 1);
            size_t c3 = event.find(':', c2 + 1);
            if (c3 == std::string::npos)
                continue;
            health = static_cast<uint16_t>(std::stoi(event.substr(c1 + 1, c2 - c1 - 1)));
            maxHealth = static_cast<uint16_t>(std::stoi(event.substr(c2 + 1, c3 - c2 - 1)));
            level = static_cast<uint8_t>(std::stoi(event.substr(c3 + 1)));
        }
    }
}

void GameScreen::renderHUD() {
    if (maxHealth == 0)
        return;  // no recibimos stats todavía

    // Barra de vida en la esquina superior izquierda
    static constexpr int HUD_X = 10;
    static constexpr int HUD_Y = 10;
    static constexpr int BAR_W = 200;
    static constexpr int BAR_H = 20;

    // Fondo gris
    renderer.SetDrawColor(60, 60, 60, 220);
    SDL_Rect bg{HUD_X, HUD_Y, BAR_W, BAR_H};
    SDL_RenderFillRect(renderer.Get(), &bg);

    // Vida actual (rojo)
    int filledW = static_cast<int>(BAR_W * (float)health / (float)maxHealth);
    renderer.SetDrawColor(180, 30, 30, 255);
    SDL_Rect fill{HUD_X, HUD_Y, filledW, BAR_H};
    SDL_RenderFillRect(renderer.Get(), &fill);

    // Borde
    renderer.SetDrawColor(0, 0, 0, 255);
    SDL_Rect border{HUD_X, HUD_Y, BAR_W, BAR_H};
    SDL_RenderDrawRect(renderer.Get(), &border);
}
