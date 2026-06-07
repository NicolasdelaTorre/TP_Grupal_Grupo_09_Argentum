#include "game.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

Game::Game(Map& map, Position playerSpawn):
        map(map), playerSpawn(playerSpawn), parser(BinaryParser()) {}

bool Game::addPlayer(int playerId, const std::string& name, const std::string& race,
                     const std::string& class_) {
    Position spawn;

    if (!parser.checkPlayerExists(name)) {
        spawn = findSpawnPosition();
        players.emplace(playerId, Player(name, spawn, race, class_));
        parser.savePlayerData(name, players.at(playerId).getData());
    } else {
        players.emplace(playerId, Player(parser.loadPlayerData(name), name));
        spawn = players.at(playerId).getPosition();
    }

    map.placeEntity(playerId, spawn.x, spawn.y, true);

    std::cout << "Hi " << name << " (" << race << "/" << class_ << ") spawned at (" << spawn.x
              << ", " << spawn.y << ")" << std::endl;

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
    // Búsqueda en espiral cuadrada desde el spawn del YAML.
    // radius=0 es el spawn, radius=1 sus 8 vecinos, radius=2 los 16 de la siguiente capa, etc.
    int maxRadius = std::max(map.getWidth(), map.getHeight());
    for (int radius = 0; radius < maxRadius; radius++) {
        for (int offsetY = -radius; offsetY <= radius; offsetY++) {
            for (int offsetX = -radius; offsetX <= radius; offsetX++) {
                // Solo la frontera del cuadrado (las interiores ya las chequeamos en radios
                // anteriores).
                if (radius > 0 && std::abs(offsetX) != radius && std::abs(offsetY) != radius)
                    continue;
                Position candidate{static_cast<int16_t>(playerSpawn.x + offsetX),
                                   static_cast<int16_t>(playerSpawn.y + offsetY)};
                if (map.isWalkable(candidate.x, candidate.y) && isPositionFree(candidate)) {
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
        map.removePlayer(p.x, p.y);
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

    if (!map.isWalkable(next.x, next.y)) {
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
    map.movePlayer(playerId, old.x, old.y, next.x, next.y);
    return true;
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

    uint8_t entityId = 0;
    bool targetPlayer = (targetType == 0);
    if (targetType != 0 && targetType != 1) {
        throw std::runtime_error(
                "Game Error: malformed attack command (expected player.id or npc.id)");
    }
    if (itPlayer->second.hasLongDistanceWeapon())
        entityId =
                map.entityInDistance(itPlayer->second.getX(), itPlayer->second.getY(), targetPlayer);
    else
        entityId = map.nextEntity(itPlayer->second.getX(), itPlayer->second.getY(), targetPlayer);

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

    // target = npc. TODO(team-gameplay): damageNpc(targetId, damage).
    return std::make_shared<AttackResultEvent>(attackerId, targetType, targetId, 0, false);
}

bool Game::tryEvade(int /*attackerId*/, int /*targetId*/) const {
    // TODO(team-gameplay): implementar fórmula real.
    // Idea: comparar dexterity del defensor vs del atacante.
    //   chance_evade = clamp((def_dex - atk_dex) * factor, min%, max%)
    // Por ahora nadie evade.
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

Game::~Game() {
    for (const auto& [id, _]: players) {
        updatePlayerData(id);
    }
}
