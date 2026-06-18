#include <iostream>

#include <unistd.h>

#include "server.h"

#ifndef PROJECT_ROOT
// Respaldo por si se compila sin la definición de CMake (no debería pasar).
#define PROJECT_ROOT "."
#endif

// Posiciona el proceso en la raíz del proyecto (horneada por CMake en
// PROJECT_ROOT) para que los paths a los .toml, mapas y .bin funcionen sin
// importar desde dónde se lance el binario.
static bool chdirToRoot() { return chdir(PROJECT_ROOT) == 0; }

int main(int argc, char* argv[]) {

    if (argc != 2)
        return 1;

    if (!chdirToRoot()) {
        std::cerr << "WARN: no pude posicionarme en la raíz del proyecto (" << PROJECT_ROOT
                  << "); el server podría no cargar sus datos." << std::endl;
    }

    Server server(argv[1]);
    server.startGame();

    return 0;
}
