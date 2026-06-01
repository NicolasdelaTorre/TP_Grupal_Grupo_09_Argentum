#include "GameScreen.h"

#include <algorithm>
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
                tileToPlayerCoords(x, y, it->second.visual);
                it->second.visual.dir = wireDirToSpriteDir(dir);
            }
        } else if (event.rfind("PLAYER_DISCONNECTED:", 0) == 0) {
            // PLAYER_DISCONNECTED:<id>
            size_t c1 = event.find(':');
            int id = std::stoi(event.substr(c1 + 1));
            otherPlayers.erase(id);
        }
    }
}
