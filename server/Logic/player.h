#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include <string>

#include "../../common/position.h"

// Entidad jugador en el mundo del juego. Es dueña única de su posición y
// (a futuro) de su vida, mana, oro, etc. El Game valida los movimientos
// contra el mapa y los otros jugadores antes de mutar este estado.
class Player {
private:
    std::string name;
    Position position;

public:
    Player(std::string name, Position position);

    // Mueve al jugador a una nueva posición. El Game ya validó que es legal.
    void move(Position newPosition);

    const std::string& getName() const;
    Position getPosition() const;
    int16_t getX() const;
    int16_t getY() const;
};

#endif
