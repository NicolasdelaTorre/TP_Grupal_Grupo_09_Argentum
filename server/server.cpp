#include "server.h"

Server::Server(const char* port):
        protocol(port),
        clientEvents(),
        clientMonitor(),
        loadedMap(loadMapFromYaml("server/assets/maps/mapa_completo.yaml")),
        gameloop(clientEvents, clientMonitor, loadedMap.map, protocol, loadedMap.playerSpawn),
        acceptor(protocol, clientEvents, clientMonitor) {}

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
