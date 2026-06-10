#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_RENDER_CONSTANTS_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_RENDER_CONSTANTS_H

#include <cstdint>

// Constantes y enums propios del renderer/cliente. No viajan por el wire.

static constexpr float ANIM_SPEED = 0.15f;        // segundos por frame
static constexpr float PLAYER_MOVE_SPEED = 4.0f;  // tiles por segundo
static constexpr int TILE_SIZE = 64;
static constexpr int SPRITE_W = 27;
static constexpr int SPRITE_H = 48;
static constexpr int PHANTOM_SPRITE_W = 32;
static constexpr int PHANTOM_SPRITE_H = 64;
static constexpr int ANIM_FRAMES = 4;

// Fila del spritesheet del personaje segun direccion visual. Es solo del
// renderer del cliente: no viaja en ningun evento.
enum class SpriteRow : uint8_t { UP = 1, LEFT = 2, DOWN = 0, RIGHT = 3 };

#endif
