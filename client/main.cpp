#include <exception>
#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "client.h"

// Esta comentado para que no se queje el pre commit
// using namespace SDL2pp;
using SDL2pp::Renderer;
using SDL2pp::SDL;
using SDL2pp::Window;

int main(int argc, char* argv[]) {

    try {

        if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " <hostname or IP> <servicename or port>"
                      << std::endl;
            return 1;
        }

        const char* hostname = argv[1];
        const char* servicename = argv[2];
        bool fullscreen = false;
        if (argv[3] && std::string(argv[3]) == "--fullscreen") {
            fullscreen = true; 
        }



        client cli = client(hostname, servicename, fullscreen);
        cli.run();

        return 0;

    } catch (std::exception& e) {
        // If case of error, print it and exit with error
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
