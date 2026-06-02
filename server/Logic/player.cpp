#include "player.h"

#include <utility>

Player::Player(const std::string& name, Position position, const std::string& race,
               const std::string& class_):
        name(name), inventory(N) {
    data.experience = 0;
    data.gold = StatsDefinition().safeGold(data.level);

    data.position = position;
    data.health = StatsDefinition().maxHealth(data.level, race, class_);
    data.mana = 100;

    data.race = Race::fromString(race);
    data.class_ = Class_::fromString(class_);
    data.mapId = 0;
    data.level = 1;
    data.equippedWeapon = 0;
    data.equippedArmor = 0;
    data.equippedHelmet = 0;
    data.equippedShield = 0;
    data.isGhost = false;

    for (int i = 0; i < N; ++i) {
        data.inventory[i] = 0;
    }
}

Player::Player(PlayerData data, const std::string& name): data(std::move(data)), name(name) {}

void Player::move(Position newPosition) { data.position = newPosition; }

void Player::setDirection(uint8_t dir) { direction = dir; }

const std::string& Player::getName() const { return name; }

Position Player::getPosition() const { return data.position; }

int16_t Player::getX() const { return data.position.x; }

int16_t Player::getY() const { return data.position.y; }

uint8_t Player::getDirection() const { return direction; }

PlayerData Player::getData() const { return data; }

bool Player::hasLongDistanceWeapon() { return equippedWeapon.longDistance(); }

void Player::receiveDamage(uint16_t damage) {
    // Cambiar proximamente
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
    if (equippedWeapon.emptyItem()) {
        return 0;
    }

    return StatsDefinition().damage(Race::toString(data.race), equippedWeapon.getMinDamage(),
                                    equippedWeapon.getMaxDamage());
}

bool Player::addItem(const std::string& itemName) {
    Item newItem;
    newItem.createItem(itemName);
    if (newItem.getType() == ItemType::WEAPON && inventory.size() < N) {
        inventory.push_back(newItem);
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

    switch (itemToEquip.getType()) {
        case ItemType::WEAPON:
            equippedWeapon = itemToEquip;
            break;
        case ItemType::ARMOR:
            equippedArmor = itemToEquip;
            break;
        case ItemType::HELMET:
            equippedHelmet = itemToEquip;
            break;
        case ItemType::SHIELD:
            equippedShield = itemToEquip;
            break;
        default:
            return false;  // No se pueden equipar otros tipos de items
    }

    // Eliminar el item del inventario
    inventory.erase(inventory.begin() + inventorySlot);
    return true;
}
