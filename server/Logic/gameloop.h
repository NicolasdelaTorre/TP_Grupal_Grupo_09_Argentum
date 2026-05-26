#ifndef GAMELOOP_H
#define GAMELOOP_H

#include <string>

#include "../../common/queue.h"
#include "../../common/thread.h"
#include "../Comunication/client_monitor.h"

#include "game.h"

class Gameloop: public Thread {
private:
    Queue<std::string>& commands;
    ClientMonitor& clientQueues;
    bool gameFinished;
    Game game;

    void processCommand(const std::string& command);

public:
    Gameloop(Queue<std::string>& commands, ClientMonitor& clientQueues);

    virtual void run() override;

    virtual void stop() override;
};

#endif
