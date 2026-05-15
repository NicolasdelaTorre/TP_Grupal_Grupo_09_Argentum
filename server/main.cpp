#include "servidor.h"

int main(int argc, char* argv[]) {

    if (argc != 2) return 1;

    Servidor servidor(argv[1]);
    servidor.empezarJuego();

    return 0;
}
