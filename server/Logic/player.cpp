#include "player.h"

#include <utility>
#include <iostream>

Player::Player(const std::string& name, Position position, const std::string& race,
               const std::string& class_):
        name(name) {
    inventory.reserve(N);
    data.level = 1;
    data.experience = 0;
    data.gold = StatsDefinition().safeGold(data.level);

    data.position = position;
    data.health = StatsDefinition().maxHealth(data.level, race, class_);
    maxHealth = data.health;
    data.mana = StatsDefinition().maxMana(data.level, race, class_);
    maxMana = data.mana;

    data.race = Race::fromString(race);
    data.class_ = Class_::fromString(class_);
    data.mapId = 0;
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

Player::Player(PlayerData data, const std::string& name): data(std::move(data)), name(name) {
    inventory.reserve(N);
    maxHealth = StatsDefinition().maxHealth(data.level, Race::toString(data.race),
                                            Class_::ToString(data.class_));
    maxMana = StatsDefinition().maxMana(data.level, Race::toString(data.race),
                                        Class_::ToString(data.class_));

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

void Player::move(Position newPosition) { data.position = newPosition; }

void Player::setDirection(uint8_t dir) { direction = dir; }

const std::string& Player::getName() const { return name; }

Position Player::getPosition() const { return data.position; }

int16_t Player::getX() const { return data.position.x; }

int16_t Player::getY() const { return data.position.y; }

uint8_t Player::getDirection() const { return direction; }

PlayerData Player::getData() const { return data; }

std::vector<Item> Player::getInventory() const { return inventory; }

uint16_t Player::getMaxHealth() const { return maxHealth; }

uint16_t Player::getMaxMana() const { return maxMana; }

bool Player::hasLongDistanceWeapon() { return equippedWeapon.longDistance(); }

void Player::receiveDamage(uint16_t damage) {
    // Cambiar proximamente
    if (data.isGhost) {
        std::cout << "Player " << name << " is already a ghost and can't receive more damage." << std::endl;
        return;
    } else {
        std::cout << "Player " << name << " receives " << damage << " damage!" << std::endl;
    }
    if (damage >= data.health) {
        data.health = 0;
        data.isGhost = true;
    } else {
        data.health -= damage;
    }
}

bool Player::isEquipped() { return !equippedWeapon.emptyItem(); }

bool Player::isAlive() { return !data.isGhost; }

uint16_t Player::dealDamage() {
    if (equippedWeapon.emptyItem() || !equippedWeapon.isOffensiveWeapon()) {
        return 0;
    }

    return StatsDefinition().damage(Race::toString(data.race), equippedWeapon.getMinDamage(),
                                    equippedWeapon.getMaxDamage());
}

bool Player::addItem(const std::string& itemName) {
    Item newItem;
    newItem.createItem(itemName);
    if (inventory.size() < N) {
        inventory.push_back(newItem);
        data.inventory[inventory.size() - 1] = newItem.getId();
        return true;
    }

    return false;
}

bool Player::equipItem(int inventorySlot) {
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
        default:
            throw std::runtime_error("Player Error: trying to equip an item that is not exist");
    }

    return true;
}

bool Player::unequipItem(ItemType type) {
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
    if (equippedWeapon.emptyItem() || equippedWeapon.getType() != ItemType::HEAL) {
        return 0;
    }

    uint16_t healAmount = equippedWeapon.getHealthRestore();

    uint16_t manaCost = equippedWeapon.getManaWaste();

    if (manaCost != 0) {
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

void Player::resetStats() {
    maxHealth = StatsDefinition().maxHealth(data.level, "Elf", "Mage");
    maxMana = StatsDefinition().maxMana(data.level, "Elf", "Mage");
    data.health = maxHealth;
    data.mana = maxMana;
    data.isGhost = false;
}
