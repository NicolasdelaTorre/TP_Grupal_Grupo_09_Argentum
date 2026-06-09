#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_DTOS_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_DTOS_H

#include <cstdint>

// Data Transfer Objects: tipos compartidos entre cliente y server. Lo que
// vive aca o viaja por el wire (enums con valor binario). NO debe haber
// logica, constantes de renderer ni structs propios de cliente/server.
// Los structs visuales del cliente viven en client/visual_types.h.

// Direcciones de movimiento/giro (MovementEvent / TurnEvent).
enum class MoveDirection : uint8_t {
    TOP = 3,
    BOTTOM = 4,
    LEFT = 5,
    RIGHT = 6,
};

// Tipos de tile (piso). Viaja como byte en el wire.
enum class TileCode : uint8_t {
    GRASS = 0,
    WATER,
    DIRT,
    SAND,
    INTERIOR  // piso de ciudad / interior
};

// Tipos de obstaculo. Viaja como obstacleId en cada Cell del mapa.
enum class ObstacleCode : uint8_t {
    NONE = 0,
    ROCK = 1,
    TREE = 2,
    NPC = 3,
    ENTRY = 4,
    WALL = 5,
    ROCK_SMALL = 6,
    ROCK_LARGE = 7,
    LAMP = 8,
    WOOD = 9,
    CART = 10,
    MILL = 11,
    CACTUS = 12,
    BUSH = 13,
    NPC_PRIEST = 14,
    NPC_MERCHANT = 15,
    NPC_BANKER = 16,
    BANK = 17,
    HOUSE_BLUE = 18,
    HOUSE_RED = 19,
    HOUSE_SNOW = 20,
    FENCE = 21,
    TARGET = 22,
    HAYBALE = 23,
    FOUNTAIN = 24,
    BLACKSMITH = 25,
    HOTEL = 26,
    CHURCH = 27,
    TRAINING_DUMMY = 28,
    EXIT = 29,
};

enum class RaceCode : uint8_t { HUMAN = 0, ELF, DWARF, GNOME };

enum class ClassCode : uint8_t { MAGE = 0, CLERIC, CHAMPION, WARRIOR };

enum class WeaponCode : uint8_t { MELEE = 0, RANGED };

// Tipo de criatura NPC. SPIDER..GOLEM son hostiles, el resto amigos de ciudad.
enum class NpcCode : uint8_t {
    SPIDER = 0,
    SKELETON,
    ZOMBIE,
    GOBLIN,
    ORC,
    GOLEM,
    MERCHANT,
    BANKER,
    PRIEST
};

#endif
