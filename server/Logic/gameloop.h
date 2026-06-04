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

public:
    Gameloop(Queue<std::string>& commands, ClientMonitor& clientQueues, Map& map,
             ProtocolServer& protocol, Position playerSpawn);

    virtual void run() override;

    virtual void stop() override;
};

#endif
