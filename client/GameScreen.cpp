#include "GameScreen.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <utility>

#include "../common/Communication/events/client_events.h"
#include "../common/Communication/message_types.h"

#include "equipment_sprites.h"
#include "item_sprites.h"
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
        gm.tiles[i].textureId = cell.textureId;
        if (cell.obstacleId != 0) {
            gm.tiles[i].blocked = true;
            gm.tiles[i].obstacleType = static_cast<ObstacleCode>(cell.obstacleId);
        } else {
            gm.tiles[i].blocked = false;
        }
    }
    // Obstáculos colocados
    const auto& obstacles = m.getObstacles();
    gm.obstacles.reserve(obstacles.size());
    for (const auto& o: obstacles) {
        gm.obstacles.push_back({static_cast<ObstacleCode>(o.type), o.x, o.y, o.w, o.h});
    }
    return gm;
}

// Convierte un SpriteRow (sprite) al MoveDirection del wire.
MoveDirection spriteDirToWire(SpriteRow d) {
    switch (d) {
        case SpriteRow::UP: return MoveDirection::TOP;
        case SpriteRow::DOWN: return MoveDirection::BOTTOM;
        case SpriteRow::LEFT: return MoveDirection::LEFT;
        case SpriteRow::RIGHT: return MoveDirection::RIGHT;
    }
    return MoveDirection::BOTTOM;
}

}  // namespace

// Forward decl: la implementacion vive mas abajo, junto al miembro
// applyEquippedVisuals que la usa.
static void applyEquipmentToVisual(Player_& visual, const std::array<uint8_t, 4>& equipped,
                                   int baseSkin);

// tile del servidor (donde caen los pies) → coords del Player.
// p.x e p.y = tile del jugador, así el sprite queda centrado en la celda.
static void tileToPlayerCoords(int16_t tileX, int16_t tileY, Player_& p) {
    p.x = static_cast<float>(tileX);
    p.y = static_cast<float>(tileY);
}

// Mapea la dirección wire (3=TOP, 4=BOTTOM, 5=LEFT, 6=RIGHT) a la SpriteRow
// del cliente (que usa otros valores porque son índices de fila en el spritesheet).
static SpriteRow wireDirToSpriteDir(uint8_t wireDir) {
    switch (wireDir) {
        case 3:
            return SpriteRow::UP;
        case 4:
            return SpriteRow::DOWN;
        case 5:
            return SpriteRow::LEFT;
        case 6:
            return SpriteRow::RIGHT;
        default:
            return SpriteRow::DOWN;
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
    baseSkin = this->player.skin;

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

    // Camara centrada en el area jugable (sin contar el HUD derecho ni la
    // caja de chat de arriba), para que el sprite no quede tapado.
    float playW = screenW - hudPanelW();
    float camX = player.x * TILE_SIZE - playW / 2.0f + TILE_SIZE / 2.0f;
    float camY = player.y * TILE_SIZE - (screenH + chatBoxH()) / 2.0f + TILE_SIZE / 2.0f;

    // Clampear camara al mapa
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

    renderStatsBar();
    renderInventoryPanel();
    renderChat();

    renderer.Present();
}

bool GameScreen::handleEvents(float dt) {
    // Para resolver clicks en coords de mundo necesitamos la camara (misma que render()).
    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer.Get(), &screenW, &screenH);
    float playW = screenW - hudPanelW();
    float camX = player.x * TILE_SIZE - playW / 2.0f + TILE_SIZE / 2.0f;
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
                        addChatLine("> " + chatBuffer);
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
        // Doble click sobre un slot del inventario → equipar/desequipar ese item.
        // SDL marca el segundo click de un doble click con clicks == 2.
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT &&
            e.button.clicks == 2) {
            int slot = inventorySlotAt(e.button.x, e.button.y);
            if (slot >= 0 && slot < (int)inventoryItems.size()) {
                uint8_t itemId = inventoryItems[slot];
                // Si el item ya está equipado, el slotType es el índice en
                // equippedItems (0=arma,1=armor,2=casco,3=escudo) → desequipar.
                // Si no, equipar por inventory slot.
                int slotType = equippedSlotTypeOf(itemId);
                if (slotType >= 0) {
                    std::cout << "[inv] doble click slot=" << slot << " itemId=" << (int)itemId
                              << " → UNEQUIP slotType=" << slotType << std::endl;
                    clientEvents.push(std::make_shared<UnequipItemEvent>(
                            static_cast<uint8_t>(slotType)));
                } else {
                    std::cout << "[inv] doble click slot=" << slot << " itemId=" << (int)itemId
                              << " → EQUIP" << std::endl;
                    clientEvents.push(std::make_shared<EquipItemEvent>(static_cast<uint8_t>(slot)));
                }
                continue;  // consumido por el inventario, no es un ataque.
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
                    spawnProjectile(op.visual.x + HEAD_OFFSET, op.visual.y + CHEST_OFFSET);
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
                        bool isFriendly = (n.visual.type == NpcCode::MERCHANT ||
                                           n.visual.type == NpcCode::BANKER ||
                                           n.visual.type == NpcCode::PRIEST);
                        if (isFriendly) {
                            clientEvents.push(std::make_shared<SelectNpcEvent>(
                                    static_cast<uint16_t>(entry.first)));
                        } else {
                            clientEvents.push(std::make_shared<AttackEvent>(
                                    /*targetType=*/1, static_cast<uint16_t>(entry.first)));
                            spawnProjectile(n.visual.x + HEAD_OFFSET, n.visual.y + CHEST_OFFSET);
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

    // Si estamos en medio del snap de reconciliacion, ignoramos teclas
    if (snapping) {
        player.moving = false;
        return true;
    }

    // Movimiento continuo con teclas sostenidas (level-triggered).
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    float dx = 0, dy = 0;

    if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
        dy = -PLAYER_MOVE_SPEED * dt;
        player.dir = SpriteRow::UP;
    } else if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
        dy = PLAYER_MOVE_SPEED * dt;
        player.dir = SpriteRow::DOWN;
    } else if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
        dx = -PLAYER_MOVE_SPEED * dt;
        player.dir = SpriteRow::LEFT;
    } else if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
        dx = PLAYER_MOVE_SPEED * dt;
        player.dir = SpriteRow::RIGHT;
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

    // Avanza el snap suave hacia el tile que dice el server. t va de 0 a 1.
    if (snapping) {
        snapElapsed += dt;
        float t = snapElapsed / SNAP_DURATION;
        if (t >= 1.0f) {
            player.x = snapToX;
            player.y = snapToY;
            snapping = false;
            // Sin esto, notifyTileChange dispararia un MovementEvent fantasma.
            lastTileX = (int)(player.x + HEAD_OFFSET);
            lastTileY = (int)(player.y + FEET_OFFSET);
        } else {
            player.x = snapFromX + (snapToX - snapFromX) * t;
            player.y = snapFromY + (snapToY - snapFromY) * t;
        }
    }

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
        arrow.age += dt;
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
    // NPCs (criaturas y friendlies) tambien bloquean. Usamos el TARGET porque el server valida que no puedan caminar hacia un tile ocupado por otro NPC, así evitamos que se amontonen visualmente.
    for (const auto& npcEntry: npcs) {
        const auto& rn = npcEntry.second;
        if (!rn.alive) continue;
        int nx = (int)(rn.targetX + HEAD_OFFSET);
        int ny = (int)(rn.targetY + FEET_OFFSET);
        if (nx == tileX && ny == tileY) return true;
    }
    return false;
}

void GameScreen::consumeServerEvents() {
    std::shared_ptr<ServerEvent> ev;
    while (serverEvents.try_pop(ev)) {
        if (auto* mapEvent = dynamic_cast<MapEvent*>(ev.get())) {
            map = convertToGameMap(*mapEvent);
            otherPlayers.clear();
            npcs.clear();
            droppedItems.clear();
            bloodEffects.clear();
            arrows.clear();
        } else if (auto* np = dynamic_cast<NewPlayerEvent*>(ev.get())) {
            OtherPlayer op;
            tileToPlayerCoords(np->getX(), np->getY(), op.visual);
            op.targetX = static_cast<float>(np->getX());
            op.targetY = static_cast<float>(np->getY());
            op.visual.dir = wireDirToSpriteDir(np->getDir());
            op.visual.skin = np->getSkin();
            op.baseSkin = np->getSkin();  // skin sin armor, para volver al desequipar.
            op.name = np->getName();
            otherPlayers[np->getId()] = std::move(op);
        } else if (auto* mr = dynamic_cast<MoveRejectedEvent*>(ev.get())) {
            // Server nos dice donde estamos realmente. Arrancamos el snap suave hacia ese tile (update() lo anima).
            snapping = true;
            snapElapsed = 0.0f;
            snapFromX = player.x;
            snapFromY = player.y;
            snapToX = static_cast<float>(mr->getX());
            snapToY = static_cast<float>(mr->getY());
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
            // Otro jugador equipo/desequipo algo (itemId=0 → desequipo).
            // Actualizamos su slot y re-volcamos visuales.
            auto it = otherPlayers.find(eq->getPlayerId());
            if (it != otherPlayers.end() && eq->getSlot() < 4) {
                it->second.equippedItems[eq->getSlot()] = eq->getItemId();
                applyEquipmentToVisual(it->second.visual, it->second.equippedItems,
                                       it->second.baseSkin);
            }
        } else if (auto* inv = dynamic_cast<InventoryUpdateEvent*>(ev.get())) {
            inventoryItems.clear();
            for (uint8_t id: inv->getItems()) {
                if (id != 0) inventoryItems.push_back(id);
            }
            // itemIds equipados por slotType (0=arma,1=armor,2=casco,3=escudo).
            // 0 = ese slot de equipo está vacío.
            equippedItems[0] = inv->getEquippedWeapon();
            equippedItems[1] = inv->getEquippedArmor();
            equippedItems[2] = inv->getEquippedHelmet();
            equippedItems[3] = inv->getEquippedShield();
            applyEquippedVisuals();
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
            rn.visual.type = static_cast<NpcCode>(nn->getType());
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
            if (cb->getAuthorId() == 0) {
                addChatLine("[sistema] " + cb->getText());
            } else {
                addChatLine(cb->getAuthorName() + ": " + cb->getText());
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

void GameScreen::spawnProjectile(float targetX, float targetY) {
    // El itemId del arma equipada decide el tipo de proyectil y su color. Ver
    // items.toml: 4 ash staff, 6 root staff, 7 socketed staff; 8/9 arcos.
    uint8_t weapon = equippedItems[0];

    ArrowProjectile p;
    switch (weapon) {
        case 4:  // ash staff -> flecha mágica, tinte arcano (celeste).
            p.kind = ProjectileKind::MAGIC_ARROW;
            p.tintR = 120; p.tintG = 200; p.tintB = 255;
            break;
        case 6:  // root staff -> misil, tinte natura (verde).
            p.kind = ProjectileKind::MISSILE;
            p.tintR = 120; p.tintG = 230; p.tintB = 90;
            break;
        case 7:  // socketed staff -> explosión animada, tinte ígneo (naranja).
            p.kind = ProjectileKind::EXPLOSION;
            p.tintR = 255; p.tintG = 10; p.tintB = 10;
            break;
        case 8:  // simple bow -> flecha normal de Flechas.png
            p.kind = ProjectileKind::ARROW;
            p.arrowType = 0;
            break;
        case 9:  // composite bow -> flecha azul propia
            p.kind = ProjectileKind::COMPOSITE_ARROW;
            break;
        default:
            return;  // arma cuerpo a cuerpo o sin equipar: no hay proyectil.
    }

    // Origen: pecho del jugador local. Dirección normalizada hacia el target.
    float ox = player.x + HEAD_OFFSET;
    float oy = player.y + CHEST_OFFSET;
    float dx = targetX - ox;
    float dy = targetY - oy;
    float dist = std::sqrt(dx * dx + dy * dy);
    if (dist < 0.001f) return;
    p.x = ox;
    p.y = oy;
    p.vx = dx / dist * ARROW_SPEED;
    p.vy = dy / dist * ARROW_SPEED;
    p.lifetime = dist / ARROW_SPEED;  // muere al llegar al objetivo.
    arrows.push_back(p);
}

void GameScreen::renderStatsBar() {
    // TODO(team-ui): dibujar barra HP/MP/oro/exp/nivel en la esquina superior
    // izquierda a partir de health/maxHealth/mana/maxMana/gold/etc.
}

float GameScreen::uiScale() const {
    int w, h;
    SDL_GetRendererOutputSize(renderer.Get(), &w, &h);
    (void)w;
    return h / (float)BASE_SCREEN_H;
}

int GameScreen::hudPanelW() const { return (int)(HUD_PANEL_W * uiScale()); }

int GameScreen::chatBoxH() const { return (int)(CHAT_BOX_H * uiScale()); }

void GameScreen::addChatLine(const std::string& line) {
    chatHistory.push_back(line);
    if (chatHistory.size() > MAX_CHAT_LINES) {
        chatHistory.erase(chatHistory.begin(),
                          chatHistory.end() - static_cast<long>(MAX_CHAT_LINES));
    }
}

void GameScreen::renderInventoryPanel() {
    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer.Get(), &screenW, &screenH);

    float scale = uiScale();
    const int hudW = hudPanelW();

    // Fondo del HUD: cubre toda la franja derecha. El PNG es escala de grises;
    // colorMod tinta el blanco.
    SDL2pp::Texture& fondo = cache.get("/Pantallas/Fondo_inventario.png");
    fondo.SetColorMod(120, 160, 255);
    renderer.Copy(fondo, SDL2pp::NullOpt, SDL2pp::Rect(screenW - hudW, 0, hudW, screenH));

    // Panel del inventario: recorte 4 cols x 5 rows del PNG completo, escalado.
    const int INV_W = (int)(210 * scale);
    const int INV_H = (int)(255 * scale);
    int invX = screenW - hudW + (hudW - INV_W) / 2;
    int invY = screenH / 2 - INV_H / 2;
    renderer.Copy(cache.get("/Pantallas/Inventario_completo.png"),
                  SDL2pp::Rect(0, 0, 140, 175),
                  SDL2pp::Rect(invX, invY, INV_W, INV_H));

    // Items sobre el grid: 4 cols x 5 rows = 20 slots. El recorte de cada item
    // sale del mapeo compartido (item_sprites.h), igual que los items del piso.
    static constexpr int GRID_COLS = 4;
    static constexpr int GRID_ROWS = 5;

    float cw = INV_W / (float)GRID_COLS;
    float ch = INV_H / (float)GRID_ROWS;
    int pad = (int)(4 * scale);

    for (size_t i = 0; i < inventoryItems.size() && i < GRID_COLS * GRID_ROWS; i++) {
        uint8_t itemId = inventoryItems[i];
        if (itemId == 0) continue;
        ItemSpriteRef ref = itemSpriteFor(itemId);
        SDL2pp::Texture& itemsTex = cache.get(ref.sheetPath);

        int col = i % GRID_COLS;
        int row = i / GRID_COLS;
        int dstX = invX + (int)(col * cw) + pad;
        int dstY = invY + (int)(row * ch) + pad;
        int dstW = (int)cw - 2 * pad;
        int dstH = (int)ch - 2 * pad;

        // Slot equipado: fondo verde semitransparente + borde, debajo del item.
        if (equippedSlotTypeOf(itemId) >= 0) {
            SDL2pp::Rect cell(dstX, dstY, dstW, dstH);
            renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
            renderer.SetDrawColor(60, 220, 90, 90);
            renderer.FillRect(cell);
            renderer.SetDrawColor(60, 220, 90, 220);
            renderer.DrawRect(cell);
        }

        renderer.Copy(itemsTex, SDL2pp::Rect(ref.srcX, ref.srcY, ref.srcW, ref.srcH),
                      SDL2pp::Rect(dstX, dstY, dstW, dstH));
    }

    // Barras de vida/mana justo debajo del grid del inventario.
    renderResourceBars(invX, invY, INV_W, INV_H, scale);
}

void GameScreen::renderResourceBars(int invX, int invY, int invW, int invH, float scale) {
    // Geometria base (px a 600 de alto), escalada con uiScale().
    const int BAR_H = (int)(22 * scale);
    const int BAR_GAP = (int)(8 * scale);
    const int TOP_MARGIN = (int)(14 * scale);
    const int ICON = (int)(28 * scale);
    const int ICON_GAP = (int)(6 * scale);
    const int BORDER = std::max(2, (int)(3 * scale));

    int iconX = invX;
    int barX = invX + ICON + ICON_GAP;
    int barW = invW - ICON - ICON_GAP;
    int barY = invY + invH + TOP_MARGIN;

    SDL2pp::Texture& frame = cache.get("/Pantallas/barra.png");
    frame.SetColorMod(255, 255, 255);

    // Pequeño helper local para dibujar una barra (frame + relleno + icono + texto).
    auto drawBar = [&](int y, uint16_t value, uint16_t maxValue, SDL_Color fill,
                       SDL2pp::Texture& iconTex, const SDL2pp::Rect& iconSrc) {
        // Marco de madera de fondo.
        renderer.Copy(frame, SDL2pp::NullOpt, SDL2pp::Rect(barX, y, barW, BAR_H));

        // Relleno proporcional, recortado dentro del marco.
        float frac = maxValue > 0 ? (float)value / (float)maxValue : 0.0f;
        frac = std::clamp(frac, 0.0f, 1.0f);
        int innerW = barW - 2 * BORDER;
        int fillW = (int)(innerW * frac);
        renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
        // Fondo oscuro de la porción vacía.
        renderer.SetDrawColor(0, 0, 0, 140);
        renderer.FillRect(SDL2pp::Rect(barX + BORDER, y + BORDER, innerW, BAR_H - 2 * BORDER));
        if (fillW > 0) {
            renderer.SetDrawColor(fill.r, fill.g, fill.b, 235);
            renderer.FillRect(SDL2pp::Rect(barX + BORDER, y + BORDER, fillW, BAR_H - 2 * BORDER));
        }

        // Icono a la izquierda, centrado verticalmente respecto a la barra.
        int iconY = y + (BAR_H - ICON) / 2;
        renderer.Copy(iconTex, iconSrc, SDL2pp::Rect(iconX, iconY, ICON, ICON));

        // Texto "value/max" centrado sobre la barra.
        std::string label = std::to_string(value) + "/" + std::to_string(maxValue);
        try {
            SDL_Color white = {255, 255, 255, 255};
            auto surface = chatFont.RenderUTF8_Blended(label, white);
            SDL2pp::Texture tex(renderer, surface);
            int tw = tex.GetWidth();
            int th = tex.GetHeight();
            renderer.Copy(tex, SDL2pp::NullOpt,
                          SDL2pp::Rect(barX + (barW - tw) / 2, y + (BAR_H - th) / 2, tw, th));
        } catch (...) {}
    };

    // Vida: cruz roja de Signo_vida.png (recorte de la primera cruz, arriba-izq).
    SDL2pp::Texture& vida = cache.get("/Pantallas/Signo_vida.png");
    drawBar(barY, health, maxHealth, SDL_Color{200, 40, 40, 255}, vida,
            SDL2pp::Rect(3, 3, 24, 29));

    // Mana: orbe azul recortado de Mana.png.
    SDL2pp::Texture& manaIcon = cache.get("/Pantallas/Mana.png");
    int manaY = barY + BAR_H + BAR_GAP;
    drawBar(manaY, mana, maxMana, SDL_Color{50, 110, 230, 255}, manaIcon,
            SDL2pp::Rect(0, 160, 128, 128));

    // Oro: icono (pila de monedas) + cantidad, debajo del mana. Sin barra de
    // relleno: es un contador, no un recurso acotado.
    int goldY = manaY + BAR_H + BAR_GAP;
    int iconY = goldY + (BAR_H - ICON) / 2;
    renderer.Copy(cache.get("/Pantallas/Items_recolectables.png"),
                  SDL2pp::Rect(0, 320, 32, 32), SDL2pp::Rect(iconX, iconY, ICON, ICON));
    try {
        SDL_Color goldColor = {255, 215, 60, 255};
        auto surface = chatFont.RenderUTF8_Blended(std::to_string(gold), goldColor);
        SDL2pp::Texture tex(renderer, surface);
        int th = tex.GetHeight();
        renderer.Copy(tex, SDL2pp::NullOpt,
                      SDL2pp::Rect(iconX + ICON + ICON_GAP, goldY + (BAR_H - th) / 2,
                                   tex.GetWidth(), th));
    } catch (...) {}

    // Experiencia + nivel, debajo del oro. El nivel va como "badge" en el slot
    // del icono; la barra muestra el progreso hacia el siguiente nivel.
    int expY = goldY + BAR_H + BAR_GAP;

    // Badge de nivel: cuadrado oscuro con borde dorado y el número centrado.
    int badgeY = expY + (BAR_H - ICON) / 2;
    SDL2pp::Rect badge(iconX, badgeY, ICON, ICON);
    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    renderer.SetDrawColor(20, 20, 30, 220);
    renderer.FillRect(badge);
    renderer.SetDrawColor(255, 215, 60, 255);
    renderer.DrawRect(badge);
    try {
        SDL_Color lvlColor = {255, 235, 150, 255};
        auto surface = chatFont.RenderUTF8_Blended(std::to_string((int)level), lvlColor);
        SDL2pp::Texture tex(renderer, surface);
        int tw = tex.GetWidth();
        int th = tex.GetHeight();
        renderer.Copy(tex, SDL2pp::NullOpt,
                      SDL2pp::Rect(iconX + (ICON - tw) / 2, badgeY + (ICON - th) / 2, tw, th));
    } catch (...) {}

    // Barra de experiencia: marco de madera + relleno verde proporcional.
    renderer.Copy(frame, SDL2pp::NullOpt, SDL2pp::Rect(barX, expY, barW, BAR_H));
    float expFrac = nextLevelExp > 0 ? (float)experience / (float)nextLevelExp : 0.0f;
    expFrac = std::clamp(expFrac, 0.0f, 1.0f);
    int innerW = barW - 2 * BORDER;
    renderer.SetDrawColor(0, 0, 0, 140);
    renderer.FillRect(SDL2pp::Rect(barX + BORDER, expY + BORDER, innerW, BAR_H - 2 * BORDER));
    int expFillW = (int)(innerW * expFrac);
    if (expFillW > 0) {
        renderer.SetDrawColor(120, 210, 70, 235);
        renderer.FillRect(SDL2pp::Rect(barX + BORDER, expY + BORDER, expFillW, BAR_H - 2 * BORDER));
    }
    try {
        SDL_Color white = {255, 255, 255, 255};
        std::string label = std::to_string(experience) + "/" + std::to_string(nextLevelExp);
        auto surface = chatFont.RenderUTF8_Blended(label, white);
        SDL2pp::Texture tex(renderer, surface);
        int tw = tex.GetWidth();
        int th = tex.GetHeight();
        renderer.Copy(tex, SDL2pp::NullOpt,
                      SDL2pp::Rect(barX + (barW - tw) / 2, expY + (BAR_H - th) / 2, tw, th));
    } catch (...) {}
}

// Helper: vuelca los itemIds equipados sobre los campos visuales de un Player_
// (local o remoto). Se llama tras un cambio de equipo (al recibir
// InventoryUpdateEvent para el local, o PlayerEquippedEvent para otros).
static void applyEquipmentToVisual(Player_& visual, const std::array<uint8_t, 4>& equipped,
                                   int baseSkin) {
    // Reset: -1 = sin equipo en ese slot; el cuerpo vuelve al skin base.
    visual.weaponId = -1;
    visual.shieldId = -1;
    visual.helmetId = -1;
    visual.skin = baseSkin;

    for (uint8_t itemId: equipped) {
        EquipVisual v = equipVisualFor(itemId);
        switch (v.slot) {
            case EquipSlot::WEAPON: visual.weaponId = v.index; break;
            case EquipSlot::ARMOR: visual.skin = v.index; break;
            case EquipSlot::HELMET: visual.helmetId = v.index; break;
            case EquipSlot::SHIELD: visual.shieldId = v.index; break;
            case EquipSlot::NONE: break;
        }
    }
}

void GameScreen::applyEquippedVisuals() {
    applyEquipmentToVisual(player, equippedItems, baseSkin);
}

int GameScreen::equippedSlotTypeOf(uint8_t itemId) const {
    if (itemId == 0) return -1;  // slot vacío, nunca "equipado".
    for (size_t t = 0; t < equippedItems.size(); t++) {
        if (equippedItems[t] == itemId) return (int)t;
    }
    return -1;
}

int GameScreen::inventorySlotAt(int mouseX, int mouseY) const {
    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer.Get(), &screenW, &screenH);

    float scale = uiScale();
    const int hudW = hudPanelW();

    // Mismo cálculo de invX/invY/INV_W/INV_H que renderInventoryPanel().
    const int INV_W = (int)(210 * scale);
    const int INV_H = (int)(255 * scale);
    int invX = screenW - hudW + (hudW - INV_W) / 2;
    int invY = screenH / 2 - INV_H / 2;

    // Fuera del rectángulo del grid → no hay slot.
    if (mouseX < invX || mouseX >= invX + INV_W || mouseY < invY || mouseY >= invY + INV_H)
        return -1;

    static constexpr int GRID_COLS = 4;
    static constexpr int GRID_ROWS = 5;
    float cw = INV_W / (float)GRID_COLS;
    float ch = INV_H / (float)GRID_ROWS;

    int col = (int)((mouseX - invX) / cw);
    int row = (int)((mouseY - invY) / ch);
    if (col < 0 || col >= GRID_COLS || row < 0 || row >= GRID_ROWS)
        return -1;
    return row * GRID_COLS + col;
}

void GameScreen::renderChat() {
    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer.Get(), &screenW, &screenH);
    (void)screenH;

    float scale = uiScale();
    const int BAR_W = screenW - hudPanelW();
    const int boxH = chatBoxH();

    // Fondo negro semi-transparente.
    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    renderer.SetDrawColor(0, 0, 0, 160);
    renderer.FillRect(SDL2pp::Rect(0, 0, BAR_W, boxH));

    SDL_Color color = {255, 255, 255, 255};
    int lineH = (int)(CHAT_LINE_H * scale);
    int pad = (int)(CHAT_PAD * scale);

    // Historial: hasta MAX_CHAT_LINES desde arriba.
    for (size_t i = 0; i < chatHistory.size(); i++) {
        const std::string& text = chatHistory[i];
        if (text.empty()) continue;
        try {
            auto surface = chatFont.RenderUTF8_Blended(text, color);
            SDL2pp::Texture tex(renderer, surface);
            int tw = tex.GetWidth();
            int th = tex.GetHeight();
            renderer.Copy(tex, SDL2pp::NullOpt,
                          SDL2pp::Rect(pad, pad + (int)i * lineH, tw, th));
        } catch (...) {}
    }

    // Linea de input (solo si chat activo). Prefijo ">".
    if (chatActive) {
        std::string prompt = "> " + chatBuffer + "_";
        try {
            auto surface = chatFont.RenderUTF8_Blended(prompt, color);
            SDL2pp::Texture tex(renderer, surface);
            int tw = tex.GetWidth();
            int th = tex.GetHeight();
            renderer.Copy(tex, SDL2pp::NullOpt,
                          SDL2pp::Rect(pad, boxH - pad - th, tw, th));
        } catch (...) {}
    }
}
