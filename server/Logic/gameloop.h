#ifndef GAMELOOP_H
#define GAMELOOP_H

#include <string>

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

public:
    Gameloop(Queue<std::string>& commands, ClientMonitor& clientQueues, Map& map,
             ProtocolServer& protocol);

    virtual void run() override;

    virtual void stop() override;
};

#endif
