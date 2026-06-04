//
// Created by nicolas on 19/5/26.
//

#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_DTOS_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_DTOS_H

#include <cstdint>
#include <string>
#include <vector>

// enum class Command { ... };
using Command = std::string;

// enum class ServerMessageType { ... };
using ServerMessageType = std::string;


static constexpr float ANIM_SPEED = 0.15f;        // segundos por frame
static constexpr float PLAYER_MOVE_SPEED = 4.0f;  // tiles por segundo
static constexpr int TILE_SIZE = 64;              // tiles son 128x128
static constexpr int SPRITE_W = 27;               // frame del personaje
static constexpr int SPRITE_H = 49;
static constexpr int PHANTOM_SPRITE_W = 32;  // distancia en tiles del centro del cuerpo a la cabeza
static constexpr int PHANTOM_SPRITE_H = 64;
static constexpr int ANIM_FRAMES = 4;  // columnas del spritesheet


struct LoginResult {
    std::vector<char> username;
    bool confirmed;
};


// ── Tipos de tile ─────────────────────────────────────────────
enum class TileType : uint8_t {
    GRASS = 0,
    WATER,
    DIRT,
    SAND,
    INTERIOR  // piso de ciudad / interior
};

// Tipos de obstáculo. Se mandan como obstacleId en cada Cell del mapa.
enum class ObstacleType : uint8_t {
    NONE = 0,
    ROCK = 1,  // roca (3x3) → 7225.png
    TREE = 2,  // arbol / arbol_grande / tronco (sin textura)
    NPC = 3,   // NPC genérico (fallback)
    ENTRY = 4,
    WALL = 5,
    ROCK_SMALL = 6,     // piedra_pequenia (2x2) → roca_03_ajustada.png
    ROCK_LARGE = 7,     // piedra_grande (6x4)   → roca_01_ajustada.png
    LAMP = 8,           // lampara_ciudad  (1x1) → lampara_corregida.png
    WOOD = 9,           // pila_maderas    (1x1) → maderas_apiladas_corregida.png
    CART = 10,          // carretilla      (2x2) → segunda_carretilla_primera_fila.png
    MILL = 11,          // molino          (6x4) → molino_recortado.png
    CACTUS = 12,        // cactus          (1x1) → cactus_arriba_derecha_128x128.png
    BUSH = 13,          // arbusto         (1x1) (sin textura)
    NPC_PRIEST = 14,    // sacerdote → Sacerdote.png
    NPC_MERCHANT = 15,  // comerciante → Sacerdote.png
    NPC_BANKER = 16,    // banquero → Sacerdote.png
};

// ── Dirección del personaje ───────────────────────────────────
enum class Direction : uint8_t { UP = 1, LEFT = 2, DOWN = 0, RIGHT = 3 };

enum class Direction_phantom : uint8_t { UP = 1, LEFT = 3, DOWN = 0, RIGHT = 2 };

enum class Race : uint8_t { HUMAN = 0, ELF, DWARF, GNOME };

enum class Classtype : uint8_t { MAGE = 0, CLERIC, PALADIN, WARRIOR };

enum class WeaponType : uint8_t { MELEE = 0, RANGED };

struct Obj {
    uint8_t id;
    std::string name;
    std::string imagePath;
};

struct Weapon: public Obj {
    WeaponType type;
    uint8_t damage;
};

// Tipo de criatura NPC dinámica
enum class NpcType : uint8_t { SPIDER = 0, SKELETON, ZOMBIE, GOBLIN, ORC, GOLEM };

enum class CheatCode : uint8_t { SUICIDE = 0, GOLD, EXPERIENCE };

struct NpcEntity {
    uint16_t id = 0;
    float x = 0, y = 0;
    Direction dir = Direction::DOWN;
    bool moving = false;
    int animFrame = 0;
    float animTimer = 0.0f;
    NpcType type = NpcType::SPIDER;
};

// Item dropped on the floor. x/y are tile coordinates.
// sheetId 0 → Items_recolectables.png, 1 → Items_recolectables_2.png, 2 → Items_recolectables_3.png
struct DroppedItem {
    int16_t x = 0, y = 0;
    uint8_t sheetId = 0;
    uint16_t itemId = 0;  // row * cols_per_row + col
};

struct Player {
    float x = 5.0f, y = 5.0f;  // posición en tiles
    Direction dir = Direction::DOWN;
    bool moving = false;
    int animFrame = 0;
    float animTimer = 0.0f;
    int skin = 2;
    int headId = 4;
    int weaponId = 0;  // 0=Espada, 1=Daga, 2=Arco, 3=Baculo, -1=sin arma
    Race race = Race::HUMAN;
    Classtype classtype = Classtype::MAGE;
    std::vector<Obj> inventory;
    bool killed = false;
};

#endif  // TP_GRUPAL_GRUPO_09_ARGENTUM_DTOS_H
