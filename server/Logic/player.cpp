#include "player.h"

#include <utility>

Player::Player(const std::string& name, Position position, const std::string& race,
               const std::string& class_):
        name(name) {
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
