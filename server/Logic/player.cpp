#include "player.h"

#include <utility>

Player::Player(std::string name, Position position):
        name(std::move(name)), position(position) {}

void Player::move(Position newPosition) { position = newPosition; }

void Player::setDirection(uint8_t dir) { direction = dir; }

const std::string& Player::getName() const { return name; }

Position Player::getPosition() const { return position; }

int16_t Player::getX() const { return position.x; }

int16_t Player::getY() const { return position.y; }

uint8_t Player::getDirection() const { return direction; }
