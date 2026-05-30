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
static constexpr int TILE_SIZE = 32;              // tiles son 128x128
static constexpr int SPRITE_W = 27;               // frame del personaje
static constexpr int SPRITE_H = 49;
static constexpr int ANIM_FRAMES = 4;  // columnas del spritesheet


struct LoginResult {
    std::vector<char> username;
    bool confirmed;  // true = presionó Enter, false = cerró la ventana
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
    NONE  = 0,
    ROCK  = 1,
    TREE  = 2,
    NPC   = 3,
    ENTRY = 4,
    WALL  = 5,
};

// ── Dirección del personaje ───────────────────────────────────
enum class Direction : uint8_t { UP = 1, LEFT = 2, DOWN = 0, RIGHT = 3 };

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

struct Player {
    float x = 5.0f, y = 5.0f;  // posición en tiles
    Direction dir = Direction::DOWN;
    bool moving = false;
    int animFrame = 0;
    float animTimer = 0.0f;
    int headId = 4;
    int weaponId = 0;  // 0=Espada, 1=Daga, 2=Arco, 3=Baculo, -1=sin arma
    Race race = Race::HUMAN;
    Classtype classtype = Classtype::MAGE;
    std::vector<Obj> inventory;
};

#endif  // TP_GRUPAL_GRUPO_09_ARGENTUM_DTOS_H
