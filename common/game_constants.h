#ifndef GAME_CONSTANTS_H
#define GAME_CONSTANTS_H

#include <cstddef>
#include <cstdint>

// Constantes de gameplay compartidas entre cliente y server.

// Item id reservado para "drop de oro" en el suelo.
static constexpr uint8_t GOLD_ITEM_ID = 254;

// Author id que usa el server para mandar mensajes de sistema por chat.
static constexpr uint16_t SYSTEM_AUTHOR_ID = 0;

// Rango para armas a distancia / hechizos.
static constexpr int16_t COMBAT_RANGE = 3;

// Tamaño del inventario de un jugador.
static constexpr std::size_t INVENTORY_SIZE = 20;

#endif
