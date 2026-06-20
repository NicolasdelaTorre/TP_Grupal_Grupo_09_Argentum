#include "game.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include <ctime>

#include "NPC/creature.h"
#include "catalog/attribute_catalog.h"
#include "catalog/creature_catalog.h"
#include "catalog/formula_catalog.h"
#include "catalog/merchant_catalog.h"
#include "stats_definition.h"

Game::Game(Map& world):
        map(world), playerSpawn(map.getPlayerSpawn(0)), parser(BinaryParser()),
        bank(0, "global", 0, 0) {
    // Seedeamos el rand() del server una sola vez aca asi todas las corridas arrancan con una secuencia random distinta.
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    const auto& cc = FormulaCatalog::instance().getClan();
    clans.configure(cc.maxMembers, cc.foundLevel);

    // Fuerzo la carga ahora asi no la dispara el primer jugador.
    MerchantCatalog::instance();
}

bool Game::playerExistsInRecords(const std::string& name) {
    return parser.checkPlayerExists(name);
}

bool Game::addNewPlayer(int playerId, const std::string& name, RaceCode race, ClassCode class_) {
    Position spawn = findSpawnPosition();
    players.emplace(playerId, Player(name, spawn, race, class_));
    parser.savePlayerData(name, players.at(playerId).getData());
    bank.addPlayer(name);
    map.placeEntity(playerId, spawn.x, spawn.y, true, 0);
    std::cout << "Hi " << name << " (" << Race::toString(race) << "/"
              << PlayerClass::toString(class_) << ") spawned at (" << spawn.x << ", " << spawn.y
              << ")" << std::endl;
    return true;
}

bool Game::loadExistingPlayer(int playerId, const std::string& name) {
    players.emplace(playerId, Player(parser.loadPlayerData(name), name));
    Position spawn = players.at(playerId).getPosition();
    bank.addPlayer(name);
    map.placeEntity(playerId, spawn.x, spawn.y, true, players.at(playerId).getMapId());
    std::cout << "Welcome back " << name << " at (" << spawn.x << ", " << spawn.y << ")"
              << std::endl;
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
        return {-1, -1};
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
    return StatsDefinition().nextLevelExp(it->second.getData().level);
}

uint8_t Game::getPlayerMapId(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    return it->second.getData().mapId;
}

bool Game::hasPlayer(int playerId) const { return players.find(playerId) != players.end(); }

bool Game::isPlayerGhost(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) return false;
    return it->second.getData().isGhost;
}

int Game::getPlayerIdAt(int16_t x, int16_t y, uint8_t mapId) const {
    for (const auto& [id, player]: players) {
        if (player.getMapId() == mapId && player.getX() == x && player.getY() == y) {
            return id;
        }
    }
    return -1;
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
        map.removeEntity(p.x, p.y, it->second.getMapId(), true);
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
    Position old = player.getPosition();
    if (!map.isWalkable(next.x, next.y, mapId)) {
        std::cout << "MOVE rejected player=" << playerId << " from (" << old.x << "," << old.y
                  << ") to (" << next.x << "," << next.y << ") map=" << static_cast<int>(mapId)
                  << " reason=not_walkable" << std::endl;
        return false;
    }
    // moveEntity valida internamente que la celda destino no este ocupada por
    // NPC ni por player y devuelve false si lo esta.
    if (!map.moveEntity(playerId, old.x, old.y, next.x, next.y, true, mapId)) {
        std::cout << "MOVE rejected player=" << playerId << " from (" << old.x << "," << old.y
                  << ") to (" << next.x << "," << next.y << ") map=" << static_cast<int>(mapId)
                  << " reason=occupied" << std::endl;
        return false;
    }

    player.move(next);
    player.setDirection(static_cast<uint8_t>(direction));

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
        map.removeEntity(pos.x, pos.y, currentMapId, true);

        // Si veníamos del overworld vamos a la dungeon; si veníamos de una
        // dungeon, salimos al overworld.
        if (currentMapId == 0) {
            std::string mapId = map.getMapId(pos.x, pos.y);
            if (mapId.empty()) {
                return;
            }
            map.placePlayerIntoTheDungeon(playerId, pos, mapId);
            itPlayer->second.changeMapId(static_cast<uint8_t>(mapId[mapId.size() - 1] - '0'));
            Position newPosition = map.getEntrySpawnPosition(mapId);
            itPlayer->second.move(newPosition);
        } else {
            map.placePlayerIntoTheOverworld(playerId, currentMapId);
            itPlayer->second.changeMapId(0);
            Position entryPosition = map.getEntryPosition(currentMapId);
            itPlayer->second.move({entryPosition.x, static_cast<int16_t>(entryPosition.y + 1)});
        }
    }
}

Game::AttackOutcome Game::processAttack(int playerId, uint8_t targetType, uint16_t targetId) {
    AttackOutcome outcome;
    outcome.attackerId = playerId;
    outcome.targetType = targetType;

    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }

    if (!itPlayer->second.isEquipped() || !itPlayer->second.isAlive()) {
        return outcome;
    }

    if (targetType != static_cast<uint8_t>(TargetType::PLAYER) &&
        targetType != static_cast<uint8_t>(TargetType::NPC)) {
        throw std::runtime_error(
                "Game Error: malformed attack command (expected player.id or npc.id)");
    }

    outcome.attackerName = itPlayer->second.getName();

    // Sin mana suficiente para el hechizo
    if (!itPlayer->second.hasEnoughManaForAttack()) {
        outcome.blockedReason = "No tenés maná suficiente para lanzar el hechizo";
        return outcome;
    }

    bool targetPlayer = (targetType == static_cast<uint8_t>(TargetType::PLAYER));
    uint8_t mapId = itPlayer->second.getMapId();
    uint8_t entityId;
    if (itPlayer->second.hasLongDistanceWeapon())
        entityId = map.entityInDistance(itPlayer->second.getX(), itPlayer->second.getY(),
                                        targetPlayer, mapId);
    else
        entityId = map.nextEntity(itPlayer->second.getX(), itPlayer->second.getY(), targetPlayer,
                                  mapId);

    if (entityId != targetId) {
        // Antes se rebotaba silencioso: el cliente ya mostró la animación pero
        // el server no avisa nada. Ahora avisamos para que el jugador entienda.
        outcome.blockedReason = "Estás demasiado lejos del objetivo";
        return outcome;
    }

    uint16_t attackerId = static_cast<uint16_t>(playerId);
    uint8_t atkLvl = itPlayer->second.getData().level;
    uint8_t atkLvlBefore = atkLvl;

    const auto& f = FormulaCatalog::instance().getFormulas();
    bool isCritical = (std::rand() % 100) < f.criticalChancePct;

    if (targetType == static_cast<uint8_t>(TargetType::PLAYER)) {
        auto itTarget = players.find(entityId);
        if (itTarget == players.end()) {
            throw std::runtime_error("Game Error: player in sight not found");
        }
        if (!itTarget->second.isAlive()) {
            return outcome;
        }
        outcome.targetName = itTarget->second.getName();
        outcome.targetId = static_cast<int>(targetId);
        // Zona segura: nada de PvP si atacante o target están adentro.
        Position atkPos = itPlayer->second.getPosition();
        Position tgtPos = itTarget->second.getPosition();
        if (map.isSafeZone(atkPos.x, atkPos.y, mapId) ||
            map.isSafeZone(tgtPos.x, tgtPos.y, mapId)) {
            outcome.blockedReason = "No podés atacar dentro de una zona segura";
            return outcome;
        }
        // Fair play: newbies (lvl <= newbieLevel) no participan en PvP y la
        // diferencia de niveles no puede pasar maxLevelDiff.
        uint8_t tgtLvlFP = itTarget->second.getData().level;
        if (atkLvl <= f.fairPlayNewbieLevel || tgtLvlFP <= f.fairPlayNewbieLevel) {
            outcome.blockedReason = "Los newbies no pueden participar en PvP";
            return outcome;
        }
        uint8_t diff = (atkLvl > tgtLvlFP) ? (atkLvl - tgtLvlFP) : (tgtLvlFP - atkLvl);
        if (diff > f.fairPlayMaxLevelDiff) {
            outcome.blockedReason =
                    "Diferencia de niveles demasiado grande para atacar a " + outcome.targetName;
            return outcome;
        }
        // Mismo clan: no se pueden atacar entre si.
        if (areInSameClan(playerId, static_cast<int>(targetId))) {
            outcome.blockedReason = "No podés atacar a un miembro de tu clan";
            return outcome;
        }
        // dealDamage primero: gasta el mana del hechizo aunque después esquive
        uint16_t rawDamage = itPlayer->second.dealDamage();
        if (isCritical) rawDamage *= 2;
        // Solo se intenta esquivar si NO es crítico.
        if (!isCritical && tryEvade(playerId, static_cast<int>(targetId))) {
            outcome.valid = true;
            outcome.evaded = true;
            outcome.event = std::make_shared<AttackResultEvent>(attackerId, targetType, targetId,
                                                                0, false);
            return outcome;
        }
        // Bonus de clan
        const auto& cc = FormulaCatalog::instance().getClan();
        int nearAtk = countNearbyClanMates(playerId);
        int nearTgt = countNearbyClanMates(static_cast<int>(targetId));
        if (nearAtk > 0)
            rawDamage = static_cast<uint16_t>(rawDamage * (1.0f + cc.bonusPctPerMember * nearAtk));
        uint16_t def = itTarget->second.rollDefense();
        if (nearTgt > 0)
            def = static_cast<uint16_t>(def * (1.0f + cc.bonusPctPerMember * nearTgt));
        uint16_t damage = (def >= rawDamage) ? 0 : (rawDamage - def);
        uint8_t tgtLvl = itTarget->second.getData().level;
        uint16_t tgtMaxHp = itTarget->second.getMaxHealth();
        itTarget->second.receiveDamage(damage);
        int factor = std::max(0, static_cast<int>(tgtLvl) - static_cast<int>(atkLvl) + f.expLevelDiffBase);
        itPlayer->second.grantExp(static_cast<uint32_t>(damage) * factor);
        outcome.killed = !itTarget->second.isAlive();
        if (outcome.killed) {
            uint32_t kill = (std::rand() % (f.expKillBonusMaxPct + 1)) * tgtMaxHp / 100;
            itPlayer->second.grantExp(kill * factor);
        }
        outcome.valid = true;
        outcome.critical = isCritical;
        outcome.damage = damage;
        outcome.event = std::make_shared<AttackResultEvent>(attackerId, targetType, targetId,
                                                            damage, true);
    } else {
        uint16_t damage = itPlayer->second.dealDamage();
        if (isCritical) damage *= 2;
        // Bonus de clan al atacar NPCs
        {
            const auto& cc = FormulaCatalog::instance().getClan();
            int near = countNearbyClanMates(playerId);
            if (near > 0)
                damage = static_cast<uint16_t>(damage * (1.0f + cc.bonusPctPerMember * near));
        }
        Creature* npc = map.getNPC(targetId);
        uint8_t tgtLvl = npc->getLevel();
        uint16_t tgtMaxHp = npc->getMaxHealth();
        outcome.targetName = npc->getName();
        uint16_t damageReceived = npc->receiveDamage(damage);
        int factor = std::max(0, static_cast<int>(tgtLvl) - static_cast<int>(atkLvl) + f.expLevelDiffBase);
        itPlayer->second.grantExp(static_cast<uint32_t>(damageReceived) * factor);
        outcome.killed = npc->isDead();
        if (outcome.killed) {
            // Sacar la creature del mapa para que deje de bloquear celda.
            Position npcPos = npc->getPosition();
            map.removeEntity(npcPos.x, npcPos.y, itPlayer->second.getMapId(), false);

            uint32_t kill = (std::rand() % (f.expKillBonusMaxPct + 1)) * tgtMaxHp / 100;
            itPlayer->second.grantExp(kill * factor);
        }
        outcome.valid = true;
        outcome.critical = isCritical;
        outcome.damage = damageReceived;
        outcome.event = std::make_shared<AttackResultEvent>(attackerId, targetType, targetId,
                                                            damageReceived, true);
    }

    uint8_t atkLvlAfter = itPlayer->second.getData().level;
    if (atkLvlAfter > atkLvlBefore) {
        outcome.leveledUp = true;
        outcome.newLevel = atkLvlAfter;
    }
    return outcome;
}

bool Game::hasHealWeaponEquipped(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) return false;
    return it->second.hasHealWeapon();
}

Game::HealOutcome Game::processHealCast(int casterId, uint16_t targetId) {
    HealOutcome outcome;
    outcome.casterId = casterId;

    auto itCaster = players.find(casterId);
    if (itCaster == players.end()) return outcome;
    if (!itCaster->second.isAlive()) return outcome;
    if (!itCaster->second.hasHealWeapon()) return outcome;
    outcome.casterName = itCaster->second.getName();

    // Auto-curarse: el cliente no manda clicks sobre uno mismo, pero por las dudas.
    if (static_cast<int>(targetId) == casterId) {
        uint16_t healed = itCaster->second.castHealOn(itCaster->second);
        if (healed == 0) {
            outcome.blockedReason = "No tenés maná suficiente para curar";
            return outcome;
        }
        outcome.valid = true;
        outcome.healAmount = healed;
        outcome.targetName = outcome.casterName;
        outcome.targetId = casterId;
        return outcome;
    }

    // Chequeo de rango: la Flauta es a distancia, asi que uso entityInDistance.
    uint8_t mapId = itCaster->second.getMapId();
    uint16_t entityId = map.entityInDistance(itCaster->second.getX(), itCaster->second.getY(),
                                             true, mapId);
    if (entityId != targetId) {
        outcome.blockedReason = "Estás demasiado lejos del objetivo";
        return outcome;
    }

    auto itTarget = players.find(static_cast<int>(targetId));
    if (itTarget == players.end()) return outcome;
    if (!itTarget->second.isAlive()) {
        outcome.blockedReason = "No podés curar a un jugador muerto";
        return outcome;
    }
    outcome.targetName = itTarget->second.getName();
    outcome.targetId = static_cast<int>(targetId);

    uint16_t healed = itCaster->second.castHealOn(itTarget->second);
    if (healed == 0) {
        outcome.blockedReason = "No tenés maná suficiente para curar";
        return outcome;
    }
    outcome.valid = true;
    outcome.healAmount = healed;
    return outcome;
}

bool Game::tryEvade(int /*attackerId*/, int targetId) const {
    // Formula del enunciado: rand(0,1)^Agilidad < evadeThreshold.
    auto it = players.find(targetId);
    if (it == players.end()) return false;
    const auto& r = AttributeCatalog::instance().getRace(it->second.getData().race);
    if (r.agility == 0) return false;
    float thr = FormulaCatalog::instance().getFormulas().evadeThreshold;
    float roll = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    return std::pow(roll, static_cast<float>(r.agility)) < thr;
}

void Game::setSkin(int playerId, uint8_t bodySkinId, uint8_t headSkinId) {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    itPlayer->second.setSkin(bodySkinId, headSkinId);
}

uint8_t Game::getPlayerHead(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    return it->second.getData().headSkinId;
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

// Mapeo de itemId -> nombre canonico (en minuscula, igual que en items.toml).
static const char* itemNameById(uint8_t id) {
    switch (id) {
        case 1: return "sword";
        case 2: return "axe";
        case 3: return "hammer";
        case 4: return "ash staff";
        case 5: return "elven flute";
        case 6: return "root staff";
        case 7: return "socketed staff";
        case 8: return "simple bow";
        case 9: return "composite bow";
        case 10: return "leather armor";
        case 11: return "plate armor";
        case 12: return "blue tunic";
        case 13: return "hood";
        case 14: return "iron helmet";
        case 15: return "turtle shield";
        case 16: return "iron shield";
        case 17: return "wizard hat";
        case 18: return "health potion";
        case 19: return "mana potion";
        default: return nullptr;
    }
}

// Inverso de itemNameById. Acepta el input en cualquier capitalizacion
// ("sword", "Sword", "SWORD"); como los canonicos ya son minuscula, solo
// hace falta lowercasear el input. Devuelve 0 si no existe.
static uint8_t itemIdByName(std::string name) {
    for (char& c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    for (uint8_t id = 1; id <= 19; id++) {
        const char* n = itemNameById(id);
        if (n && name == n) return id;
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
    std::string type;
    if (npcType == static_cast<uint8_t>(NpcCode::MERCHANT)) type = "trader";
    else if (npcType == static_cast<uint8_t>(NpcCode::PRIEST)) type = "priest";
    else return {"Este NPC no vende nada"};

    const auto* items = MerchantCatalog::instance().getItems(type);
    if (!items || items->empty()) {
        return {"Sin items en venta"};
    }
    std::vector<std::string> lines;
    for (const auto& [id, price]: *items) {
        const char* name = itemNameById(id);
        if (!name) continue;
        lines.push_back(std::string("- ") + name + " ($" + std::to_string(price) + ")");
    }
    return lines;
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

// Devuelve el catalogo correspondiente al tipo de NPC, o nullptr si no vende.
static const std::vector<MerchantCatalog::ItemPrice>* catalogFor(uint8_t npcType) {
    if (npcType == static_cast<uint8_t>(NpcCode::MERCHANT))
        return MerchantCatalog::instance().getItems("trader");
    if (npcType == static_cast<uint8_t>(NpcCode::PRIEST))
        return MerchantCatalog::instance().getItems("priest");
    return nullptr;
}

Game::InteractionResult Game::buyFromNpc(int playerId, uint8_t npcType,
                                         const std::string& itemName) {
    auto it = players.find(playerId);
    if (it == players.end()) return {false, "Jugador no existe"};
    uint8_t itemId = itemIdByName(itemName);
    if (itemId == 0) return {false, "Item desconocido: " + itemName};
    std::string canonical = itemNameById(itemId);

    const auto* catalog = catalogFor(npcType);
    if (!catalog) return {false, "Este NPC no vende nada"};

    uint32_t price = 0;
    bool found = false;
    for (const auto& [id, p] : *catalog) {
        if (id == itemId) { price = p; found = true; break; }
    }
    if (!found) return {false, "No vende " + canonical};
    if (it->second.getData().gold < price) {
        return {false, "Te faltan " + std::to_string(price - it->second.getData().gold) + " de oro"};
    }
    if (!it->second.addItem(canonical)) {
        return {false, "Tu inventario esta lleno"};
    }
    it->second.removeGold(price);
    return {true, "Compraste " + canonical + " por " + std::to_string(price)};
}

Game::InteractionResult Game::sellToNpc(int playerId, uint8_t npcType,
                                        const std::string& itemName) {
    auto it = players.find(playerId);
    if (it == players.end()) return {false, "Jugador no existe"};
    // Solo el comerciante compra. El sacerdote no.
    if (npcType != static_cast<uint8_t>(NpcCode::MERCHANT)) {
        return {false, "Solo el comerciante compra items"};
    }
    uint8_t itemId = itemIdByName(itemName);
    if (itemId == 0) return {false, "Item desconocido: " + itemName};
    std::string canonical = itemNameById(itemId);

    // El precio de venta es la mitad del precio de compra del trader.
    const auto* catalog = catalogFor(npcType);
    if (!catalog) return {false, "Este NPC no compra"};
    uint32_t buyPrice = 0;
    for (const auto& [id, p] : *catalog) {
        if (id == itemId) { buyPrice = p; break; }
    }
    if (buyPrice == 0) return {false, "No compra " + canonical};
    uint32_t sellPrice = buyPrice / 2;

    if (it->second.isItemEquipped(itemId)) {
        return {false, "Tenes " + canonical + " equipado, desequipalo primero"};
    }
    if (it->second.removeItemByName(canonical) == 0) {
        return {false, "No tenes " + canonical + " en el inventario"};
    }
    it->second.addGold(sellPrice);
    return {true, "Vendiste " + canonical + " por " + std::to_string(sellPrice)};
}

Game::InteractionResult Game::depositItemToBank(int playerId, const std::string& itemName) {
    auto it = players.find(playerId);
    if (it == players.end()) return {false, "Jugador no existe"};

    uint8_t itemId = itemIdByName(itemName);
    if (itemId == 0) return {false, "Item desconocido: " + itemName};

    std::string canonical = itemNameById(itemId);
    if (it->second.isItemEquipped(itemId)) {
        return {false, "Tenes " + canonical + " equipado, desequipalo primero"};
    }
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

    it->second.revive();
    
    // it->second.resetStats();

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

            // Drop de oro: se suma al gold del jugador (respetando cap).
            if (rec.itemId == GOLD_ITEM_ID) {
                it->second.addGold(rec.goldAmount);
                return {true, "Levantaste " + std::to_string(rec.goldAmount) + " de oro", rec};
            }

            // Item normal: addItem por nombre canonico.
            const char* name = itemNameById(rec.itemId);
            if (!name) return {false, "Item desconocido", {}};
            if (!it->second.addItem(name)) {
                // Inventario lleno: lo devolvemos al piso para no perderlo.
                droppedItems.insert(droppedItems.begin() + i, rec);
                return {false, "Tu inventario esta lleno", {}};
            }
            return {true, std::string("Levantaste ") + name, rec};
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
    if (it->second.isItemEquipped(itemId)) {
        return {false, "Tenes " + itemName + " equipado, desequipalo primero", {}};
    }
    // Sacamos el item del inventario para que no quede duplicado.
    if (it->second.removeItemByName(itemName) == 0) {
        return {false, "No se pudo tirar (no estaba en inventario)", {}};
    }
    DroppedItemRecord rec{nextDropId++, itemId, pos.x, pos.y, 0};
    droppedItems.push_back(rec);
    return {true, "Tiraste " + itemName, rec};
}

std::vector<Game::DroppedItemRecord> Game::dropPlayerLootOnDeath(int playerId) {
    std::vector<DroppedItemRecord> drops;
    auto it = players.find(playerId);
    if (it == players.end()) return drops;
    Player& p = it->second;
    Position pos = p.getPosition();

    // Iteramos el inventario y tiramos cada item. Usamos getInventory() una
    // vez y vamos sacando uno a uno con removeItemByName: cada llamada baja el
    // tamaño, asi que iteramos sobre la copia inicial.
    auto inv = p.getInventory();
    for (const Item& item: inv) {
        std::string name = item.getName();
        uint8_t id = item.getId();
        if (p.removeItemByName(name) == 0) continue;
        DroppedItemRecord rec{nextDropId++, id, pos.x, pos.y, 0};
        droppedItems.push_back(rec);
        drops.push_back(rec);
    }
    // Desequipar todo: equipItem solo copia al slot equipado sin sacar del
    // inventario, asi que removeItemByName arriba no toca los equipped slots.
    // Al revivir el player no puede atacar/defender sin volver a equipar.
    p.unequipItem(ItemType::WEAPON);
    p.unequipItem(ItemType::ARMOR);
    p.unequipItem(ItemType::HELMET);
    p.unequipItem(ItemType::SHIELD);

    // Oro en exceso del cap (= 100 * Nivel^1.1). Se queda con safeGold y tira
    // el resto.
    uint32_t cap = StatsDefinition().safeGold(p.getData().level);
    uint32_t gold = p.getData().gold;
    if (gold > cap) {
        uint32_t excess = gold - cap;
        p.removeGold(excess);
        DroppedItemRecord rec{nextDropId++, GOLD_ITEM_ID, pos.x, pos.y, excess};
        droppedItems.push_back(rec);
        drops.push_back(rec);
        std::cout << "DEATH-DROP gold player=" << playerId << " excess=" << excess << std::endl;
    }
    return drops;
}

std::vector<Game::DroppedItemRecord> Game::dropCreatureLootOnDeath(uint16_t npcId) {
    std::vector<DroppedItemRecord> drops;
    Creature* npc = map.getNPC(npcId);
    if (!npc) return drops;
    Position pos = npc->getPosition();

    // Probabilidades acumuladas: nothing, gold, potion, item. Lo que sobre
    // hasta 100 cae como "nada" (efectivo).
    const auto& L = CreatureCatalog::instance().getLoot();
    int roll = std::rand() % 100;
    int goldCut = L.nothingChance + L.goldChance;
    int potionCut = goldCut + L.potionChance;
    int itemCut = potionCut + L.itemChance;
    if (roll < L.nothingChance) {
        // nada
    } else if (roll < goldCut) {
        // Oro = rand(min, max) / 100 * VidaMaxNPC. Min = 1 para asegurar
        // que el monto nunca sea 0.
        uint16_t vidaMax = npc->getMaxHealth();
        uint8_t span = L.goldFactorMaxPct - L.goldFactorMinPct + 1;
        uint32_t factorPct = L.goldFactorMinPct + std::rand() % span;
        uint32_t amount = (vidaMax * factorPct) / 100;
        if (amount == 0) amount = 1;
        DroppedItemRecord rec{nextDropId++, GOLD_ITEM_ID, pos.x, pos.y, amount};
        droppedItems.push_back(rec);
        drops.push_back(rec);
    } else if (roll < potionCut) {
        uint8_t potionId = (std::rand() & 1) ? L.potionHealthId : L.potionManaId;
        DroppedItemRecord rec{nextDropId++, potionId, pos.x, pos.y, 0};
        droppedItems.push_back(rec);
        drops.push_back(rec);
    } else if (roll < itemCut) {
        uint8_t poolSize = L.itemIdMax - L.itemIdMin + 1;
        uint8_t itemId = L.itemIdMin + (std::rand() % poolSize);
        DroppedItemRecord rec{nextDropId++, itemId, pos.x, pos.y, 0};
        droppedItems.push_back(rec);
        drops.push_back(rec);
    }
    if (!drops.empty()) {
        std::cout << "CREATURE-DROP npc=" << npcId << " drops=" << drops.size() << std::endl;
    }
    return drops;
}

bool Game::equipOrUseItem(int playerId, uint8_t invSlot) {
    auto it = players.find(playerId);
    if (it == players.end()) {
        return false;
    }
    return it->second.equipItem(static_cast<int>(invSlot));
}

bool Game::unequipSlot(int playerId, uint8_t slotType) {
    auto it = players.find(playerId);
    if (it == players.end()) {
        return false;
    }
    ItemType type;
    switch (static_cast<EquipmentSlot>(slotType)) {
        case EquipmentSlot::WEAPON: type = ItemType::WEAPON; break;
        case EquipmentSlot::ARMOR:  type = ItemType::ARMOR; break;
        case EquipmentSlot::HELMET: type = ItemType::HELMET; break;
        case EquipmentSlot::SHIELD: type = ItemType::SHIELD; break;
        default: return false;
    }
    return it->second.unequipItem(type);
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

uint16_t Game::applyNPCAttack(uint8_t playerId, uint16_t rawDamage) {
    auto it = players.find(playerId);
    if (it == players.end()) return 0;
    uint16_t def = it->second.rollDefense();
    uint16_t finalDmg = (def >= rawDamage) ? 0 : (rawDamage - def);
    it->second.receiveDamage(finalDmg);
    return finalDmg;
}

bool Game::checkIfPlayerIsMeditating(int playerId) const {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        // It's ok if the player is not found. The player has disconnected
        return false;
    }

    return itPlayer->second.getMeditationState() ? 1 : 0;
}

bool Game::checkIfPlayerIsTeleporting(int playerId) const {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        // It's ok if the player is not found. The player has disconnected
        return false;
    }

    return itPlayer->second.getTeleportingState() ? 1 : 0;
}

void Game::finishTeleportingState(int playerId) {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        return;
    }

    itPlayer->second.finishTeleporting();
}

void Game::restorePlayerManaForMeditation(int playerId) {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        return;
    }

    itPlayer->second.restoreManaForMeditation();
}

bool Game::startPlayerResurrect(int playerId) {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }

    if (itPlayer->second.isAlive()) {
        return false;
    }

    itPlayer->second.startTeleporting();

    return true;
}

bool Game::lowerHealth(int playerId) {
    auto player = players.find(playerId);
    if (player == players.end()) {
        return false;
    }

    return player->second.isAlive() && player->second.getCurrentHealth() < player->second.getMaxHealth();
}

bool Game::lowerMana(int playerId) {
    auto player = players.find(playerId);
    if (player == players.end()) {
        return false;
    }

    return player->second.isAlive() && player->second.getCurrentMana() < player->second.getMaxMana();
}

void Game::restorePlayerHealth(int playerId) {
    auto player = players.find(playerId);
    if (player == players.end()) {
        return;
    }

    player->second.restoreHealthThroughTime();
}

void Game::restorePlayerMana(int playerId) {
    auto player = players.find(playerId);
    if (player == players.end()) {
        return;
    }

    player->second.restoreManaThroughTime();
}

void Game::fastTravel(int playerId, Position newPosition) {
    auto player = players.find(playerId);
    if (player == players.end()) {
        return;
    }

    player->second.move(newPosition);
}

// ── Clanes ──────────────────────────────────────────────────────────────

ClanOutcome Game::tryFoundClan(int playerId, const std::string& clanName) {
    auto it = players.find(playerId);
    if (it == players.end())
        return {false, "Jugador no encontrado"};
    return clans.tryFound(clanName, it->second.getName(), it->second.getData().level);
}

ClanOutcome Game::tryRequestJoinClan(int playerId, const std::string& clanName) {
    auto it = players.find(playerId);
    if (it == players.end())
        return {false, "Jugador no encontrado"};
    return clans.tryRequestJoin(clanName, it->second.getName());
}

ClanOutcome Game::tryAcceptClanRequest(int playerId, const std::string& applicantNick) {
    auto it = players.find(playerId);
    if (it == players.end())
        return {false, "Jugador no encontrado"};
    return clans.tryAccept(it->second.getName(), applicantNick);
}

ClanOutcome Game::tryRejectClanRequest(int playerId, const std::string& applicantNick) {
    auto it = players.find(playerId);
    if (it == players.end())
        return {false, "Jugador no encontrado"};
    return clans.tryReject(it->second.getName(), applicantNick);
}

ClanOutcome Game::tryBanFromClan(int playerId, const std::string& targetNick) {
    auto it = players.find(playerId);
    if (it == players.end())
        return {false, "Jugador no encontrado"};
    return clans.tryBan(it->second.getName(), targetNick);
}

ClanOutcome Game::tryKickFromClan(int playerId, const std::string& targetNick) {
    auto it = players.find(playerId);
    if (it == players.end())
        return {false, "Jugador no encontrado"};
    return clans.tryKick(it->second.getName(), targetNick);
}

ClanOutcome Game::tryLeaveClan(int playerId) {
    auto it = players.find(playerId);
    if (it == players.end())
        return {false, "Jugador no encontrado"};
    return clans.tryLeave(it->second.getName());
}

std::vector<std::string> Game::reviewClan(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end())
        return {};
    const std::string& nick = it->second.getName();
    auto clanName = clans.getClanOf(nick);
    if (!clanName)
        return {"No estás en ningún clan"};
    if (!clans.isFounder(nick))
        return {"Solo el fundador puede revisar el clan"};
    std::vector<std::string> lines;
    lines.push_back("Clan " + *clanName + ":");
    auto members = clans.getMembersOf(*clanName);
    lines.push_back("Miembros (" + std::to_string(members.size()) + "):");
    for (const auto& m: members)
        lines.push_back("  - " + m);
    auto pending = clans.getPendingOf(*clanName);
    lines.push_back("Pedidos pendientes (" + std::to_string(pending.size()) + "):");
    for (const auto& p: pending)
        lines.push_back("  - " + p);
    return lines;
}

std::string Game::getClanOf(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end())
        return "";
    auto c = clans.getClanOf(it->second.getName());
    return c.value_or("");
}

std::vector<std::string> Game::getClanMembers(const std::string& clanName) const {
    return clans.getMembersOf(clanName);
}

bool Game::areInSameClan(int playerA, int playerB) const {
    auto itA = players.find(playerA);
    auto itB = players.find(playerB);
    if (itA == players.end() || itB == players.end())
        return false;
    return clans.areInSameClan(itA->second.getName(), itB->second.getName());
}

int Game::countNearbyClanMates(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end())
        return 0;
    auto myClan = clans.getClanOf(it->second.getName());
    if (!myClan)
        return 0;
    const auto& cc = FormulaCatalog::instance().getClan();
    Position myPos = it->second.getPosition();
    uint8_t myMap = it->second.getMapId();
    auto mates = clans.getMembersOf(*myClan);
    int count = 0;
    for (const auto& mateNick: mates) {
        if (mateNick == it->second.getName())
            continue;
        for (const auto& [otherId, other]: players) {
            if (other.getName() != mateNick)
                continue;
            if (other.getMapId() != myMap)
                break;
            Position op = other.getPosition();
            int dist = std::abs(op.x - myPos.x) + std::abs(op.y - myPos.y);
            if (dist <= cc.bonusRadius)
                count++;
            break;
        }
    }
    return count;
}

void Game::changeMapId(int playerId, uint8_t newMapId) {
    auto it = players.find(playerId);
    if (it == players.end()) return;
    it->second.changeMapId(newMapId);
}

Game::~Game() {
    for (const auto& [id, _]: players) {
        updatePlayerData(id);
    }
}
