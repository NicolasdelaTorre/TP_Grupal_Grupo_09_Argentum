#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_VISUAL_TYPES_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_VISUAL_TYPES_H

#include <cstdint>
#include <vector>

#include "../common/DTOs.h"

#include "render_constants.h"

// Tipos de estado visual del cliente. NO viajan por la red; los arma el
// cliente para alimentar el renderer (posicion en floats interpolada,
// animacion, etc.).

// Resultado del LoginScreen.
struct LoginResult {
    std::vector<char> username;
    bool confirmed;
};


constexpr int SKIN_DEFAULT = 0;  // índice del skin por defecto (columna en Skins.png)
// Item tirado al piso (DroppedItemEvent). sheetId y itemId mapean al
// spritesheet de items. dropId es la clave que asigna el server.
struct DroppedItem {
    uint16_t dropId = 0;
    int16_t x = 0, y = 0;
    uint8_t sheetId = 0;
    uint16_t itemId = 0;
};

// Estado visual de un jugador en el cliente (incluye animacion).
struct Player_ {
    uint16_t id = 0;
    float x = 5.0f, y = 5.0f;
    SpriteRow dir = SpriteRow::DOWN;
    bool moving = false;
    int animFrame = 0;
    float animTimer = 0.0f;
    int skin = SKIN_DEFAULT;
    int headId = 6;
    // -1 = slot vacío (no se renderiza). Los pone applyEquipmentToVisual al
    // recibir PlayerEquippedEvent / InventoryUpdateEvent.
    int helmetId = -1;
    int weaponId = -1;
    int shieldId = -1;
    RaceCode race = RaceCode::HUMAN;
    ClassCode classCode = ClassCode::MAGE;
    bool killed = false;
};

// Estado visual de un NPC en el cliente.
struct NpcEntity {
    uint16_t id = 0;
    float x = 0, y = 0;
    SpriteRow dir = SpriteRow::DOWN;
    bool moving = false;
    int animFrame = 0;
    float animTimer = 0.0f;
    NpcCode type = NpcCode::SPIDER;
};

#endif
