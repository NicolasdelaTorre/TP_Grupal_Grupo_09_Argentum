#include "item_sprites.h"

namespace {

// Geometría por sheet (cols, ancho/alto de celda px, origen px del grid).
// celda = fila*cols + col, contando desde 0 arriba-izquierda en ese sheet.
//   INV    -> Items_inventario.png     : 31 cols, 32x32 (padding raro)
//   REC1   -> Items_recolectables.png  : 16 cols, 32x32
//   REC2   -> Items_recolectables_2.png: 16 cols, 32x32
//   GORROS -> Skins/Gorros.png : tira de iconos abajo, arrancando en y=764.
//             18 cols x 5 filas, 32x32; el índice de icono == helmetId.
enum ItemSheet { INV = 0, REC1 = 1, REC2 = 2, GORROS = 3 };

struct SheetGeom {
    const char* path;
    int cols;
    int cellW;
    int cellH;
    int originX;
    int originY;
};

constexpr SheetGeom kSheets[] = {
    {"/Pantallas/Items_inventario.png", 31, 32, 32, 0, 0},
    {"/Pantallas/Items_recolectables.png", 16, 32, 32, 0, 0},
    {"/Pantallas/Items_recolectables_2.png", 16, 32, 32, 0, 0},
    {"/Skins/Gorros.png", 18, 32, 32, 0, 764},
};

// índice = id del item (items.toml), valor = {sheet, celda}.
// El id 0 no se usa. Para items que no estén en INV, poner REC1/REC2/GORROS.
struct ItemSprite {
    ItemSheet sheet;
    int cell;
};

constexpr ItemSprite kItemSprite[] = {
    {INV, 0},      // 0  -> sin usar
    {INV, 1},      // 1  Sword
    {INV, 93},     // 2  Axe
    {INV, 5},      // 3  Hammer
    {INV, 96},     // 4  Ash Staff
    {REC1, 68},    // 5  Elven Flute
    {INV, 220},    // 6  Root Staff
    {INV, 241},    // 7  Socketed Staff
    {INV, 10},     // 8  Simple Bow
    {INV, 127},    // 9  Composite Bow
    {INV, 25},     // 10 Lether Armor
    {INV, 31},     // 11 Plate Armor
    {INV, 51},     // 12 Blue Tunic
    {GORROS, 0},   // 13 Hood
    {GORROS, 8},   // 14 Iron Helmet
    {INV, 184},    // 15 Turtle Shield
    {INV, 142},    // 16 Iron Shield
    {GORROS, 20},  // 17 Wizard Hat
    {REC2, 33},    // 18 Health Potion
    {REC2, 32},    // 19 Mana Potion
};
constexpr int kItemSpriteCount = sizeof(kItemSprite) / sizeof(kItemSprite[0]);

}  // namespace

ItemSpriteRef itemSpriteFor(uint16_t itemId) {
    const ItemSprite& sp = (itemId < kItemSpriteCount) ? kItemSprite[itemId] : kItemSprite[0];
    const SheetGeom& g = kSheets[sp.sheet];
    return {
        g.path,
        g.originX + (sp.cell % g.cols) * g.cellW,
        g.originY + (sp.cell / g.cols) * g.cellH,
        g.cellW,
        g.cellH,
    };
}
