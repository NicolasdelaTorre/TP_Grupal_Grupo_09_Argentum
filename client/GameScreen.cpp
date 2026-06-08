#include "GameScreen.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <utility>

#include "tile_textures.h"

// Ancho de la franja del HUD a la derecha (inventario + fondo). El área jugable
// es screenW - HUD_PANEL_W, y la cámara centra al jugador ahí (no en toda la
// pantalla) para que el sprite quede centrado a la izquierda del inventario.
static constexpr int HUD_PANEL_W = 230;

namespace {

// ReceivedMap (wire) → GameMap (lo que pinta el MapRenderer).
GameMap convertToGameMap(const ReceivedMap& m) {
    GameMap gm;
    gm.width = m.width;
    gm.height = m.height;
    gm.tiles.resize(m.cells.size());
    for (size_t i = 0; i < m.cells.size(); i++) {
        const auto& cell = m.cells[i];
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
                       const ReceivedMap& mapData, Position spawn, Player player):
        renderer(renderer),
        chatTtf(),
        chatFont(assetsPath + "/font.ttf", 14),
        cache(renderer, assetsPath),
        mapRenderer(renderer, cache),
        map(convertToGameMap(mapData)),
        events_queue(events_queue),
        server_queue(server_queue),
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

        // Mientras el chat está abierto, capturamos todo el teclado para escribir
        // el comando. Enter lo manda, Escape cancela, Backspace borra.
        if (chatActive) {
            if (e.type == SDL_TEXTINPUT) {
                chatInput += e.text.text;
            } else if (e.type == SDL_KEYDOWN) {
                SDL_Keycode sym = e.key.keysym.sym;
                if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) {
                    submitChat();
                    chatInput.clear();
                    chatActive = false;
                    SDL_StopTextInput();
                } else if (sym == SDLK_ESCAPE) {
                    chatInput.clear();
                    chatActive = false;
                    SDL_StopTextInput();
                } else if (sym == SDLK_BACKSPACE) {
                    if (!chatInput.empty())
                        chatInput.pop_back();
                }
            }
            continue;  // chat abierto: ignoramos movimiento/clicks
        }

        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE)
                return false;
            // Enter abre la barra de chat para tipear un comando.
            if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
                chatActive = true;
                SDL_StartTextInput();
                continue;
            }
        }
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            // Click sobre otro player → ATTACK con id del target.
            // Server valida si el atacante tiene arma equipada, si es de rango
            // o si está adyacente (para melee), etc. El cliente no decide nada.
            int clickTileX = (int)((e.button.x + camX) / TILE_SIZE);
            int clickTileY = (int)((e.button.y + camY) / TILE_SIZE);
            for (const auto& entry: otherPlayers) {
                const auto& op = entry.second;
                int opTileX = (int)(op.visual.x + HEAD_OFFSET);
                int opTileY = (int)(op.visual.y + FEET_OFFSET);
                if (opTileX == clickTileX && opTileY == clickTileY) {
                    events_queue.push("ATTACK:0:" + std::to_string(entry.first));
                    break;
                }
            }
        }
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
            // NEW_PLAYER:<id>:<x>:<y>:<dir>:<skin>:<name>
            size_t c1 = event.find(':');
            size_t c2 = event.find(':', c1 + 1);
            size_t c3 = event.find(':', c2 + 1);
            size_t c4 = event.find(':', c3 + 1);
            size_t c5 = event.find(':', c4 + 1);
            size_t c6 = event.find(':', c5 + 1);
            if (c6 == std::string::npos)
                continue;
            int id = std::stoi(event.substr(c1 + 1, c2 - c1 - 1));
            int16_t x = static_cast<int16_t>(std::stoi(event.substr(c2 + 1, c3 - c2 - 1)));
            int16_t y = static_cast<int16_t>(std::stoi(event.substr(c3 + 1, c4 - c3 - 1)));
            uint8_t dir = static_cast<uint8_t>(std::stoi(event.substr(c4 + 1, c5 - c4 - 1)));
            uint8_t skin = static_cast<uint8_t>(std::stoi(event.substr(c5 + 1, c6 - c5 - 1)));
            OtherPlayer op;
            tileToPlayerCoords(x, y, op.visual);
            op.targetX = static_cast<float>(x);
            op.targetY = static_cast<float>(y);
            op.visual.dir = wireDirToSpriteDir(dir);
            op.visual.skin = skin;
            op.name = event.substr(c6 + 1);
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
