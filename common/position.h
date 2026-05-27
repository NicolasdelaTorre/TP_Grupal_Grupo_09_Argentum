#ifndef POSITION_H
#define POSITION_H

#include <cstdint>

// Coordenada (x, y) en tiles sobre el mapa. Usada por Player, NPC e items
// del juego que necesiten una posición discreta.
struct Position {
    int16_t x;
    int16_t y;
};

#endif  // POSITION_H
