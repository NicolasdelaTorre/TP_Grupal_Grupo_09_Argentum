#include "equipment_sprites.h"

namespace {

// índice = itemId (items.toml). Ver items.toml para tipo/nombre.
// OJO con los recursos disponibles (worn):
//   Armas/: Espada, Daga, Arco, Baculo            -> weaponFiles[] = {0,1,2,3}
//   Armas/Escudo.png                              -> shieldFiles[] = {0}
//   Skins de cuerpo (get_path): 0 Skin_inicial, 1 Armadura_de_cuero,
//                               2 Gladiador_azul, 3 Hechicero, 4 Hechicera
//   Skins/Gorros.png: bloques de 4 filas (Down/Up/Left/Right) x 32 columnas,
//                     celda 32x64. helmetId empaquetado = block*32 + col.
//
// [REVISAR]: marcados así donde no hay un recurso 1:1 y elegí un placeholder.
// Cambiá el index cuando sumen el sprite real.
constexpr EquipVisual kEquip[] = {
    {EquipSlot::NONE, -1},     // 0  sin usar
    {EquipSlot::WEAPON, 0},    // 1  Sword          -> Espada
    {EquipSlot::WEAPON, 1},    // 2  Axe            -> Hacha       
    {EquipSlot::WEAPON, 6},    // 3  Hammer         -> Martillo    [REVISAR: no hay png de martillo]
    {EquipSlot::WEAPON, 4},    // 4  Ash Staff      -> Baculo
    {EquipSlot::WEAPON, 5},    // 5  Elven Flute    -> no se viste [REVISAR: item HEAL]
    {EquipSlot::WEAPON, 4},    // 6  Root Staff     -> Baculo
    {EquipSlot::WEAPON, 4},    // 7  Socketed Staff -> Baculo
    {EquipSlot::WEAPON, 2},    // 8  Simple Bow     -> Arco
    {EquipSlot::WEAPON, 3},    // 9  Composite Bow  -> Arco
    {EquipSlot::ARMOR, 1},     // 10 Lether Armor   -> Armadura_de_cuero
    {EquipSlot::ARMOR, 2},     // 11 Plate Armor    -> Gladiador_azul [REVISAR]
    {EquipSlot::ARMOR, 3},     // 12 Blue Tunic     -> Hechicero      [REVISAR]
    {EquipSlot::HELMET, 79},   // 13 Hood           -> Gorros block 2 col 15 (2*32+15)
    {EquipSlot::HELMET, 38},   // 14 Iron Helmet    -> Gorros block 1 col 6  (1*32+6)
    {EquipSlot::SHIELD, 0},    // 15 Turtle Shield  -> Escudo
    {EquipSlot::SHIELD, 0},    // 16 Iron Shield    -> Escudo
    {EquipSlot::HELMET, 35},   // 17 Wizard Hat     -> Gorros block 1 col 3  (1*32+3)
    {EquipSlot::NONE, -1},     // 18 Health Potion  -> no se viste
    {EquipSlot::NONE, -1},     // 19 Mana Potion    -> no se viste
};
constexpr int kEquipCount = sizeof(kEquip) / sizeof(kEquip[0]);

}  // namespace

EquipVisual equipVisualFor(uint8_t itemId) {
    if (itemId >= kEquipCount)
        return {EquipSlot::NONE, -1};
    return kEquip[itemId];
}
