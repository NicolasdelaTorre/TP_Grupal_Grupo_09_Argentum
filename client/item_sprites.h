#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_ITEM_SPRITES_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_ITEM_SPRITES_H

#include <cstdint>

// Mapeo único item (items.toml) -> recorte del spritesheet. El orden de los
// sheets NO coincide con el id del item, así que se mapea a mano (ver tabla en
// item_sprites.cpp). Lo usan tanto el panel de inventario como los items
// tirados en el piso, para que el mismo id pinte siempre el mismo dibujo.
struct ItemSpriteRef {
    const char* sheetPath;  // path relativo de assets, p.ej "/Pantallas/Items_inventario.png"
    int srcX, srcY;         // origen del recorte dentro del sheet, en px
    int srcW, srcH;         // tamaño de la celda, en px
};

// Resuelve el id de item (items.toml, 1-based) a su recorte. Para un id sin
// mapear devuelve la celda 0 de Items_inventario.png (placeholder).
ItemSpriteRef itemSpriteFor(uint16_t itemId);

#endif
