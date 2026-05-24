#include "server.h"

int main(int argc, char* argv[]) {

    if (argc != 2)
        return 1;

    Server server(argv[1]);
    server.startGame();

    return 0;
}
