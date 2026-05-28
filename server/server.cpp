#include "server.h"

Server::Server(const char* port):
        protocol(port),
        clientCommands(),
        clientQueues(),
        loadedMap(loadMapFromYaml("server/assets/maps/mapa_inicial.yaml")),
        gameloop(clientCommands, clientQueues, loadedMap.map, protocol, loadedMap.playerSpawn),
        acceptor(protocol, clientCommands, clientQueues) {
    protocol.setMap(loadedMap.map);
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
