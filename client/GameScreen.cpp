#include "GameScreen.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <utility>

#include "../common/Communication/events/client_events.h"
#include "../common/Communication/message_types.h"

#include "tile_textures.h"

namespace {

// MapEvent (wire) → GameMap (lo que pinta el MapRenderer).
GameMap convertToGameMap(const MapEvent& m) {
    GameMap gm;
    gm.width = m.getWidth();
    gm.height = m.getHeight();
    const auto& cells = m.getCells();
    gm.tiles.resize(cells.size());
    for (size_t i = 0; i < cells.size(); i++) {
        const auto& cell = cells[i];
        // safeZone fuerza piso de ciudad (priority sobre textureId del bioma).
        if (cell.safeZone) {
            gm.tiles[i].floor = TileType::INTERIOR;
        } else {
            gm.tiles[i].floor = tileTypeFromTextureId(cell.textureId);
        }
        if (cell.obstacleId != 0) {
            gm.tiles[i].blocked = true;
            gm.tiles[i].obstacleType = static_cast<ObstacleType>(cell.obstacleId);
        } else {
            gm.tiles[i].blocked = false;
        }
    }
    return gm;
}

// Convierte un Direction (sprite) al MoveDirection del wire.
MoveDirection spriteDirToWire(Direction d) {
    switch (d) {
        case Direction::UP: return MoveDirection::TOP;
        case Direction::DOWN: return MoveDirection::BOTTOM;
        case Direction::LEFT: return MoveDirection::LEFT;
        case Direction::RIGHT: return MoveDirection::RIGHT;
    }
    return MoveDirection::BOTTOM;
}

}  // namespace

// tile del servidor (donde caen los pies) → coords del Player.
// p.x e p.y = tile del jugador, así el sprite queda centrado en la celda.
static void tileToPlayerCoords(int16_t tileX, int16_t tileY, Player_& p) {
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
                       OutgoingQueue& clientEvents, IncomingQueue& serverEvents,
                       const MapEvent& mapData, Position spawn, Player_ player):
        renderer(renderer),
        cache(renderer, assetsPath),
        mapRenderer(renderer, cache),
        map(convertToGameMap(mapData)),
        clientEvents(clientEvents),
        serverEvents(serverEvents),
        player(player) {
    tileToPlayerCoords(spawn.x, spawn.y, this->player);

    lastTileX = (int)(this->player.x + HEAD_OFFSET);
    lastTileY = (int)(this->player.y + FEET_OFFSET);
    lastSentDir = this->player.dir;
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

    // NPCs vivos (debajo de los jugadores).
    for (const auto& npcEntry: npcs) {
        if (!npcEntry.second.alive)
            continue;
        mapRenderer.renderNpcEntity(npcEntry.second.visual, camX, camY);
    }

    // Otros jugadores primero, el local queda visualmente encima.
    // for (const auto& [id, op: otherPlayers]) {(void)id .....}
    for (const auto& playerEntry: otherPlayers) {
        const auto& op = playerEntry.second;
        mapRenderer.renderPlayer(op.visual, camX, camY);
        mapRenderer.renderWeapon(op.visual, camX, camY);
        mapRenderer.renderShield(op.visual, camX, camY);
        mapRenderer.renderHead(op.visual, camX, camY);
        mapRenderer.renderHelmet(op.visual, camX, camY);
    }

    mapRenderer.renderPlayer(player, camX, camY);
    mapRenderer.renderWeapon(player, camX, camY);
    mapRenderer.renderShield(player, camX, camY);
    mapRenderer.renderHead(player, camX, camY);
    mapRenderer.renderHelmet(player, camX, camY);

    mapRenderer.renderArrows(arrows, camX, camY);

    renderBloodEffects(camX, camY);

    renderHUD();

    renderer.Present();
}

bool GameScreen::handleEvents(float dt) {
    // Para resolver clicks en coords de mundo necesitamos la cámara.
    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer.Get(), &screenW, &screenH);
    float camX = player.x * TILE_SIZE - screenW / 2.0f + TILE_SIZE / 2.0f;
    float camY = player.y * TILE_SIZE - screenH / 2.0f + TILE_SIZE / 2.0f;
    camX = std::max(0.0f, std::min(camX, (float)(map.width * TILE_SIZE - screenW)));
    camY = std::max(0.0f, std::min(camY, (float)(map.height * TILE_SIZE - screenH)));

    // Acciones edge-triggered: un evento = una acción. Filtramos los repeats
    // sintéticos del OS con e.key.repeat == 0.
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT)
            return false;

        // Modo chat: capturamos texto y volamos cualquier otro input.
        if (chatActive) {
            if (e.type == SDL_TEXTINPUT) {
                chatBuffer += e.text.text;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
                    if (!chatBuffer.empty()) {
                        clientEvents.push(std::make_shared<ChatMessageEvent>(chatBuffer));
                    }
                    chatBuffer.clear();
                    chatActive = false;
                    SDL_StopTextInput();
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    chatBuffer.clear();
                    chatActive = false;
                    SDL_StopTextInput();
                } else if (e.key.keysym.sym == SDLK_BACKSPACE && !chatBuffer.empty()) {
                    chatBuffer.pop_back();
                }
            }
            continue;
        }

        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE)
                return false;
            if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
                chatActive = true;
                chatBuffer.clear();
                SDL_StartTextInput();
                continue;
            }
        }
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            // Click sobre otro player o NPC → ATTACK con id del target.
            // Server valida si el atacante tiene arma equipada, si es de rango
            // o si está adyacente (para melee), etc. El cliente no decide nada.
            int clickTileX = (int)((e.button.x + camX) / TILE_SIZE);
            int clickTileY = (int)((e.button.y + camY) / TILE_SIZE);
            bool clicked = false;
            for (const auto& entry: otherPlayers) {
                const auto& op = entry.second;
                int opTileX = (int)(op.visual.x + HEAD_OFFSET);
                int opTileY = (int)(op.visual.y + FEET_OFFSET);
                if (opTileX == clickTileX && opTileY == clickTileY) {
                    clientEvents.push(std::make_shared<AttackEvent>(
                            /*targetType=*/0, static_cast<uint16_t>(entry.first)));
                    clicked = true;
                    break;
                }
            }
            if (!clicked) {
                for (const auto& entry: npcs) {
                    const auto& n = entry.second;
                    if (!n.alive)
                        continue;
                    int nTileX = (int)(n.visual.x + HEAD_OFFSET);
                    int nTileY = (int)(n.visual.y + FEET_OFFSET);
                    if (nTileX == clickTileX && nTileY == clickTileY) {
                        clientEvents.push(std::make_shared<AttackEvent>(
                                /*targetType=*/1, static_cast<uint16_t>(entry.first)));
                        break;
                    }
                }
            }
        }
    }

    // En modo chat las teclas no mueven al jugador.
    if (chatActive) {
        player.moving = false;
        return true;
    }

    // Movimiento continuo con teclas sostenidas (level-triggered).
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
    clientEvents.push(std::make_shared<TurnEvent>(spriteDirToWire(player.dir)));
    lastSentDir = player.dir;
}

void GameScreen::notifyTileChange() {
    int curTileX = (int)(player.x + HEAD_OFFSET);
    int curTileY = (int)(player.y + FEET_OFFSET);

    if (curTileX != lastTileX || curTileY != lastTileY) {
        std::cout << "Pos: (" << curTileX << ", " << curTileY << ")" << std::endl;
    }

    if (curTileX != lastTileX) {
        clientEvents.push(std::make_shared<MovementEvent>(
                curTileX > lastTileX ? MoveDirection::RIGHT : MoveDirection::LEFT));
    }
    if (curTileY != lastTileY) {
        clientEvents.push(std::make_shared<MovementEvent>(
                curTileY > lastTileY ? MoveDirection::BOTTOM : MoveDirection::TOP));
    }

    lastTileX = curTileX;
    lastTileY = curTileY;
}

void GameScreen::update(float dt) {
    // Consumimos eventos del servidor antes de animar.
    consumeServerEvents();

    // Tick blood effects and remove expired ones.
    for (auto& b: bloodEffects) b.timer -= dt;
    bloodEffects.erase(std::remove_if(bloodEffects.begin(), bloodEffects.end(),
                                      [](const BloodEffect& b) { return b.timer <= 0.0f; }),
                       bloodEffects.end());

    // Tick arrows and remove ones that reached the target or expired.
    for (auto& arrow: arrows) {
        arrow.x += arrow.vx * dt;
        arrow.y += arrow.vy * dt;
        arrow.lifetime -= dt;
    }
    arrows.erase(std::remove_if(arrows.begin(), arrows.end(),
                                [](const ArrowProjectile& a) { return a.lifetime <= 0.0f; }),
                 arrows.end());

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

    // Mismo trato para los NPCs: interpolación visual hacia su tile destino.
    for (auto& entry: npcs) {
        auto& rn = entry.second;
        if (!rn.alive)
            continue;
        float dx = rn.targetX - rn.visual.x;
        float dy = rn.targetY - rn.visual.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        float step = PLAYER_MOVE_SPEED * dt;
        if (dist <= step || dist == 0.0f) {
            rn.visual.x = rn.targetX;
            rn.visual.y = rn.targetY;
            rn.visual.moving = false;
            rn.visual.animFrame = 0;
            rn.visual.animTimer = 0;
        } else {
            rn.visual.x += (dx / dist) * step;
            rn.visual.y += (dy / dist) * step;
            rn.visual.moving = true;
            rn.visual.animTimer += dt;
            if (rn.visual.animTimer >= ANIM_SPEED) {
                rn.visual.animTimer -= ANIM_SPEED;
                rn.visual.animFrame = (rn.visual.animFrame + 1) % ANIM_FRAMES;
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
    std::shared_ptr<ServerEvent> ev;
    while (serverEvents.try_pop(ev)) {
        if (auto* np = dynamic_cast<NewPlayerEvent*>(ev.get())) {
            OtherPlayer op;
            tileToPlayerCoords(np->getX(), np->getY(), op.visual);
            op.targetX = static_cast<float>(np->getX());
            op.targetY = static_cast<float>(np->getY());
            op.visual.dir = wireDirToSpriteDir(np->getDir());
            op.visual.skin = np->getSkin();
            op.name = np->getName();
            otherPlayers[np->getId()] = std::move(op);
        } else if (auto* pm = dynamic_cast<PlayerMovedEvent*>(ev.get())) {
            auto it = otherPlayers.find(pm->getId());
            if (it != otherPlayers.end()) {
                it->second.targetX = static_cast<float>(pm->getX());
                it->second.targetY = static_cast<float>(pm->getY());
                it->second.visual.dir = wireDirToSpriteDir(pm->getDir());
            }
        } else if (auto* pd = dynamic_cast<PlayerDisconnectedEvent*>(ev.get())) {
            auto it = otherPlayers.find(pd->getId());
            if (it != otherPlayers.end()) {
                std::cout << "Player " << it->second.name << " (id=" << pd->getId()
                          << ") disconnected" << std::endl;
            }
            otherPlayers.erase(pd->getId());
        } else if (auto* ar = dynamic_cast<AttackResultEvent*>(ev.get())) {
            // TODO(team-ui): mostrar feedback visual.
            std::cout << "ATTACK: " << ar->getAttackerId() << " -> " << ar->getTargetId()
                      << (ar->getHit() ? " hit for " : " MISS (") << ar->getDamage()
                      << (ar->getHit() ? " dmg" : ")") << std::endl;
        } else if (auto* eq = dynamic_cast<PlayerEquippedEvent*>(ev.get())) {
            // TODO(team-ui): aplicar al sprite del otro jugador.
            std::cout << "PLAYER_EQUIPPED pid=" << eq->getPlayerId()
                      << " slot=" << (int)eq->getSlot() << " itemId=" << (int)eq->getItemId()
                      << std::endl;
        } else if (auto* inv = dynamic_cast<InventoryUpdateEvent*>(ev.get())) {
            // TODO(team-ui): dibujar el inventario en el HUD.
            std::cout << "INVENTORY_UPDATE count=" << inv->getItems().size()
                      << " eqW=" << (int)inv->getEquippedWeapon() << std::endl;
        } else if (auto* st = dynamic_cast<StatsEvent*>(ev.get())) {
            health = st->getHp();
            maxHealth = st->getMaxHp();
            mana = st->getMana();
            maxMana = st->getMaxMana();
            gold = st->getGold();
            experience = st->getExp();
            nextLevelExp = st->getNextLvlExp();
            level = st->getLevel();
            std::cout << "STATS hp=" << health << "/" << maxHealth << " mana=" << mana << "/"
                      << maxMana << " gold=" << gold << " exp=" << experience << "/" << nextLevelExp
                      << " lvl=" << (int)level << std::endl;
        } else if (auto* nn = dynamic_cast<NewNpcEvent*>(ev.get())) {
            RemoteNpc rn;
            rn.visual.id = nn->getId();
            rn.visual.x = static_cast<float>(nn->getX());
            rn.visual.y = static_cast<float>(nn->getY());
            rn.visual.type = static_cast<NpcType>(nn->getType());
            rn.targetX = rn.visual.x;
            rn.targetY = rn.visual.y;
            rn.alive = nn->getAlive();
            npcs[nn->getId()] = std::move(rn);
        } else if (auto* nm = dynamic_cast<NpcMovedEvent*>(ev.get())) {
            auto it = npcs.find(nm->getId());
            if (it != npcs.end()) {
                it->second.targetX = static_cast<float>(nm->getX());
                it->second.targetY = static_cast<float>(nm->getY());
                it->second.visual.dir = wireDirToSpriteDir(nm->getDir());
            }
        } else if (auto* nd = dynamic_cast<NpcDiedEvent*>(ev.get())) {
            auto it = npcs.find(nd->getId());
            if (it != npcs.end()) {
                it->second.alive = false;
            }
        } else if (auto* nr = dynamic_cast<NpcRespawnedEvent*>(ev.get())) {
            auto it = npcs.find(nr->getId());
            if (it != npcs.end()) {
                it->second.visual.x = static_cast<float>(nr->getX());
                it->second.visual.y = static_cast<float>(nr->getY());
                it->second.targetX = it->second.visual.x;
                it->second.targetY = it->second.visual.y;
                it->second.alive = true;
            }
        } else if (auto* cb = dynamic_cast<ChatBroadcastEvent*>(ev.get())) {
            // TODO(team-ui): dibujar burbuja sobre el autor o ventana de chat.
            if (cb->getAuthorId() == 0) {
                std::cout << "[sistema] " << cb->getText() << std::endl;
            } else {
                std::cout << "[chat] " << cb->getAuthorName() << ": " << cb->getText() << std::endl;
            }
        }
    }
}

void GameScreen::renderBloodEffects(float camX, float camY) {
    static constexpr int BLOOD_FRAMES = 5;
    for (const auto& b: bloodEffects) {
        float elapsed = BLOOD_DURATION - b.timer;
        int frame = std::min(BLOOD_FRAMES - 1, (int)(elapsed / (BLOOD_DURATION / BLOOD_FRAMES)));
        mapRenderer.renderBlood(b.x, b.y, frame, 255, camX, camY);
    }
}

void GameScreen::renderHUD() {
    // TODO(team-ui): dibujar HUD con HP/MP/oro/exp/nivel a partir de los
    // miembros `health/maxHealth/mana/maxMana/gold/experience/nextLevelExp/level`
    // que ya se actualizan al recibir STATS_JUGADOR.
}
