#ifndef GAMELOOP_H
#define GAMELOOP_H

#include <memory>
#include <string>

#include "../../common/Communication/events/server_event.h"
#include "../../common/Communication/move_direction.h"
#include "../../common/position.h"
#include "../../common/thread.h"
#include "../Communication/client_monitor.h"
#include "../Communication/server_receiver.h"  // IncomingQueue alias
#include "../Communication/server_protocol.h"

#include "game.h"
#include "turn_manager.h"

class Gameloop: public Thread {
private:
    IncomingQueue& clientEvents;
    ClientMonitor& clientMonitor;
    bool gameFinished;
    Map& map;
    Game game;
    ServerProtocol& protocol;
    TurnManager turnManager;

    // Construye un StatsEvent con el snapshot actual del jugador.
    std::shared_ptr<ServerEvent> buildStatsEvent(int idPlayer);

    // Manda PLAYER_EQUIPPED por cada slot equipado del jugador.
    // recipientId == -1 → broadcast a todos menos a él. Sino, sólo a ese cliente.
    void sendEquipmentSnapshot(int idPlayer, int recipientId);

    // Avanza el turno de cada NPC vivo; si alguno ataca, dispara los eventos al cliente.
    void NPCTurns();

public:
    Gameloop(IncomingQueue& clientEvents, ClientMonitor& clientMonitor, Map& map,
             ServerProtocol& protocol);

    virtual void run() override;

    virtual void stop() override;

    // Despacha el ClientEvent al handle correspondiente (un único switch
    // por tipo, con dynamic_cast). Cada caso vive en su propio handler.
    void dispatch(const ClientEvent& ev);

    void handleDisconnect(int playerId);
    void handleUserArrival(int playerId, const std::string& name, const std::string& race,
                           const std::string& class_);
    void handleSkinSelected(int playerId, uint8_t skinId);
    void handleHeadSelected(int playerId, uint8_t headId);
    void handleMovement(int playerId, MoveDirection direction);
    void handleTurn(int playerId, MoveDirection direction);
    void handleAttack(int playerId, uint8_t targetType, uint16_t targetId);
    void handleCheat(int playerId, uint8_t code);
    void handlePickUp(int playerId);
    void handleDrop(int playerId, uint8_t invSlot);
    void handleEquip(int playerId, uint8_t invSlot);
    void handleUnequip(int playerId, uint8_t slotType);
};

#endif
