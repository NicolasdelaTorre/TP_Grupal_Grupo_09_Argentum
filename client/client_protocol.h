//
// Created by nicolas on 17/5/26.
//

#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_PROTOCOL_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_PROTOCOL_H
#include <string>
#include <vector>

#include "../common/DTOs.h"
#include "../common/common_protocol.h"
#include "../common/message_types.h"
#include "../common/position.h"
#include "../common/socket.h"

// Celda como la manda el servidor (walkable = obstacleId == 0).
struct ReceivedCell {
    uint16_t textureId;
    uint16_t obstacleId;
    bool safeZone;
};

// Mapa recibido del servidor. cells en row-major: idx = y * width + x.
struct ReceivedMap {
    uint16_t width;
    uint16_t height;
    std::vector<ReceivedCell> cells;
};

// Evento de un jugador (NEW_PLAYER y PLAYER_MOVED). En PLAYER_MOVED `name` queda vacío.
// dir usa valores wire: 3=TOP, 4=BOTTOM, 5=LEFT, 6=RIGHT.
struct PlayerEvent {
    uint16_t id;
    int16_t x;
    int16_t y;
    uint8_t dir;
    uint8_t skin;
    std::string name;
};

// Stats del jugador local (STATS_JUGADOR).
struct StatsEvent {
    uint16_t health;
    uint16_t maxHealth;
    uint8_t level;
};

// Resultado de un ataque (ATTACK_RESULT). Broadcast a todos.
struct AttackResultEvent {
    uint16_t attackerId;
    uint8_t targetType;  // 0 = player, 1 = npc
    uint16_t targetId;
    uint16_t damage;
    bool hit;  // false => evasión, damage = 0
};

// Snapshot del inventario del jugador local (INVENTORY_UPDATE).
// `items` son ids de cada item en una slot ocupada (sin slots vacías).
// Los `equipped*` son ids de item, 0 = nada equipado.
struct InventoryEvent {
    std::vector<uint8_t> items;
    uint8_t equippedWeapon;
    uint8_t equippedArmor;
    uint8_t equippedHelmet;
    uint8_t equippedShield;
};

class client_protocol {
    common_protocol protocol;

public:
    explicit client_protocol(Socket skt);

    // Enviar movimiento: direction debe ser TOP, BOTTOM, LEFT o RIGHT
    void send_move(ClientMsg direction);

    // Enviar cambio de dirección sin moverse (girar en la misma celda).
    void send_turn(ClientMsg direction);

    // Leer el tipo del próximo mensaje que mandó el servidor (1 byte de opcode)
    ServerMsg recv_msg_type();

    // Cierra el socket — desbloquea cualquier recv pendiente
    void close();

    // Envío del login: [USER_ARRIVAL][len:2][name][len:2][race][len:2][class].
    int send_user_arrival(const std::vector<char>& name, const std::string& race,
                          const std::string& class_);

    // Las funciones recv_*_payload asumen que el opcode ya fue consumido vía recv_msg_type().
    uint16_t recv_my_player_id();
    Position recv_login_ok_payload();
    ReceivedMap recv_map();
    PlayerEvent recv_new_player_payload();
    PlayerEvent recv_player_moved_payload();
    uint16_t recv_player_disconnected_payload();
    StatsEvent recv_stats_payload();
    AttackResultEvent recv_attack_result_payload();
    InventoryEvent recv_inventory_update_payload();

    // Envía la skin elegida en la pantalla de creación de personaje.
    void send_skin_selected(uint8_t skinId);

    // Envía la cabeza elegida en la pantalla de creación de personaje.
    void send_head_selected(uint8_t headId);

    // Envía un ataque en la dirección dada (TOP, BOTTOM, LEFT, RIGHT).
    // El server resuelve quién es el target por línea de vista
    void send_attack(ClientMsg direction);

    // Envía un ataque a un target específico (para arcos/magia que apuntan a un jugador o NPC). targetType: 0 = player, 1 = npc. Server valida rango.
    void send_targeted_attack(uint8_t targetType, uint16_t targetId);

    // Recibe la lista completa de NPCs dinámicos (criaturas).
    std::vector<NpcEntity> recv_npc_list_payload();

    // Recibe la lista de items tirados en el piso.
    std::vector<DroppedItem> recv_dropped_items_payload();

    void sendCheat(CheatCode cheat);

    // /tomar — server resuelve qué item hay en la celda del jugador.
    void send_pick_up_item();
    // /tirar — slot del inventario.
    void send_drop_item(uint8_t invSlot);
    // Equipar o usar (poción) el item de la slot. El server decide según tipo.
    void send_equip_item(uint8_t invSlot);
    // Desequipar slot_type: 0=arma, 1=armadura, 2=casco, 3=escudo.
    void send_unequip_item(uint8_t slotType);
};


#endif  // TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_PROTOCOL_H
