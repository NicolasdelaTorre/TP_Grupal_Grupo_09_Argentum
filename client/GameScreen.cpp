#include "GameScreen.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <utility>

#include "../common/Communication/events/client_events.h"
#include "../common/Communication/message_types.h"

#include "tile_textures.h"

// Ancho de la franja del HUD a la derecha (inventario + fondo). El área jugable
// es screenW - HUD_PANEL_W, y la cámara centra al jugador ahí (no en toda la
// pantalla) para que el sprite quede centrado a la izquierda del inventario.
static constexpr int HUD_PANEL_W = 230;

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
        chatTtf(),
        chatFont(assetsPath + "/font.ttf", 14),
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

float GameScreen::uiScale() const {
    int w, h;
    SDL_GetRendererOutputSize(renderer.Get(), &w, &h);
    return h / (float)BASE_SCREEN_H;
}

int GameScreen::hudPanelW() const { return (int)(HUD_PANEL_W * uiScale()); }

int GameScreen::chatBoxH() const { return (int)(CHAT_BOX_H * uiScale()); }

void GameScreen::render() {
    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer.Get(), &screenW, &screenH);

    // Cámara centrada en el jugador dentro del área jugable (sin contar el HUD).
    float playW = screenW - hudPanelW();
    float camX = player.x * TILE_SIZE - playW / 2.0f + TILE_SIZE / 2.0f;
    // Centramos al jugador en el área que queda debajo de la caja de chat:
    // el "techo" de la zona jugable es el borde inferior del chat.
    float camY = player.y * TILE_SIZE - (screenH + chatBoxH()) / 2.0f + TILE_SIZE / 2.0f;

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

    renderChat();

    renderer.Present();
}

bool GameScreen::handleEvents(float dt) {
    // Para resolver clicks en coords de mundo necesitamos la cámara.
    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer.Get(), &screenW, &screenH);
    float playW = screenW - hudPanelW();
    float camX = player.x * TILE_SIZE - playW / 2.0f + TILE_SIZE / 2.0f;
    // Centramos al jugador en el área que queda debajo de la caja de chat:
    // el "techo" de la zona jugable es el borde inferior del chat.
    float camY = player.y * TILE_SIZE - (screenH + chatBoxH()) / 2.0f + TILE_SIZE / 2.0f;
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
                        // Amigos (merchant/banker/priest) → SELECT_NPC.
                        // Hostiles (spider/skeleton/etc) → ATTACK.
                        bool isFriendly = (n.visual.type == NpcType::MERCHANT ||
                                           n.visual.type == NpcType::BANKER ||
                                           n.visual.type == NpcType::PRIEST);
                        if (isFriendly) {
                            clientEvents.push(std::make_shared<SelectNpcEvent>(
                                    static_cast<uint16_t>(entry.first)));
                        } else {
                            clientEvents.push(std::make_shared<AttackEvent>(
                                    /*targetType=*/1, static_cast<uint16_t>(entry.first)));
                        }
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
    // Con el chat abierto no movemos: las teclas son texto del comando.
    const Uint8* keys = chatActive ? nullptr : SDL_GetKeyboardState(nullptr);
    float dx = 0, dy = 0;

    if (keys && (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W])) {
        dy = -PLAYER_MOVE_SPEED * dt;
        player.dir = Direction::UP;
    } else if (keys && (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S])) {
        dy = PLAYER_MOVE_SPEED * dt;
        player.dir = Direction::DOWN;
    } else if (keys && (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A])) {
        dx = -PLAYER_MOVE_SPEED * dt;
        player.dir = Direction::LEFT;
    } else if (keys && (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D])) {
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

void GameScreen::addChatLine(const std::string& line) {
    chatHistory.push_back(line);
    if (chatHistory.size() > MAX_CHAT_LINES)
        chatHistory.erase(chatHistory.begin(),
                          chatHistory.end() - static_cast<long>(MAX_CHAT_LINES));
}

// Parsea el texto del chat a un evento que entiende client_sender y lo encola.
// Formato esperado: "/comando [argumento]". Comandos desconocidos se ignoran.
// Cada comando (y su feedback local) queda registrado en el historial del chat.
void GameScreen::submitChat() {
    // Trim de espacios a los costados.
    size_t start = chatInput.find_first_not_of(" \t");
    if (start == std::string::npos)
        return;
    size_t end = chatInput.find_last_not_of(" \t");
    std::string msg = chatInput.substr(start, end - start + 1);

    // Eco de lo tipeado.
    addChatLine("> " + msg);

    // Separamos comando y argumento (lo que va después del primer espacio).
    std::string cmd, arg;
    size_t sp = msg.find(' ');
    if (sp == std::string::npos) {
        cmd = msg;
    } else {
        cmd = msg.substr(0, sp);
        size_t argStart = msg.find_first_not_of(' ', sp);
        if (argStart != std::string::npos)
            arg = msg.substr(argStart);
    }

    // Comandos sin argumento.
    if (cmd == "/tomar") {
        events_queue.push("PICK_UP");
    } else if (cmd == "/oro") {
        events_queue.push("CHEAT_GOLD");
    } else if (cmd == "/exp") {
        events_queue.push("CHEAT_EXPERIENCE");
    } else if (cmd == "/morir") {
        events_queue.push("CHEAT_SUICIDE");
    } else if (cmd == "/tirar" && !arg.empty()) {
        // /tirar <slot>  → suelta el item de esa slot del inventario.
        events_queue.push("DROP:" + arg);
    } else if (cmd == "/equipar" && !arg.empty()) {
        // /equipar <slot> → equipa el item de esa slot.
        events_queue.push("EQUIP:" + arg);
    } else if (cmd == "/desequipar" && !arg.empty()) {
        // /desequipar <tipo> → desequipa la slot de equipo de ese tipo.
        events_queue.push("UNEQUIP:" + arg);
    } else {
        addChatLine("  comando desconocido");
    }
}

void GameScreen::renderChat() {
    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer.Get(), &screenW, &screenH);

    // Escalamos toda la geometría del chat para que mantenga su proporción en
    // ventana fija y en fullscreen (ver uiScale).
    float scale = uiScale();
    const int LINE_H = (int)(CHAT_LINE_H * scale);  // alto de cada línea de texto
    const int PAD = (int)(CHAT_PAD * scale);         // margen interno de la caja
    // La caja ocupa todo el área jugable: termina donde empieza el panel del HUD
    // (fondo del inventario) a la derecha.
    const int BAR_W = screenW - hudPanelW();
    // La caja muestra MAX_CHAT_LINES de historial + 1 línea de input.
    const int boxH = chatBoxH();

    // Fondo negro semi-transparente. Necesitamos blend habilitado para el alpha.
    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    renderer.SetDrawColor(0, 0, 0, 140);
    renderer.FillRect(SDL2pp::Rect(0, 0, BAR_W, boxH));
    renderer.SetDrawBlendMode(SDL_BLENDMODE_NONE);

    auto drawLine = [&](const std::string& text, int row, SDL_Color color) {
        if (text.empty())
            return;
        try {
            auto surface = chatFont.RenderUTF8_Blended(text, color);
            SDL2pp::Texture tex(renderer, surface);
            auto [tw, th] = tex.GetSize();
            // Escalamos el texto (renderizado a tamaño fijo) de forma uniforme
            // para que entre en la línea escalada sin deformarse.
            float textScale = th > 0 ? (float)LINE_H / th : 1.0f;
            int dstW = std::min((int)(tw * textScale), BAR_W - PAD * 2);
            int y = PAD + row * LINE_H;
            renderer.Copy(tex, SDL2pp::NullOpt, SDL2pp::Rect(PAD + 2, y, dstW, LINE_H));
        } catch (...) {}
    };

    // Historial: ocupa las primeras MAX_CHAT_LINES filas, alineado abajo
    // (las líneas más nuevas quedan pegadas a la línea de input).
    SDL_Color histColor = {210, 210, 210, 255};
    int firstRow = (int)MAX_CHAT_LINES - (int)chatHistory.size();
    for (size_t i = 0; i < chatHistory.size(); i++)
        drawLine(chatHistory[i], firstRow + (int)i, histColor);

    // Línea de input (siempre visible, en la última fila). Mientras se escribe
    // mostramos un cursor "_"; si no, un prompt apagado.
    SDL_Color inputColor = chatActive ? SDL_Color{255, 255, 255, 255}
                                      : SDL_Color{140, 140, 140, 255};
    std::string inputLine = chatActive ? "> " + chatInput + "_"
                                       : "Enter para escribir un comando";
    drawLine(inputLine, (int)MAX_CHAT_LINES, inputColor);
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
        } else if (event.rfind("PLAYER_DISCONNECTED:", 0) == 0) {
            // PLAYER_DISCONNECTED:<id>
            size_t c1 = event.find(':');
            int id = std::stoi(event.substr(c1 + 1));
            otherPlayers.erase(id);
        } else if (event.rfind("ATTACK_RESULT:", 0) == 0) {
            // ATTACK_RESULT:<atk>:<ttype>:<tid>:<dmg>:<hit>
            // TODO(team-ui): mostrar feedback visual (número flotante de daño
            // sobre el target si hit==1, "MISS" si hit==0, animación de impacto).
            // Por ahora solo loggeamos para confirmar que el evento llega.
            size_t c1 = event.find(':');
            size_t c2 = event.find(':', c1 + 1);
            size_t c3 = event.find(':', c2 + 1);
            size_t c4 = event.find(':', c3 + 1);
            size_t c5 = event.find(':', c4 + 1);
            if (c5 == std::string::npos)
                continue;
            int atk = std::stoi(event.substr(c1 + 1, c2 - c1 - 1));
            int tid = std::stoi(event.substr(c3 + 1, c4 - c3 - 1));
            int dmg = std::stoi(event.substr(c4 + 1, c5 - c4 - 1));
            int hit = std::stoi(event.substr(c5 + 1));
            std::cout << "ATTACK: " << atk << " -> " << tid << (hit ? " hit for " : " MISS (")
                      << dmg << (hit ? " dmg" : ")") << std::endl;
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
        } else if (event.rfind("EQUIPPED:", 0) == 0) {
            // EQUIPPED:<playerId>:<slot>:<itemId>
            // TODO(team-ui): aplicar al sprite. El mapping itemId → columna
            // del spritesheet lo define la capa de render.
            std::cout << "PLAYER_EQUIPPED " << event << std::endl;
        } else if (event.rfind("INVENTORY:", 0) == 0) {
            // INVENTORY:<n>:<id1>:...:<idN>:<eqW>:<eqA>:<eqH>:<eqS>
            // Solo nos quedamos con los N itemIds (slots ocupadas, en orden) para
            // dibujarlos sobre el grid del inventario. Los 4 equipped van al final.
            size_t c1 = event.find(':');
            size_t c2 = event.find(':', c1 + 1);
            if (c2 == std::string::npos)
                continue;
            int n = std::stoi(event.substr(c1 + 1, c2 - c1 - 1));
            inventoryItems.clear();
            size_t pos = c2;
            for (int i = 0; i < n && pos != std::string::npos; i++) {
                size_t next = event.find(':', pos + 1);
                inventoryItems.push_back(static_cast<uint8_t>(
                        std::stoi(event.substr(pos + 1, next == std::string::npos
                                                                ? std::string::npos
                                                                : next - pos - 1))));
                pos = next;
            }
        } else if (event.rfind("STATS:", 0) == 0) {
            // STATS:<hp>:<maxHp>:<mana>:<maxMana>:<gold>:<exp>:<nextLvlExp>:<level>
            size_t c1 = event.find(':');
            size_t c2 = event.find(':', c1 + 1);
            size_t c3 = event.find(':', c2 + 1);
            size_t c4 = event.find(':', c3 + 1);
            size_t c5 = event.find(':', c4 + 1);
            size_t c6 = event.find(':', c5 + 1);
            size_t c7 = event.find(':', c6 + 1);
            size_t c8 = event.find(':', c7 + 1);
            if (c8 == std::string::npos)
                continue;
            health = static_cast<uint16_t>(std::stoi(event.substr(c1 + 1, c2 - c1 - 1)));
            maxHealth = static_cast<uint16_t>(std::stoi(event.substr(c2 + 1, c3 - c2 - 1)));
            mana = static_cast<uint16_t>(std::stoi(event.substr(c3 + 1, c4 - c3 - 1)));
            maxMana = static_cast<uint16_t>(std::stoi(event.substr(c4 + 1, c5 - c4 - 1)));
            gold = static_cast<uint32_t>(std::stoul(event.substr(c5 + 1, c6 - c5 - 1)));
            experience = static_cast<uint32_t>(std::stoul(event.substr(c6 + 1, c7 - c6 - 1)));
            nextLevelExp = static_cast<uint32_t>(std::stoul(event.substr(c7 + 1, c8 - c7 - 1)));
            level = static_cast<uint8_t>(std::stoi(event.substr(c8 + 1)));
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
        } else if (auto* id_ = dynamic_cast<ItemDroppedEvent*>(ev.get())) {
            DroppedItem di;
            di.dropId = id_->getDropId();
            di.x = id_->getX();
            di.y = id_->getY();
            // TODO(team-ui-nico): elegir sheetId/itemId reales según el id que
            // viene del server. Por ahora cae todo en el primer sprite.
            di.sheetId = 0;
            di.itemId = id_->getItemId();
            droppedItems.push_back(di);
            std::cout << "[drop] item " << (int)id_->getItemId() << " en (" << id_->getX() << ","
                      << id_->getY() << ") dropId=" << id_->getDropId() << std::endl;
        } else if (auto* ip = dynamic_cast<ItemPickedUpEvent*>(ev.get())) {
            for (auto it2 = droppedItems.begin(); it2 != droppedItems.end(); ++it2) {
                if (it2->dropId == ip->getDropId()) {
                    droppedItems.erase(it2);
                    break;
                }
            }
            std::cout << "[pickup] dropId=" << ip->getDropId() << " levantado" << std::endl;
        } else if (auto* pd = dynamic_cast<PlayerDiedEvent*>(ev.get())) {
            auto it = otherPlayers.find(pd->getId());
            if (it != otherPlayers.end()) {
                it->second.ghost = true;
                std::cout << "[muerte] " << it->second.name << " murió" << std::endl;
            } else {
                // id no está entre los otros → soy yo.
                localGhost = true;
                std::cout << "[muerte] moriste — usá /resucitar" << std::endl;
            }
        } else if (auto* pr = dynamic_cast<PlayerRevivedEvent*>(ev.get())) {
            auto it = otherPlayers.find(pr->getId());
            if (it != otherPlayers.end()) {
                it->second.ghost = false;
                it->second.targetX = static_cast<float>(pr->getX());
                it->second.targetY = static_cast<float>(pr->getY());
                it->second.visual.x = it->second.targetX;
                it->second.visual.y = it->second.targetY;
                std::cout << "[revivió] " << it->second.name << std::endl;
            } else {
                localGhost = false;
                player.x = static_cast<float>(pr->getX());
                player.y = static_cast<float>(pr->getY());
                lastTileX = pr->getX();
                lastTileY = pr->getY();
                std::cout << "[revivió] volviste a la vida en ("
                          << pr->getX() << "," << pr->getY() << ")" << std::endl;
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
    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer.Get(), &screenW, &screenH);

    // Fondo del HUD: cubre toda la franja derecha. El inventario va encima,
    // dejando el fondo visible arriba y abajo (espacio para HP/Mana/Exp/oro/nivel).
    // El PNG es escala de grises; color mod multiplica cada pixel, así que el
    // blanco toma el tinte (azul) y el negro queda negro.
    // Escalamos el HUD para mantener su proporción en ventana fija y fullscreen.
    float scale = uiScale();
    const int hudW = hudPanelW();

    SDL2pp::Texture& fondo = cache.get("/Pantallas/Fondo_inventario.png");
    fondo.SetColorMod(120, 160, 255);
    renderer.Copy(fondo, SDL2pp::NullOpt, SDL2pp::Rect(screenW - hudW, 0, hudW, screenH));

    // Left panel of the PNG is 5 cols x 7 rows in ~246x240px source pixels.
    // Crop to 3 cols x 5 rows and scale up so cells appear bigger.
    const int INV_W = (int)(210 * scale);
    const int INV_H = (int)(255 * scale);

    int invX = screenW - hudW + (hudW - INV_W) / 2;
    int invY = screenH / 2 - INV_H / 2;

    renderer.Copy(cache.get("/Pantallas/Inventario_completo.png"),
                  SDL2pp::Rect(0, 0, 140, 175),
                  SDL2pp::Rect(invX, invY, INV_W, INV_H));

    // Items sobre el grid. El inventario visible es 3 cols x 5 rows = 15 slots;
    // inventoryItems trae solo las slots ocupadas, en orden (fila por fila).
    static constexpr int GRID_COLS = 3;
    static constexpr int GRID_ROWS = 5;
    // Items_inventario.png: 31 cols x 10 rows de iconos cuadrados (~33px).
    static constexpr int SHEET_COLS = 31;
    static constexpr float SHEET_CELL = 1024.0f / SHEET_COLS;  // px por icono en el sheet

    SDL2pp::Texture& itemsTex = cache.get("/Pantallas/Items_inventario.png");
    float cw = INV_W / (float)GRID_COLS;
    float ch = INV_H / (float)GRID_ROWS;
    int pad = (int)(4 * scale);  // margen para que el icono entre dentro del recuadro

    for (size_t i = 0; i < inventoryItems.size() && i < GRID_COLS * GRID_ROWS; i++) {
        uint8_t itemId = inventoryItems[i];
        if (itemId == 0)
            continue;
        // itemId es 1-based (ver items.toml); el sheet es row-major desde 0.
        int cell = itemId - 1;
        int srcX = (int)((cell % SHEET_COLS) * SHEET_CELL);
        int srcY = (int)((cell / SHEET_COLS) * SHEET_CELL);

        int col = i % GRID_COLS;
        int row = i / GRID_COLS;
        int dstX = invX + (int)(col * cw) + pad;
        int dstY = invY + (int)(row * ch) + pad;
        int dstW = (int)cw - 2 * pad;
        int dstH = (int)ch - 2 * pad;

        renderer.Copy(itemsTex, SDL2pp::Rect(srcX, srcY, (int)SHEET_CELL, (int)SHEET_CELL),
                      SDL2pp::Rect(dstX, dstY, dstW, dstH));
    }
}
