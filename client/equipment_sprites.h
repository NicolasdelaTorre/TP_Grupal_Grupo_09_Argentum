#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_EQUIPMENT_SPRITES_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_EQUIPMENT_SPRITES_H

#include <cstdint>

// Mapeo itemId (items.toml) -> sprite EQUIPADO (worn) que dibuja el MapRenderer
// sobre el cuerpo del jugador. Es un espacio DISTINTO al de los iconos del
// inventario (ver item_sprites.h / itemSpriteFor): un mismo item usa un PNG para
// el icono del inventario y otro recurso para verse puesto encima del personaje
// (p.ej. todos los cascos/gorros salen de Skins/Gorros.png).
//
// La animación (dirección = fila, frame = columna) la resuelve el renderer; acá
// solo decidimos QUÉ recurso y QUÉ variante usar para cada item.

enum class EquipSlot { NONE, WEAPON, ARMOR, HELMET, SHIELD };

struct EquipVisual {
    EquipSlot slot = EquipSlot::NONE;
    // Significado de index según slot (mismas convenciones que el MapRenderer):
    //   WEAPON -> índice en weaponFiles[]: 0 Espada, 1 Daga, 2 Arco, 3 Baculo
    //   ARMOR  -> índice de skin de cuerpo (get_path): 0..4
    //   HELMET -> Gorros.png empaquetado block*32+col (block = grupo de 4 filas
    //             Down/Up/Left/Right; ver MapRenderer::renderHelmet)
    //   SHIELD -> índice en shieldFiles[]: 0 Escudo
    int index = -1;
};

// Devuelve el sprite worn para un itemId. itemId desconocido / no vestible
// (pociones, etc.) -> EquipSlot::NONE.
EquipVisual equipVisualFor(uint8_t itemId);

#endif
