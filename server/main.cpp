#include <iostream>

#include "../common/project_root.h"

#include "server.h"

int main(int argc, char* argv[]) {

    if (argc != 2)
        return 1;

    // Posicionarse en la raíz del proyecto para que los paths a los .toml, mapas
    // y .bin funcionen sin importar desde dónde se lance el binario.
    if (!project_root::chdirToRoot()) {
        std::cerr << "WARN: no encontré config.toml; el server podría no cargar sus datos."
                  << std::endl;
    }

    Server server(argv[1]);
    server.startGame();

    return 0;
}
