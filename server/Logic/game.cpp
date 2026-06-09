#include "game.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "NPC/creature.h"

Game::Game(Map& world):
        map(world), playerSpawn(map.getPlayerSpawn(0)), parser(BinaryParser()),
        bank(0, "global", 0, 0) {}

bool Game::addPlayer(int playerId, const std::string& name, RaceCode race, ClassCode class_) {
    Position spawn;

    if (!parser.checkPlayerExists(name)) {
        spawn = findSpawnPosition();
        players.emplace(playerId, Player(name, spawn, race, class_));
        parser.savePlayerData(name, players.at(playerId).getData());
    } else {
        players.emplace(playerId, Player(parser.loadPlayerData(name), name));
        spawn = players.at(playerId).getPosition();
    }

    // Cargar/crear la cuenta del banco para este jugador.
    bank.addPlayer(name);

    map.placeEntity(playerId, spawn.x, spawn.y, true, 0);

    std::cout << "Hi " << name << " (" << Race::toString(race) << "/" << PlayerClass::toString(class_)
              << ") spawned at (" << spawn.x << ", " << spawn.y << ")" << std::endl;

    return true;
}

bool Game::turnPlayer(int playerId, MoveDirection direction) {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        return false;
    }
    itPlayer->second.setDirection(static_cast<uint8_t>(direction));
    return true;
}

Position Game::findSpawnPosition() const {
    int maxRadius = std::max(map.getWidth(0), map.getHeight(0));
    for (int radius = 0; radius < maxRadius; radius++) {
        for (int offsetY = -radius; offsetY <= radius; offsetY++) {
            for (int offsetX = -radius; offsetX <= radius; offsetX++) {
                if (radius > 0 && std::abs(offsetX) != radius && std::abs(offsetY) != radius)
                    continue;
                Position candidate{static_cast<int16_t>(playerSpawn.x + offsetX),
                                   static_cast<int16_t>(playerSpawn.y + offsetY)};
                if (map.isWalkable(candidate.x, candidate.y, 0) && isPositionFree(candidate)) {
                    return candidate;
                }
            }
        }
    }

    throw std::runtime_error("Game Error: no free cell available for new player");
}

bool Game::isPositionFree(Position pos) const {
    for (const auto& [id, player]: players) {
        if (player.getX() == pos.x && player.getY() == pos.y) {
            return false;
        }
    }
    return true;
}

Position Game::getPlayerPosition(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    return it->second.getPosition();
}

const std::string& Game::getPlayerName(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    return it->second.getName();
}

uint8_t Game::getPlayerDirection(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    return it->second.getDirection();
}

uint8_t Game::getPlayerSkin(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    return it->second.getData().bodySkinId;
}

uint16_t Game::getPlayerHealth(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    return it->second.getData().health;
}

uint16_t Game::getPlayerMaxHealth(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    return it->second.getMaxHealth();
}

uint8_t Game::getPlayerLevel(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    return it->second.getData().level;
}

uint16_t Game::getPlayerMana(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    return it->second.getData().mana;
}

uint16_t Game::getPlayerMaxMana(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    return it->second.getMaxMana();
}

uint32_t Game::getPlayerGold(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    return it->second.getData().gold;
}

uint32_t Game::getPlayerExperience(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    return it->second.getData().experience;
}

uint32_t Game::getPlayerNextLevelExp(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    // TODO(team-gameplay): devolver el límite de exp para el próximo nivel.
    // Fórmula del enunciado: 1000 * Nivel^1.8 (o como lo decida StatsDefinition).
    return 0;
}

bool Game::hasPlayer(int playerId) const { return players.find(playerId) != players.end(); }

bool Game::isPlayerGhost(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) return false;
    return it->second.getData().isGhost;
}

std::vector<int> Game::getPlayerIds() const {
    std::vector<int> ids;
    ids.reserve(players.size());
    for (const auto& [id, _]: players) {
        ids.push_back(id);
    }
    return ids;
}

void Game::updatePlayerData(int playerId) {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }

    parser.updatePlayerData(it->second.getName(), it->second.getData());
}

void Game::removePlayer(int playerId) {
    auto it = players.find(playerId);
    if (it != players.end()) {
        Position p = it->second.getPosition();
        map.removePlayer(p.x, p.y, it->second.getMapId());
    }
    updatePlayerData(playerId);
    players.erase(playerId);
}

bool Game::movePlayer(int playerId, MoveDirection direction) {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }

    Player& player = itPlayer->second;
    Position next = player.getPosition();

    switch (direction) {
        case MoveDirection::TOP: next.y -= 1; break;
        case MoveDirection::BOTTOM: next.y += 1; break;
        case MoveDirection::LEFT: next.x -= 1; break;
        case MoveDirection::RIGHT: next.x += 1; break;
        default: return false;
    }

    uint8_t mapId = player.getMapId();
    if (!map.isWalkable(next.x, next.y, mapId)) {
        std::cout << "Player can't move in that direction (blocked)" << std::endl;
        return false;
    }

    if (!isPositionFree(next)) {
        std::cout << "Player can't move in that direction (occupied by another player)"
                  << std::endl;
        return false;
    }

    Position old = player.getPosition();
    player.move(next);
    player.setDirection(static_cast<uint8_t>(direction));
    map.moveEntity(playerId, old.x, old.y, next.x, next.y, true, mapId);
    checkEntry(playerId);
    return true;
}

void Game::checkEntry(int playerId) {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }

    Position pos = itPlayer->second.getPosition();

    if (map.checkIfThePositionHasAnEntry(pos.x, pos.y, itPlayer->second.getMapId())) {
        // El jugador pisó una entrada a dungeon o una salida (si ya está en
        // una dungeon). Lo sacamos del mapa actual y lo metemos al destino.
        uint8_t currentMapId = itPlayer->second.getMapId();
        map.removePlayer(pos.x, pos.y, currentMapId);

        std::string mapId = map.getMapId(pos.x, pos.y);

        if (!mapId.empty()) {
            // Si veníamos del overworld vamos a la dungeon; si veníamos de
            // una dungeon, salimos al overworld.
            if (currentMapId == 0)
                map.placePlayerIntoTheDungeon(playerId, mapId);
            else
                map.placePlayerIntoTheOverworld(playerId, currentMapId);
            itPlayer->second.changeMapId(static_cast<uint8_t>((mapId[mapId.size() - 1])) - '0');
            Position newPosition = map.getEntrySpawnPosition(mapId);
            itPlayer->second.move(newPosition);
        }
    }
}

std::shared_ptr<AttackResultEvent> Game::processAttack(int playerId, uint8_t targetType,
                                                      uint16_t targetId) {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }

    if (!itPlayer->second.isEquipped() || !itPlayer->second.isAlive()) {
        return nullptr;
    }

    if (targetType != 0 && targetType != 1) {
        throw std::runtime_error(
                "Game Error: malformed attack command (expected player.id or npc.id)");
    }

    bool targetPlayer = (targetType == 0);
    uint8_t mapId = itPlayer->second.getMapId();
    uint8_t entityId;
    if (itPlayer->second.hasLongDistanceWeapon())
        entityId = map.entityInDistance(itPlayer->second.getX(), itPlayer->second.getY(),
                                        targetPlayer, mapId);
    else
        entityId = map.nextEntity(itPlayer->second.getX(), itPlayer->second.getY(), targetPlayer,
                                  mapId);

    if (entityId != targetId)
        return nullptr;

    uint16_t attackerId = static_cast<uint16_t>(playerId);

    if (targetType == 0) {
        // target = player
        auto itTarget = players.find(entityId);
        if (itTarget == players.end()) {
            throw std::runtime_error("Game Error: player in sight not found");
        }
        if (!itTarget->second.isAlive()) {
            return nullptr;
        }
        if (tryEvade(playerId, static_cast<int>(targetId))) {
            return std::make_shared<AttackResultEvent>(attackerId, targetType, targetId, 0, false);
        }
        uint16_t damage = itPlayer->second.dealDamage();
        itTarget->second.receiveDamage(damage);
        return std::make_shared<AttackResultEvent>(attackerId, targetType, targetId, damage, true);
    }

    // target = npc
    uint16_t damage = itPlayer->second.dealDamage();
    Creature* npc = map.getNPC(targetId);
    npc->receiveDamage(damage);
    return std::make_shared<AttackResultEvent>(attackerId, targetType, targetId, damage, true);
}

bool Game::tryEvade(int /*attackerId*/, int /*targetId*/) const {
    // TODO(team-gameplay): implementar fórmula real.
    return false;
}

void Game::setSkin(int playerId, uint8_t skinId) {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    // TODO(team-gameplay): el segundo parámetro es headId, queda en 0 hasta
    // que se implemente la selección de cabeza.
    itPlayer->second.setSkin(static_cast<int>(skinId), 0);
}

// ── Cheats invocables desde el chat ──────────────────────────────────────

bool Game::cheatToggleInfiniteHealth(int playerId) {
    auto it = players.find(playerId);
    if (it == players.end()) return false;
    bool on = it->second.toggleInfiniteHealth();
    std::cout << "CHEAT vidainf " << (on ? "ON" : "OFF") << " para player=" << playerId << std::endl;
    return true;
}

bool Game::cheatToggleInfiniteMana(int playerId) {
    auto it = players.find(playerId);
    if (it == players.end()) return false;
    bool on = it->second.toggleInfiniteMana();
    std::cout << "CHEAT manainf " << (on ? "ON" : "OFF") << " para player=" << playerId << std::endl;
    return true;
}

bool Game::cheatSuicide(int playerId) {
    auto it = players.find(playerId);
    if (it == players.end()) return false;
    it->second.kill();
    std::cout << "CHEAT suicidio aplicado a player=" << playerId << std::endl;
    return true;
}

bool Game::cheatLevelUp(int playerId) {
    auto it = players.find(playerId);
    if (it == players.end()) return false;
    it->second.levelUp();
    std::cout << "CHEAT levelup aplicado a player=" << playerId << std::endl;
    return true;
}

bool Game::cheatAddGold(int playerId, uint32_t amount) {
    auto it = players.find(playerId);
    if (it == players.end()) return false;
    it->second.addGold(amount);
    std::cout << "CHEAT gold +" << amount << " para player=" << playerId << std::endl;
    return true;
}

// Mapeo de itemId → nombre del item, según items.toml. Lo usan cheatSpawnItem
// y otros stubs que necesitan crear un item por id.
static const char* itemNameById(uint8_t id) {
    switch (id) {
        case 1: return "Sword";
        case 2: return "Axe";
        case 3: return "Hammer";
        case 4: return "Ash Staff";
        case 5: return "Elven Flute";
        case 6: return "Root Staff";
        case 7: return "Socketed Staff";
        case 8: return "Simple Bow";
        case 9: return "Composite Bow";
        case 10: return "Lether Armor";
        case 11: return "Plate Armor";
        case 12: return "Blue Tunic";
        case 13: return "Hood";
        case 14: return "Iron Helmet";
        case 15: return "Turtle Shield";
        case 16: return "Iron Shield";
        case 17: return "Wizard Hat";
        case 18: return "Health Potion";
        case 19: return "Mana Potion";
        default: return nullptr;
    }
}

// Inverso de itemNameById, case-insensitive: "sword", "Sword" o "SWORD" -> 1.
// Devuelve 0 si no existe.
static uint8_t itemIdByName(const std::string& name) {
    auto toLower = [](std::string s) {
        for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    };
    std::string needle = toLower(name);
    for (uint8_t id = 1; id <= 19; id++) {
        const char* n = itemNameById(id);
        if (n && toLower(n) == needle) return id;
    }
    return 0;
}

bool Game::cheatSpawnItem(int playerId, uint8_t itemId) {
    auto it = players.find(playerId);
    if (it == players.end()) return false;
    const char* name = itemNameById(itemId);
    if (!name) {
        std::cout << "CHEAT spawn item id=" << (int)itemId << " desconocido" << std::endl;
        return false;
    }
    if (!it->second.addItem(name)) {
        std::cout << "CHEAT spawn item id=" << (int)itemId << " inventario lleno" << std::endl;
        return false;
    }
    std::cout << "CHEAT spawn item id=" << (int)itemId << " (" << name << ") para player="
              << playerId << std::endl;
    return true;
}

// ── Interacción con NPCs amigos ──────────────────────────────────
// stubs solo loguean — team-gameplay rellena la lógica real usando las
// clases Merchant/Banker/Priest existentes.

std::vector<std::string> Game::listMerchantInventory(uint8_t npcType) {
    std::cout << "LIST merchant type=" << (int)npcType << " (stub)" << std::endl;
    // TODO(team-gameplay): devolver items reales del merchant según la
    // configuración de items.toml.
    return {"(stub) Sin items disponibles. Implementar listMerchantInventory."};
}

std::vector<std::string> Game::listBankAccount(int playerId, uint8_t /*npcType*/) {
    std::vector<std::string> lines;
    auto it = players.find(playerId);
    if (it == players.end()) return lines;
    const std::string& name = it->second.getName();
    lines.push_back("Oro guardado: " + std::to_string(bank.getGold(name)));
    auto items = bank.getItems(name);
    if (items.empty()) {
        lines.push_back("Sin items guardados");
    } else {
        for (uint8_t id : items) {
            const char* n = itemNameById(id);
            lines.push_back(std::string("- ") + (n ? n : "(desconocido)"));
        }
    }
    return lines;
}

Game::InteractionResult Game::buyFromNpc(int playerId, uint8_t npcType,
                                         const std::string& itemName) {
    auto it = players.find(playerId);
    if (it == players.end()) return {false, "Jugador no existe"};
    std::cout << "BUY player=" << playerId << " npcType=" << (int)npcType << " item=" << itemName
              << " (stub)" << std::endl;
    // TODO(team-gameplay): validar oro, restar precio, addItem al inventario.
    return {true, "Compraste " + itemName + " (stub)"};
}

Game::InteractionResult Game::sellToNpc(int playerId, uint8_t npcType,
                                        const std::string& itemName) {
    auto it = players.find(playerId);
    if (it == players.end()) return {false, "Jugador no existe"};
    std::cout << "SELL player=" << playerId << " npcType=" << (int)npcType << " item=" << itemName
              << " (stub)" << std::endl;
    // TODO(team-gameplay): buscar el item en inventario, removerlo, sumar oro.
    return {true, "Vendiste " + itemName + " (stub)"};
}

Game::InteractionResult Game::depositItemToBank(int playerId, const std::string& itemName) {
    auto it = players.find(playerId);
    if (it == players.end()) return {false, "Jugador no existe"};
    uint8_t itemId = itemIdByName(itemName);
    if (itemId == 0) return {false, "Item desconocido: " + itemName};
    std::string canonical = itemNameById(itemId);
    if (it->second.removeItemByName(canonical) == 0) {
        return {false, "No tenes " + canonical + " en el inventario"};
    }
    if (!bank.depositItem(it->second.getName(), itemId)) {
        // cuenta llena: lo devolvemos al inventario para no perderlo.
        it->second.addItem(canonical);
        return {false, "El banco esta lleno"};
    }
    return {true, "Depositaste " + canonical};
}

Game::InteractionResult Game::depositGoldToBank(int playerId, uint32_t amount) {
    auto it = players.find(playerId);
    if (it == players.end()) return {false, "Jugador no existe"};
    if (amount == 0) return {false, "Cantidad invalida"};
    if (!it->second.removeGold(amount)) {
        return {false, "No tenes suficiente oro"};
    }
    if (!bank.depositGold(it->second.getName(), amount)) {
        // overflow en la cuenta: devolvemos el oro al jugador para no perderlo.
        it->second.addGold(amount);
        return {false, "No se pudo depositar (cuenta llena)"};
    }
    return {true, "Depositaste " + std::to_string(amount) + " de oro"};
}

Game::InteractionResult Game::withdrawItemFromBank(int playerId, const std::string& itemName) {
    auto it = players.find(playerId);
    if (it == players.end()) return {false, "Jugador no existe"};
    uint8_t itemId = itemIdByName(itemName);
    if (itemId == 0) return {false, "Item desconocido: " + itemName};
    std::string canonical = itemNameById(itemId);
    uint8_t got = bank.withdrawItem(it->second.getName(), itemId);
    if (got == 0) return {false, "No tenes " + canonical + " en el banco"};
    if (!it->second.addItem(canonical)) {
        // inventario lleno: lo devolvemos al banco para no perderlo.
        bank.depositItem(it->second.getName(), got);
        return {false, "Tu inventario esta lleno"};
    }
    return {true, "Retiraste " + canonical};
}

Game::InteractionResult Game::withdrawGoldFromBank(int playerId, uint32_t amount) {
    auto it = players.find(playerId);
    if (it == players.end()) return {false, "Jugador no existe"};
    if (amount == 0) return {false, "Cantidad invalida"};
    uint32_t got = bank.withdrawGold(it->second.getName(), amount);
    if (got == 0) return {false, "No tenes esa cantidad en el banco"};
    // addGold respeta el cap OroMax = 100 * Nivel^1.1. El sobrante se pierde
    // o queda en el banco (aca devolvemos al banco lo que no entro).
    uint32_t before = it->second.getData().gold;
    it->second.addGold(got);
    uint32_t added = it->second.getData().gold - before;
    if (added < got) {
        bank.depositGold(it->second.getName(), got - added);
        if (added == 0) return {false, "Ya tenes el oro maximo encima"};
        return {true, "Retiraste " + std::to_string(added) +
                       " (no entraba mas por el cap)"};
    }
    return {true, "Retiraste " + std::to_string(got) + " de oro"};
}

Game::InteractionResult Game::revivePlayer(int playerId) {
    auto it = players.find(playerId);
    if (it == players.end()) return {false, "Jugador no existe"};
    if (!it->second.getData().isGhost) {
        return {false, "Ya estás vivo"};
    }
    // Reaparece en el spawn de la ciudad. resetStats() limpia isGhost y
    // restaura HP/MP. TODO(team-gameplay): tp al sacerdote más cercano según
    // enunciado (proporcional a la distancia).
    Position old = it->second.getPosition();
    Position newPos = findSpawnPosition();
    map.moveEntity(playerId, old.x, old.y, newPos.x, newPos.y, /*isPlayer=*/true,
                   it->second.getMapId());
    it->second.move(newPos);
    it->second.resetStats();
    std::cout << "REVIVE player=" << playerId << " at (" << newPos.x << "," << newPos.y << ")"
              << std::endl;
    return {true, "Volviste a la vida"};
}

Game::InteractionResult Game::healPlayer(int playerId) {
    auto it = players.find(playerId);
    if (it == players.end()) return {false, "Jugador no existe"};
    if (it->second.getData().isGhost) {
        return {false, "Estás muerto, primero resucitá"};
    }
    // resetStats restaura HP/MP a su máximo. Sirve como cura completa.
    it->second.resetStats();
    return {true, "Te curaste"};
}

Game::InteractionResult Game::meditatePlayer(int playerId) {
    auto it = players.find(playerId);
    if (it == players.end()) return {false, "Jugador no existe"};
    if (it->second.getData().isGhost) {
        return {false, "Estás muerto, no podés meditar"};
    }
    // Toggle. El tick de mana lo maneja TurnManager + gameloop::PlayerTurns.
    it->second.switchMeditationState();
    bool nowMeditating = it->second.getMeditationState();
    std::cout << "MEDITATE player=" << playerId << " on=" << nowMeditating << std::endl;
    return {true, nowMeditating ? "Empezaste a meditar" : "Saliste de meditación"};
}

Game::DropResult Game::pickUpItemAt(int playerId) {
    auto it = players.find(playerId);
    if (it == players.end()) return {false, "Jugador no existe", {}};
    Position pos = it->second.getPosition();
    // Buscamos el primer drop en la celda del jugador.
    for (size_t i = 0; i < droppedItems.size(); i++) {
        if (droppedItems[i].x == pos.x && droppedItems[i].y == pos.y) {
            DroppedItemRecord rec = droppedItems[i];
            droppedItems.erase(droppedItems.begin() + i);
            // TODO(team-gameplay): mapear itemId → itemName y llamar
            // it->second.addItem(name). Hoy solo sacamos el drop del piso.
            std::cout << "PICKUP player=" << playerId << " dropId=" << rec.dropId
                      << " itemId=" << (int)rec.itemId << " at (" << rec.x << "," << rec.y << ")"
                      << std::endl;
            return {true, "Levantaste el item (stub)", rec};
        }
    }
    return {false, "No hay nada para levantar acá", {}};
}

Game::DropResult Game::dropItem(int playerId, uint8_t invSlot) {
    auto it = players.find(playerId);
    if (it == players.end()) return {false, "Jugador no existe", {}};
    auto inv = it->second.getInventory();
    if (invSlot >= inv.size()) return {false, "Slot inválido", {}};
    Position pos = it->second.getPosition();
    uint8_t itemId = inv[invSlot].getId();
    std::string itemName = inv[invSlot].getName();
    // TODO(team-gameplay): remover el item del inventario del jugador
    // (player.removeFromSlot(slot)). Hoy el item se queda duplicado.
    DroppedItemRecord rec{nextDropId++, itemId, pos.x, pos.y};
    droppedItems.push_back(rec);
    std::cout << "DROP player=" << playerId << " slot=" << (int)invSlot
              << " name=" << itemName << " dropId=" << rec.dropId << " at (" << rec.x
              << "," << rec.y << ")" << std::endl;
    return {true, "Tiraste " + itemName, rec};
}

bool Game::equipOrUseItem(int playerId, uint8_t invSlot) {
    auto it = players.find(playerId);
    if (it == players.end()) {
        return false;
    }
    bool ok = it->second.equipItem(static_cast<int>(invSlot));
    std::cout << "EQUIP player=" << playerId << " slot=" << (int)invSlot << " ok=" << ok
              << std::endl;
    return ok;
}

bool Game::unequipSlot(int playerId, uint8_t slotType) {
    auto it = players.find(playerId);
    if (it == players.end()) {
        return false;
    }
    ItemType type;
    switch (slotType) {
        case 0: type = ItemType::WEAPON; break;
        case 1: type = ItemType::ARMOR; break;
        case 2: type = ItemType::HELMET; break;
        case 3: type = ItemType::SHIELD; break;
        default: return false;
    }
    bool ok = it->second.unequipItem(type);
    std::cout << "UNEQUIP player=" << playerId << " slotType=" << (int)slotType << " ok=" << ok
              << std::endl;
    return ok;
}

Game::InventorySnapshot Game::getInventorySnapshot(int playerId) const {
    InventorySnapshot snap;
    auto it = players.find(playerId);
    if (it == players.end()) {
        return snap;
    }
    auto inv = it->second.getInventory();
    snap.items.reserve(inv.size());
    for (const auto& item: inv) {
        snap.items.push_back(item.getId());
    }
    PlayerData d = it->second.getData();
    snap.equippedWeapon = d.equippedWeapon;
    snap.equippedArmor = d.equippedArmor;
    snap.equippedHelmet = d.equippedHelmet;
    snap.equippedShield = d.equippedShield;
    return snap;
}

bool Game::applyNPCAttack(uint8_t playerId, uint16_t damage) {
    auto it = players.find(playerId);
    if (it == players.end()) {
        return false;
    }
    it->second.receiveDamage(damage);
    return true;
}

bool Game::checkIfPlayerIsMeditating(int playerId) const {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        // It's ok if the player is not found. The player has disconnected
        return false;
    }

    return itPlayer->second.getMeditationState() ? 1 : 0;
}

void Game::restorePlayerManaForMeditation(int playerId) {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }

    itPlayer->second.restoreManaForMeditation();
}

Game::~Game() {
    for (const auto& [id, _]: players) {
        updatePlayerData(id);
    }
}
