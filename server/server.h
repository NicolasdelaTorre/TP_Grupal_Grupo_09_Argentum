#ifndef SERVER_H
#define SERVER_H

#include "../common/queue.h"
#include "Communication/acceptor.h"
#include "Communication/client_monitor.h"
#include "Communication/server_receiver.h"  // IncomingQueue alias
#include "Logic/gameloop.h"
#include "Logic/map.h"
#include "Logic/yaml_map_loader.h"
#include "Communication/server_protocol.h"

class Server {
private:
    ServerProtocol protocol;
    IncomingQueue clientEvents;
    ClientMonitor clientMonitor;
    LoadedMap loadedMap;
    Gameloop gameloop;
    Acceptor acceptor;

public:
    explicit Server(const char* port);
    void startGame();
};

#endif
