#include "player.h"

Player::Player(const std::string& name): data{name, {0, 0}} {}

void Player::changePosition(const std::string& direction) {
    if (direction == "top") {
        data.position.y -= 1;
    } else if (direction == "bottom") {
        data.position.y += 1;
    } else if (direction == "left") {
        data.position.x -= 1;
    } else if (direction == "right") {
        data.position.x += 1;
    }
}

int16_t Player::getX() { return data.position.x; }

int16_t Player::getY() { return data.position.y; }
