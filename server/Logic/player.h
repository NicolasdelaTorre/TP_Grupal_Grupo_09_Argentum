#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "../../common/position.h"
#include "../../common/DTOs.h"

#include "class_.h"
#include "items.h"
#include "race.h"
#include "stats_definition.h"

#define N 9

struct PlayerData {
    uint32_t experience;
    uint32_t gold;

    Position position;
    uint16_t health;
    uint16_t mana;

    RaceCode race;
    ClassCode class_;
    uint8_t mapId;
    uint8_t level;
    uint8_t equippedWeapon;
    uint8_t equippedArmor;
    uint8_t equippedHelmet;
    uint8_t equippedShield;
    uint8_t headSkinId;
    uint8_t bodySkinId;
    bool isGhost;

    uint8_t inventory[N];
};

class Player {
private:
    PlayerData data;
    std::string name;
    // dirección actual (valores wire: 3=TOP, 4=BOTTOM, 5=LEFT, 6=RIGHT)
    uint8_t direction = 4;
    std::vector<Item> inventory;
    Item equippedWeapon;
    Item equippedArmor;
    Item equippedHelmet;
    Item equippedShield;
    uint16_t maxHealth;
    uint16_t maxMana;
    bool isMeditating;
    // Flags de cheats runtime: NO se guardan en PlayerData (no persisten al reloguear).
    bool infiniteHealth = false;
    bool infiniteMana = false;

public:
    explicit Player(const std::string& name, Position position, RaceCode race, ClassCode class_);

    Player(PlayerData data, const std::string& name);

    void move(Position newPosition);

    void setDirection(uint8_t dir);

    const std::string& getName() const;

    Position getPosition() const;

    int16_t getX() const;

    int16_t getY() const;

    uint8_t getDirection() const;

    PlayerData getData() const;

    std::vector<Item> getInventory() const;

    uint16_t getMaxHealth() const;

    uint16_t getMaxMana() const;

    uint8_t getMapId() const;

    bool getMeditationState() const;

    bool hasLongDistanceWeapon();

    void receiveDamage(uint16_t damage);

    bool isEquipped();

    bool isAlive();

    uint16_t dealDamage();

    bool addItem(const std::string& itemName);

    bool equipItem(int inventorySlot);

    bool unequipItem(ItemType type);

    uint16_t heal();

    void setSkin(uint8_t bodySkinId, uint8_t headSkinId);

    void changeMapId(uint8_t newMapId);

    void switchMeditationState();

    void restoreManaForMeditation();

    void resetStats();

    // Mata al jugador instantaneamente (HP=0, fantasma). Ignora infiniteHealth:
    // se usa para acciones voluntarias como /suicidio, no para damage de combate.
    void kill();

    // Cheats runtime (no persisten). Devuelven el nuevo estado del flag.
    bool toggleInfiniteHealth();
    bool toggleInfiniteMana();

    // Sube un nivel y refresca stats al maximo segun raza/clase.
    void levelUp();

    // Suma oro respetando el cap = 100 * Nivel^1.1 (formula del enunciado).
    void addGold(uint32_t amount);
};

#endif
