#include "GameScreen.h"

#include <algorithm>

GameScreen::GameScreen(SDL2pp::Renderer& renderer,
                       const std::string& assetsPath,
                       Queue<std::string>& events_queue):
        renderer(renderer),
        cache(renderer, assetsPath),
        mapRenderer(renderer, cache),
        map(makeTestMap()),
        events_queue(events_queue) {
    // El tile "lógico" del jugador es donde están sus pies, calculado con
    // (player.x + HEAD_OFFSET, player.y + FEET_OFFSET). Restamos los offsets
    // para que el tile de los pies caiga en el centro del mapa, alineado
    // con la posición que el servidor le asigna en Map::addPlayer().
    player.x = map.width / 2.0f - HEAD_OFFSET;
    player.y = map.height / 2.0f - FEET_OFFSET;

    lastTileX = (int)(player.x + HEAD_OFFSET);
    lastTileY = (int)(player.y + FEET_OFFSET);
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
    mapRenderer.renderPlayer(player, camX, camY);
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
        dy = -Player::MOVE_SPEED * dt;
        player.dir = Direction::UP;
    } else if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
        dy = Player::MOVE_SPEED * dt;
        player.dir = Direction::DOWN;
    } else if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
        dx = -Player::MOVE_SPEED * dt;
        player.dir = Direction::LEFT;
    } else if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
        dx = Player::MOVE_SPEED * dt;
        player.dir = Direction::RIGHT;
    }

    player.moving = (dx != 0 || dy != 0);

    // Mover si el tile destino no está bloqueado

    float newX = player.x + dx;
    float newY = player.y + dy;


    int tileX = (int)(newX + HEAD_OFFSET);
    int tileY = (int)(newY + FEET_OFFSET);

    if (map.inBounds(tileX, (int)(player.y + FEET_OFFSET)) &&
        !map.at(tileX, (int)(player.y + FEET_OFFSET)).blocked)
        player.x = newX;

    if (map.inBounds((int)(player.x + HEAD_OFFSET), tileY) &&
        !map.at((int)(player.x + HEAD_OFFSET), tileY).blocked)
        player.y = newY;

    // Si cambiamos de tile, le avisamos al server
    notifyTileChange();
    return true;
}

void GameScreen::notifyTileChange() {
    int curTileX = (int)(player.x + HEAD_OFFSET);
    int curTileY = (int)(player.y + FEET_OFFSET);

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
    if (!player.moving) {
        player.animFrame = 0;
        player.animTimer = 0;
        return;
    }
    player.animTimer += dt;
    if (player.animTimer >= Player::ANIM_SPEED) {
        player.animTimer -= Player::ANIM_SPEED;
        player.animFrame = (player.animFrame + 1) % ANIM_FRAMES;
    }
}
