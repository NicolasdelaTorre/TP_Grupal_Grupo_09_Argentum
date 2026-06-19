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

// Tipo lógico de una celda con obstáculo. Viaja como obstacleId en cada Cell.
// El sprite de cada obstáculo lo decide su textura (MapObstacleData::texture),
// así que acá NO se enumeran los obstáculos concretos: solo quedan las
// categorías que tienen lógica (bloqueo genérico, entradas y NPCs de ciudad).
enum class ObstacleCode : uint8_t {
    NONE = 0,
    GENERIC = 1,   // obstáculo sólido común: bloquea el paso, sin lógica especial
    ENTRY = 2,     // entrada a una dungeon (se camina sobre ella para entrar)
    NPC = 3,       // NPC fijo de ciudad genérico
    NPC_PRIEST = 4,
    NPC_MERCHANT = 5,
    NPC_BANKER = 6,
};

enum class RaceCode : uint8_t { HUMAN = 0, ELF, DWARF, GNOME };

enum class ClassCode : uint8_t { MAGE = 0, CLERIC, CHAMPION, WARRIOR };

enum class WeaponCode : uint8_t { MELEE = 0, RANGED };

// Tipo de target en un AttackEvent
enum class TargetType : uint8_t { PLAYER = 0, NPC = 1 };

// Slot de equipamiento
enum class EquipmentSlot : uint8_t {
    WEAPON = 0,
    ARMOR = 1,
    HELMET = 2,
    SHIELD = 3
};
constexpr uint8_t EQUIPMENT_SLOT_COUNT = 4;

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
