#ifndef GAMELOOP_H
#define GAMELOOP_H

#include <memory>
#include <string>
#include <unordered_map>

#include "../../common/Communication/events/server_event.h"
#include "../../common/DTOs.h"
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

    // NPC amigo actualmente seleccionado por cada jugador (click). Los comandos
    // dirigidos a NPCs (/comprar, /vender, /depositar, etc.) actúan sobre éste.
    // Si no hay entrada, el jugador no tiene selección activa.
    std::unordered_map<int, uint16_t> selectedNpc;

    // Jugadores que mandaron UserArrival pero todavía no existen en el binario.
    // El handshake espera un CharacterCreated para terminar de crearlos.
    std::unordered_map<int, std::string> pendingNewPlayers;

    // Manda LOGIN_OK + map + stats + inv + snapshot de otros players y NPCs.
    // Lo usan ambos paths: jugador existente al loguearse y jugador nuevo
    // recien creado.
    void sendPostLoginSnapshots(int playerId);

    // Construye un StatsEvent con el snapshot actual del jugador.
    std::shared_ptr<ServerEvent> buildStatsEvent(int idPlayer);

    // Manda PLAYER_EQUIPPED por cada slot equipado del jugador.
    // recipientId == -1 → broadcast a todos menos a él. Sino, sólo a ese cliente.
    void sendEquipmentSnapshot(int idPlayer, int recipientId);

    // Manda el snapshot completo del mapa actual (overworld o environment).
    void sendMapSnapshot(int playerId, uint8_t mapId);

    // Manda los NewNpcEvent de los NPCs que viven en ese mapId. En el overworld
    // (mapId == 0) incluye además los NPCs amigos (merchant/banker/priest).
    void sendNpcSnapshot(int playerId, uint8_t mapId);

    // Avanza el turno de cada NPC vivo; si alguno ataca, dispara los eventos al cliente.
    void NPCTurns();

    void PlayerTurns();

    void broadcastToMap(uint8_t mapId, std::shared_ptr<ServerEvent> event, int excludedId);

    void broadcastDrops(const std::vector<Game::DroppedItemRecord>& drops, uint8_t mapId);

    void broadcastInventoryChanges(int playerId, const Game::InventorySnapshot& before,
                                      const Game::InventorySnapshot& after);

public:
    Gameloop(IncomingQueue& clientEvents, ClientMonitor& clientMonitor, Map& map,
             ServerProtocol& protocol);

    virtual void run() override;

    virtual void stop() override;

    // Despacha el ClientEvent al handle correspondiente (un único switch
    // por tipo, con dynamic_cast). Cada caso vive en su propio handler.
    void dispatch(const ClientEvent& ev);

    void handleDisconnect(int playerId);
    // Handshake fase 1: recibe el nombre. Si existe, entra directo al juego.
    // Si no, deja al jugador en pendingNewPlayers y manda FIRST_LOGIN.
    void handleUserArrival(int playerId, const std::string& name);
    // Handshake fase 2 (solo para nuevos): crea el player con raza/clase/head/skin
    // recibidos en el evento, y manda LOGIN_OK + map + snapshots.
    void handleCharacterCreated(int playerId, RaceCode race, ClassCode class_, uint8_t headId,
                                uint8_t skinId);
    void handleMovement(int playerId, MoveDirection direction);
    void handleTurn(int playerId, MoveDirection direction);
    void handleAttack(int playerId, uint8_t targetType, uint16_t targetId);
    void handlePickUp(int playerId);
    void handleDrop(int playerId, uint8_t invSlot);
    void handleEquip(int playerId, uint8_t invSlot);
    void handleUnequip(int playerId, uint8_t slotType);
    // Si text empieza con '/', va al parser de comandos. Sino se broadcastea
    // tal cual con [authorId][authorName][text].
    void handleChat(int playerId, const std::string& text);
    // Parsea "/cmd arg1 arg2 ..." y dispara la acción. Si el comando no
    // existe, le manda al jugador un ChatBroadcastEvent del sistema.
    void handleChatCommand(int playerId, const std::string& text);
    // Parsea "@nick mensaje" y se lo manda solo al destinatario (mas copia al
    // emisor). Si el nick no esta conectado, avisa al emisor.
    void handlePrivateMessage(int playerId, const std::string& text);
    // Click sobre un NPC amigo: valida adyacencia (≤2 tiles) y guarda la
    // selección. Responde con mensaje del sistema.
    void handleSelectNpc(int playerId, uint16_t npcId);
};

#endif
