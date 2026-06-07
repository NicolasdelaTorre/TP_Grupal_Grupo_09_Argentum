#include "game.h"
#include "NPC/creature.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

Game::Game(Map& world):
    map(world),
    playerSpawn(map.getPlayerSpawn(0)),
    parser(BinaryParser()) {}

bool Game::processCommand(int playerId, const std::string& command) {
    size_t commandPosition = command.find('.');
    if (commandPosition == std::string::npos) {
        throw std::runtime_error("Game Error: command from client malformed");
    }

    std::string dataType = command.substr(0, commandPosition);

    if (dataType == "user") {
        std::string user = command.substr(commandPosition + 1);
        return processUser(playerId, user);
    } else if (dataType == "movement") {
        std::string direction = command.substr(commandPosition + 1);
        return processMovement(playerId, direction);
    } else if (dataType == "turn") {
        std::string direction = command.substr(commandPosition + 1);
        return turnPlayer(playerId, direction);
    } else if (dataType == "attack") {
        // Format: "attack.player.id // attack.npc.id"
        // return processAttack(playerId, command.substr(commandPosition + 1));
        return false;
    } else if (dataType == "heal") {
        // Format: "heal"
        return processHeal(playerId);
    }

    return false;
}

bool Game::processUser(int playerId, const std::string& user) {
    // Formato: "NAME:RACE:CLASS"
    size_t firstColon = user.find(':');
    size_t secondColon = user.find(':', firstColon + 1);
    if (firstColon == std::string::npos || secondColon == std::string::npos) {
        throw std::runtime_error("Game Error: malformed user command (expected NAME:RACE:CLASS)");
    }

    std::string name = user.substr(0, firstColon);
    std::string race = user.substr(firstColon + 1, secondColon - firstColon - 1);
    std::string class_ = user.substr(secondColon + 1);

    Position spawn;

    if (!parser.checkPlayerExists(name)) {
        spawn = findSpawnPosition();
        players.emplace(playerId, Player(name, spawn, race, class_));
        parser.savePlayerData(name, players.at(playerId).getData());
    } else {
        players.emplace(playerId, Player(parser.loadPlayerData(name), name));
        spawn = players.at(playerId).getPosition();
    }

    map.placeEntity(playerId, spawn.x, spawn.y, true, 0);

    /* Codigo de testeo
    if (players.size() > 1) {
        // players.at(playerId).addItem("Elven Flute");
        // players.at(playerId).addItem("Sword");
        // players.at(playerId).equipItem(0);
        std::cout << "Inventory: ";
        for (const auto& item: players.at(playerId).getInventory()) {
            std::cout << item.getName() << " ";
        }
        std::cout << std::endl;
        // players.at(playerId).equipItem(1);
        std::cout << "Inventory: ";
        for (const auto& item: players.at(playerId).getInventory()) {
            std::cout << item.getName() << " ";
        }
        std::cout << std::endl;
        // processAttack ahora toma (playerId, targetType, targetId) tras unificar
        // con TARGETED_ATTACK. Si Oli necesita este testeo, hay que pasarle un
        // target_id válido. Comentado para que compile.
        // processAttack(playerId, "bottom");
        // processHeal(playerId);
    }
    */

    std::cout << "Hi " << name << " (" << race << "/" << class_ << ") spawned at (" << spawn.x
              << ", " << spawn.y << ")" << std::endl;

    return true;
}

bool Game::turnPlayer(int playerId, const std::string& direction) {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        return false;
    }
    uint8_t newDir;
    if (direction == "top")
        newDir = 3;
    else if (direction == "bottom")
        newDir = 4;
    else if (direction == "left")
        newDir = 5;
    else if (direction == "right")
        newDir = 6;
    else
        return false;

    itPlayer->second.setDirection(newDir);
    return true;
}

Position Game::findSpawnPosition() const {
    // Búsqueda en espiral cuadrada desde el spawn del YAML.
    // radius=0 es el spawn, radius=1 sus 8 vecinos, radius=2 los 16 de la siguiente capa, etc.
    int maxRadius = std::max(map.getWidth(0), map.getHeight(0));
    for (int radius = 0; radius < maxRadius; radius++) {
        for (int offsetY = -radius; offsetY <= radius; offsetY++) {
            for (int offsetX = -radius; offsetX <= radius; offsetX++) {
                // Solo la frontera del cuadrado (las interiores ya las chequeamos en radios
                // anteriores).
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

bool Game::processMovement(int playerId, const std::string& direction) {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }

    Player& player = itPlayer->second;
    Position next = player.getPosition();
    uint8_t newDir = 4;

    if (direction == "top") {
        next.y -= 1;
        newDir = 3;
    } else if (direction == "bottom") {
        next.y += 1;
        newDir = 4;
    } else if (direction == "left") {
        next.x -= 1;
        newDir = 5;
    } else if (direction == "right") {
        next.x += 1;
        newDir = 6;
    } else {
        return false;
    }

    if (!map.isWalkable(next.x, next.y, itPlayer->second.getMapId())) {
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
    player.setDirection(newDir);
    map.moveEntity(playerId, old.x, old.y, next.x, next.y, true, itPlayer->second.getMapId());
    checkEntry(playerId);

    return true;
}

void Game::checkEntry(int playerId) {
    // Search Player
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }

    // Current Player Position
    Position pos = itPlayer->second.getPosition();

    if (map.checkIfThePositionHasAnEntry(pos.x, pos.y, itPlayer->second.getMapId())) {
        // The Position is a Entry to a Dungeon
        // The Player is no more in the Overworld
        map.removePlayer(pos.x, pos.y, itPlayer->second.getMapId());

        // New Map Id
        std::string mapId = map.getMapId(pos.x, pos.y);

        if (!mapId.empty()) {
            // It's a real Dungeon
            // Place the Player in the Dungeon
            map.placePlayerIntoTheDungeon(playerId, mapId);

            // Save Map Id
            itPlayer->second.changeMapId(static_cast<uint8_t>((mapId[mapId.size() - 1])) - '0');

            // Set new Position for the Player
            Position newPosition = map.getEntrySpawnPosition(mapId);
            itPlayer->second.move(newPosition);
        }
    }
}

AttackResult Game::processAttack(int playerId, uint8_t targetType, uint16_t targetId) {
    AttackResult result;
    result.attackerId = static_cast<uint16_t>(playerId);
    result.targetType = targetType;
    result.targetId = targetId;

    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }

    if (!itPlayer->second.isEquipped() || !itPlayer->second.isAlive()) {
        return result;  // performed = false
    }

    uint8_t entityId = 0;
    if (targetType == 0) {
        if (itPlayer->second.hasLongDistanceWeapon())
            entityId = map.entityInDistance(itPlayer->second.getX(), itPlayer->second.getY(), true, itPlayer->second.getMapId());
        else
            entityId = map.nextEntity(itPlayer->second.getX(), itPlayer->second.getY(), true, itPlayer->second.getMapId());
    } else if (targetType == 1) {
        if (itPlayer->second.hasLongDistanceWeapon())
            entityId =
                    map.entityInDistance(itPlayer->second.getX(), itPlayer->second.getY(), false, itPlayer->second.getMapId());
        else
            entityId = map.nextEntity(itPlayer->second.getX(), itPlayer->second.getY(), false, itPlayer->second.getMapId());
    } else {
        throw std::runtime_error(
                "Game Error: malformed attack command (expected player.id or npc.id)");
    }

    if (entityId != targetId)
        return result;

    auto itTarget = players.find(entityId);
    if (itTarget == players.end()) {
        throw std::runtime_error("Game Error: player in sight not found");
    }

    if (targetType == 0) {
        // target = player
        if (!itTarget->second.isAlive()) {
            return result;
        }
        result.performed = true;
        result.attackerId = playerId;
        result.targetType = targetType;
        result.targetId = targetId;
        if (tryEvade(playerId, static_cast<int>(targetId))) {
            return result;  // hit = false, damage = 0
        }

        uint16_t damage = itPlayer->second.dealDamage();
        itTarget->second.receiveDamage(damage);
        result.hit = true;
        result.damage = damage;
    } else if (targetType == 1) {
        uint16_t damage = itPlayer->second.dealDamage();
        Creature* npc = map.getNPC(targetId);
        npc->receiveDamage(damage);

        result.performed = true;
        result.attackerId = playerId;
        result.targetType = targetType;
        result.targetId = targetId;
        result.damage = damage;
        result.hit = true;
    }

    return result;
}

bool Game::tryEvade(int /*attackerId*/, int /*targetId*/) const {
    // TODO(team-gameplay): implementar fórmula real.
    // Idea: comparar dexterity del defensor vs del atacante.
    //   chance_evade = clamp((def_dex - atk_dex) * factor, min%, max%)
    // Por ahora nadie evade.
    return false;
}

bool Game::processHeal(int playerId) {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }

    if (!itPlayer->second.isAlive()) {
        return false;
    }

    uint16_t healedAmount = itPlayer->second.heal();

    if (healedAmount == 0) {
        return false;
    }

    std::cout << "Player " << itPlayer->second.getName() << " heals for " << healedAmount
              << " health!" << std::endl;

    return true;
}

void Game::setSkin(int playerId, const std::string& skinId) {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }

    // Cambiar el cero proximamente
    itPlayer->second.setSkin(std::stoi(skinId), 0);
}

void Game::processCheat(int playerId, uint8_t code) {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    // TODO(team-gameplay): implementar la lógica de cada cheat.
    //   0 = SUICIDE     → player.receiveDamage(player.getHealth())
    //   1 = GOLD        → sumar 1000 al gold persistido
    //   2 = EXPERIENCE  → sumar 1000 a la experiencia, levelup si corresponde
    // Hoy solo loggeamos para confirmar que el mensaje llegó end-to-end.
    std::cout << "Cheat recibido: player=" << playerId << " code=" << static_cast<int>(code)
              << " (stub, sin efecto)" << std::endl;
}

bool Game::pickUpItemAt(int /*playerId*/) {
    // TODO(team-gameplay): buscar item en droppedItems en la celda del
    // jugador, llamarlo a player.addItem y removerlo del piso. Stub vacío.
    return false;
}

bool Game::dropItem(int playerId, uint8_t invSlot) {
    auto it = players.find(playerId);
    if (it == players.end()) {
        return false;
    }
    auto inv = it->second.getInventory();
    if (invSlot >= inv.size()) {
        return false;
    }
    // TODO(team-gameplay): agregar el item a droppedItems en la celda
    // actual del jugador. Hoy lo único que hacemos es loggear (el item
    // se "pierde" desde el punto de vista del piso). Para que el flujo
    // protocolo se vea, igual reflejamos el cambio: removemos del inv
    // manualmente recreando el inventario. (Player::removeItem no existe).
    // Esto es feo y temporal — se rehace cuando Oli implemente el piso.
    std::cout << "DROP player=" << playerId << " slot=" << (int)invSlot
              << " name=" << inv[invSlot].getName() << " (stub: item se pierde)" << std::endl;
    return true;
}

bool Game::equipOrUseItem(int playerId, uint8_t invSlot) {
    auto it = players.find(playerId);
    if (it == players.end()) {
        return false;
    }
    // Player::equipItem ya bifurca por tipo internamente.
    // Para pociones (HEALTH_POTION / MANA_POTION) eso no alcanza — falta
    // que Oli implemente "usar = consumir" para ese tipo. Por ahora
    // forwardeamos directo (las armas/armor/casco/escudo funcionan ya).
    // TODO(team-gameplay): manejar HEALTH_POTION/MANA_POTION en equipItem
    // o agregar un branch acá que llame a player.heal()/consumeMana().
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
        case 0:
            type = ItemType::WEAPON;
            break;
        case 1:
            type = ItemType::ARMOR;
            break;
        case 2:
            type = ItemType::HELMET;
            break;
        case 3:
            type = ItemType::SHIELD;
            break;
        default:
            return false;
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

Game::~Game() {
    for (const auto& [id, _]: players) {
        updatePlayerData(id);
    }
}
