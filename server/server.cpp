#include "server.h"

Server::Server(const char* port):
        protocol(port),
        clientCommands(),
        clientQueues(),
        gameloop(clientCommands, clientQueues),
        acceptor(protocol, clientCommands, clientQueues) {}

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
