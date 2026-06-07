#include "server.h"

Server::Server(const char* port):
        protocol(port),
        clientCommands(),
        clientQueues(),
        map(loadMapFromYaml("server/assets/maps/otro_mapa.yaml")),
        gameloop(clientCommands, clientQueues, map, protocol),
        acceptor(protocol, clientCommands, clientQueues) {
    protocol.setMap(map);
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
