#include "player.h"

#include <utility>

Player::Player(const std::string& name, Position position, const std::string& race,
               const std::string& class_):
        name(name) {
    data.position = position;
    data.level = 1;
    data.race = Race::fromString(race);
    data.class_ = Class_::fromString(class_);
    data.maxHealth = StatsDefinition().maxHealth(data.level, race, class_);
    data.health = data.maxHealth;
}

void Player::move(Position newPosition) { data.position = newPosition; }

void Player::setDirection(uint8_t dir) { direction = dir; }

const std::string& Player::getName() const { return name; }

Position Player::getPosition() const { return data.position; }

int16_t Player::getX() const { return data.position.x; }

int16_t Player::getY() const { return data.position.y; }

uint8_t Player::getDirection() const { return direction; }
