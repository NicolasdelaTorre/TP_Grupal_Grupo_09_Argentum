#ifndef SERVER_H
#define SERVER_H

#include <string>

#include "../common/queue.h"
#include "Comunication/acceptor.h"
#include "Comunication/client_monitor.h"
#include "Logic/gameloop.h"
#include "Logic/map.h"
#include "Logic/yaml_map_loader.h"
#include "Protocol/protocol_server.h"

class Server {
private:
    ProtocolServer protocol;
    Queue<std::string> clientCommands;
    ClientMonitor clientQueues;
    // Declarado antes que Gameloop porque éste lo referencia.
    Map map;
    Gameloop gameloop;
    Acceptor acceptor;

public:
    explicit Server(const char* port);

    /*
     * Starts the game loop and the acceptance of clients.
     */
    void startGame();
};

#endif
