#include "player.h"

#include <algorithm>
#include <cstdlib>
#include <utility>

Player::Player(const std::string& name, Position position, RaceCode race, ClassCode class_):
        name(name), isMeditating(false) {
    inventory.reserve(N);
    data.level = 1;
    data.experience = 0;
    data.gold = StatsDefinition().safeGold(data.level);

    data.position = position;
    data.health = StatsDefinition().maxHealth(data.level, race, class_);
    maxHealth = data.health;
    data.mana = StatsDefinition().maxMana(data.level, race, class_);
    maxMana = data.mana;

    data.race = race;
    data.class_ = class_;
    data.mapId = 0; // Overworld
    data.equippedWeapon = 0;
    data.equippedArmor = 0;
    data.equippedHelmet = 0;
    data.equippedShield = 0;
    data.headSkinId = 0;
    data.bodySkinId = 0;
    data.isGhost = false;

    for (int i = 0; i < N; ++i) {
        data.inventory[i] = 0;
    }
}

Player::Player(PlayerData data, const std::string& name): data(std::move(data)), name(name), isMeditating(false) {
    inventory.reserve(N);
    maxHealth = StatsDefinition().maxHealth(this->data.level, this->data.race, this->data.class_);
    maxMana = StatsDefinition().maxMana(this->data.level, this->data.race, this->data.class_);

    for (size_t slot = 0; slot < N; ++slot) {
        uint8_t itemId = this->data.inventory[slot];
        if (itemId == 0) {
            continue;
        }

        try {
            Item item;
            item.createItemById(itemId);
            inventory.push_back(item);
        } catch (const std::exception&) {
            this->data.inventory[slot] = 0;
        }
    }

    if (this->data.equippedWeapon != 0) {
        try {
            equippedWeapon.createItemById(this->data.equippedWeapon);
        } catch (const std::exception&) {
            this->data.equippedWeapon = 0;
        }
    }

    if (this->data.equippedArmor != 0) {
        try {
            equippedArmor.createItemById(this->data.equippedArmor);
        } catch (const std::exception&) {
            this->data.equippedArmor = 0;
        }
    }

    if (this->data.equippedHelmet != 0) {
        try {
            equippedHelmet.createItemById(this->data.equippedHelmet);
        } catch (const std::exception&) {
            this->data.equippedHelmet = 0;
        }
    }

    if (this->data.equippedShield != 0) {
        try {
            equippedShield.createItemById(this->data.equippedShield);
        } catch (const std::exception&) {
            this->data.equippedShield = 0;
        }
    }
}

void Player::move(Position newPosition) {
    if (teleporting) return;

    isMeditating = false;
    data.position = newPosition; 
}

void Player::setDirection(uint8_t dir) {
    if (teleporting) return;

    direction = dir; 
}

const std::string& Player::getName() const { return name; }

Position Player::getPosition() const { return data.position; }

int16_t Player::getX() const { return data.position.x; }

int16_t Player::getY() const { return data.position.y; }

uint8_t Player::getDirection() const { return direction; }

PlayerData Player::getData() const { return data; }

std::vector<Item> Player::getInventory() const { return inventory; }

uint16_t Player::getMaxHealth() const { return maxHealth; }

uint16_t Player::getMaxMana() const { return maxMana; }

uint8_t Player::getMapId() const { return data.mapId; }

bool Player::getMeditationState() const { return isMeditating; }

bool Player::getTeleportingState() const { return teleporting; }

uint16_t Player::getCurrentHealth() const { return data.health; }

uint16_t Player::getCurrentMana() const { return data.mana; }

bool Player::hasLongDistanceWeapon() { return equippedWeapon.longDistance(); }

void Player::receiveDamage(uint16_t damage) {
    if (teleporting) return;
    if (data.isGhost) return;
    if (infiniteHealth) return;
    isMeditating = false;
    if (damage >= data.health) {
        data.health = 0;
        data.isGhost = true;
    } else {
        data.health -= damage;
    }
}

bool Player::isEquipped() { return !equippedWeapon.emptyItem(); }

bool Player::isItemEquipped(uint8_t itemId) const {
    if (itemId == 0) return false;
    return data.equippedWeapon == itemId || data.equippedArmor == itemId ||
           data.equippedHelmet == itemId || data.equippedShield == itemId;
}

bool Player::isAlive() { return !data.isGhost; }

bool Player::hasEnoughManaForAttack() {
    if (equippedWeapon.emptyItem() || equippedWeapon.getType() != ItemType::MAGIC)
        return true;
    if (infiniteMana)
        return true;
    return equippedWeapon.getManaWaste() <= data.mana;
}

bool Player::hasHealWeapon() const {
    if (equippedWeapon.emptyItem()) return false;
    return equippedWeapon.getType() == ItemType::HEAL;
}

uint16_t Player::castHealOn(Player& target) {
    if (teleporting) return 0;
    if (equippedWeapon.emptyItem() || equippedWeapon.getType() != ItemType::HEAL) return 0;
    if (target.data.isGhost) return 0;

    uint16_t manaCost = equippedWeapon.getManaWaste();
    if (manaCost > data.mana && !infiniteMana) return 0;
    if (!infiniteMana) data.mana -= manaCost;
    isMeditating = false;

    uint16_t healAmount = equippedWeapon.getHealthRestore();
    if (target.data.health + healAmount > target.maxHealth) {
        healAmount = target.maxHealth - target.data.health;
    }
    target.data.health += healAmount;
    return healAmount;
}

uint16_t Player::dealDamage() {
    if (teleporting || equippedWeapon.emptyItem() || !equippedWeapon.isOffensiveWeapon()) {
        return 0;
    }

    isMeditating = false;

    if (equippedWeapon.getType() == ItemType::MAGIC) {
        uint16_t manaCost = equippedWeapon.getManaWaste();
        if (manaCost > data.mana && !infiniteMana) return 0;
        data.mana -= manaCost;
    }

    uint16_t damage = StatsDefinition().damage(data.race, equippedWeapon.getMinDamage(),
                                    equippedWeapon.getMaxDamage());

    return damage;
}

// Tira un rand uniforme entre min y max inclusive.
static uint16_t randRange(uint16_t min, uint16_t max) {
    if (max <= min) return min;
    return min + std::rand() % (max - min + 1);
}

uint16_t Player::rollDefense() {
    uint16_t total = 0;
    if (!equippedArmor.emptyItem()) {
        total += randRange(equippedArmor.getMinDefense(), equippedArmor.getMaxDefense());
    }
    if (!equippedHelmet.emptyItem()) {
        total += randRange(equippedHelmet.getMinDefense(), equippedHelmet.getMaxDefense());
    }
    if (!equippedShield.emptyItem()) {
        total += randRange(equippedShield.getMinDefense(), equippedShield.getMaxDefense());
    }
    return total;
}

bool Player::addItem(const std::string& itemName) {
    if (teleporting) return false;

    Item newItem;
    newItem.createItem(itemName);
    if (inventory.size() < N) {
        inventory.push_back(newItem);
        data.inventory[inventory.size() - 1] = newItem.getId();
        return true;
    }

    return false;
}

uint8_t Player::removeItemByName(const std::string& itemName) {
    for (size_t i = 0; i < inventory.size(); i++) {
        if (inventory[i].getName() == itemName) {
            uint8_t id = inventory[i].getId();
            inventory.erase(inventory.begin() + i);
            // Recompactar data.inventory para que matchee el vector.
            for (size_t j = 0; j < N; j++) {
                data.inventory[j] = j < inventory.size() ? inventory[j].getId() : 0;
            }
            return id;
        }
    }
    return 0;
}

bool Player::equipItem(int inventorySlot) {
    if (teleporting) return false;

    if (inventorySlot < 0 || (size_t)inventorySlot >= inventory.size()) {
        return false;
    }

    Item itemToEquip = inventory[inventorySlot];
    if (itemToEquip.emptyItem()) {
        return false;
    }

    ItemType type = itemToEquip.getType();
    unequipItem(type);

    switch (type) {
        case ItemType::WEAPON:
        case ItemType::HEAL:
        case ItemType::MAGIC:
            equippedWeapon = itemToEquip;
            data.equippedWeapon = equippedWeapon.getId();
            break;
        case ItemType::ARMOR:
            equippedArmor = itemToEquip;
            data.equippedArmor = equippedArmor.getId();
            break;
        case ItemType::HELMET:
            equippedHelmet = itemToEquip;
            data.equippedHelmet = equippedHelmet.getId();
            break;
        case ItemType::SHIELD:
            equippedShield = itemToEquip;
            data.equippedShield = equippedShield.getId();
            break;
        case ItemType::HEALTH_POTION:
            data.health = std::min<uint16_t>(data.health + itemToEquip.getHealthRestore(), maxHealth);
            data.inventory[inventorySlot] = 0;
            inventory.erase(inventory.begin() + inventorySlot);
            break;
        case ItemType::MANA_POTION:
            data.mana = std::min<uint16_t>(data.mana + itemToEquip.getManaRestore(), maxMana);
            data.inventory[inventorySlot] = 0;
            inventory.erase(inventory.begin() + inventorySlot);
            break;
        default:
            throw std::runtime_error("Player Error: trying to equip an item that is not exist");
    }

    return true;
}

bool Player::unequipItem(ItemType type) {
    if (teleporting) return false;

    switch (type) {
        case ItemType::WEAPON:
            if (equippedWeapon.emptyItem())
                return false;
            data.equippedWeapon = 0;
            equippedWeapon = Item();
            break;
        case ItemType::ARMOR:
            if (equippedArmor.emptyItem())
                return false;
            data.equippedArmor = 0;
            equippedArmor = Item();
            break;
        case ItemType::HELMET:
            if (equippedHelmet.emptyItem())
                return false;
            data.equippedHelmet = 0;
            equippedHelmet = Item();
            break;
        case ItemType::SHIELD:
            if (equippedShield.emptyItem())
                return false;
            data.equippedShield = 0;
            equippedShield = Item();
            break;
        default:
            return false;
    }
    return true;
}

uint16_t Player::heal() {
    if (teleporting || equippedWeapon.emptyItem() || equippedWeapon.getType() != ItemType::HEAL) {
        return 0;
    }

    uint16_t healAmount = equippedWeapon.getHealthRestore();

    uint16_t manaCost = equippedWeapon.getManaWaste();

    if (manaCost != 0 && !infiniteMana) {
        data.mana = (data.mana >= manaCost) ? data.mana - manaCost : 0;
    }

    if ((data.health + healAmount) > maxHealth) {
        healAmount = maxHealth - data.health;
    }

    data.health += healAmount;

    return healAmount;
}

void Player::setSkin(uint8_t bodySkinId, uint8_t headSkinId) {
    data.bodySkinId = bodySkinId;
    data.headSkinId = headSkinId;
}

void Player::changeMapId(uint8_t newMapId) {
    data.mapId = newMapId;
}

void Player::switchMeditationState() {
    isMeditating = !isMeditating;
}

void Player::restoreHealthThroughTime() {
    uint16_t healthRestore = StatsDefinition().recoveryStatThroughTime(data.race);
    if ((data.health + healthRestore) > maxHealth) {
        healthRestore = maxHealth - data.health;
    }
    data.health += healthRestore;
}

void Player::restoreManaThroughTime() {
    uint16_t manaRestore = StatsDefinition().recoveryStatThroughTime(data.race);
    if ((data.mana + manaRestore) > maxMana) {
        manaRestore = maxMana - data.mana;
    }
    data.mana += manaRestore;
}

void Player::restoreManaForMeditation() {
    if (isMeditating) {
        uint16_t manaRestore = StatsDefinition().meditationManaRestore(data.race, data.class_);
        if ((data.mana + manaRestore) > maxMana) {
            manaRestore = maxMana - data.mana;
        }
        data.mana += manaRestore;
    }
}

void Player::startTeleporting() {
    teleporting = true;
}

void Player::finishTeleporting() {
    teleporting = false;
}

void Player::revive() {
    data.isGhost = false;
    data.health = maxHealth;
}

void Player::resetStats() {
    maxHealth = StatsDefinition().maxHealth(data.level, data.race, data.class_);
    maxMana = StatsDefinition().maxMana(data.level, data.race, data.class_);
    data.health = maxHealth;
    data.mana = maxMana;
    data.isGhost = false;
}

void Player::kill() {
    data.health = 0;
    data.isGhost = true;
    isMeditating = false;
}

bool Player::toggleInfiniteHealth() {
    infiniteHealth = !infiniteHealth;
    if (infiniteHealth) {
        data.health = maxHealth;
        data.isGhost = false;
    }
    return infiniteHealth;
}

bool Player::toggleInfiniteMana() {
    infiniteMana = !infiniteMana;
    if (infiniteMana) {
        data.mana = maxMana;
    }
    return infiniteMana;
}

void Player::levelUp() {
    if (data.level < 255) {
        data.level++;
    }
    data.experience = 0;
    // Solo se actualizan los topes; vida/mana actuales se mantienen.
    maxHealth = StatsDefinition().maxHealth(data.level, data.race, data.class_);
    maxMana = StatsDefinition().maxMana(data.level, data.race, data.class_);
}

void Player::grantExp(uint32_t amount) {
    if (amount == 0) return;
    StatsDefinition stats;
    data.experience += amount;
    // Si supera el limite, sube de nivel. Loop por si gano mucha exp de una.
    while (data.level < 255) {
        uint32_t limit = stats.nextLevelExp(data.level);
        if (data.experience < limit) break;
        data.experience -= limit;
        data.level++;
        // Solo se recalculan maxHp/maxMana. Vida/mana/oro actuales no se tocan.
        maxHealth = stats.maxHealth(data.level, data.race, data.class_);
        maxMana = stats.maxMana(data.level, data.race, data.class_);
    }
}

void Player::addGold(uint32_t amount) {
    uint32_t cap = StatsDefinition().goldMax(data.level);
    if (data.gold >= cap) return;
    uint32_t room = cap - data.gold;
    data.gold += std::min(amount, room);
}

bool Player::removeGold(uint32_t amount) {
    if (data.gold < amount) return false;
    data.gold -= amount;
    return true;
}
