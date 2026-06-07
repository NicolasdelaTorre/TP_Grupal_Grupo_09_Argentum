#ifndef GAMELOOP_H
#define GAMELOOP_H

#include <string>

#include "../../common/position.h"
#include "../../common/queue.h"
#include "../../common/thread.h"
#include "../Comunication/client_monitor.h"
#include "../Protocol/protocol_server.h"

#include "game.h"

class Gameloop: public Thread {
private:
    Queue<std::string>& commands;
    ClientMonitor& clientQueues;
    bool gameFinished;
    Game game;
    ProtocolServer& protocol;

    void processCommand(const std::string& command);

    // Manda LOGIN_OK + MAP al jugador y avisa a todos del nuevo. Se llama
    // al final del char creation (cuando llega "skin").
    void finalizePlayerLogin(int idPlayer, const std::string& skinId);

    // Arma el string interno "STATS:hp:maxHp:mana:maxMana:gold:exp:nextLvlExp:level"
    // a partir del estado actual del jugador. Se reusa donde haga falta.
    std::string buildStatsMessage(int idPlayer);

    // Manda PLAYER_EQUIPPED por cada slot equipado del jugador.
    // recipientId == -1 → broadcast a todos menos a él. Sino, sólo a ese cliente.
    void sendEquipmentSnapshot(int idPlayer, int recipientId);

public:
    Gameloop(Queue<std::string>& commands, ClientMonitor& clientQueues, Map& world,
             ProtocolServer& protocol);

    virtual void run() override;

    virtual void stop() override;
};

#endif
