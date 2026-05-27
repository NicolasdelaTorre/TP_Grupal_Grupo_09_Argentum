#include "server.h"

Server::Server(const char* port):
        protocol(port),
        clientCommands(),
        clientQueues(),
        // Mapa 10x10 hardcodeado; se reemplaza por carga YAML en el sprint siguiente
        map(10, 10),
        gameloop(clientCommands, clientQueues, map, protocol),
        acceptor(protocol, clientCommands, clientQueues) {
    protocol.serializeMap(map);
}

void Server::startGame() {
    gameloop.start();
    acceptor.start();

    // Waiting for the user to want to end the game by pressing the 'q' key
    while (std::getchar() != 'q') {}

    gameloop.stop();
    acceptor.stop();

    gameloop.join();
    acceptor.join();
}
