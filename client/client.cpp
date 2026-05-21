//
// Created by nicolas on 16/5/26.
//

#include "client.h"

#include <SDL.h>
#include <SDL2pp/SDL2pp.hh>
#include <SDL2pp/SDLTTF.hh>

#include "login_screen.h"


client::client(const char* hostname, const char* port):
        protocol(Socket(hostname, port)),
        sender(protocol, events_queue),
        receiver(protocol, server_queue) {}

void client::run() {
    SDL2pp::SDL sdl(SDL_INIT_VIDEO);
    SDL2pp::SDLTTF ttf;

    SDL2pp::Window window("Argentum Online", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640,
                          400, SDL_WINDOW_SHOWN);

    SDL2pp::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);

    // ── Login ─────────────────────────────────────────────
    LoginScreen login(renderer);
    LoginResult result = login.run();

    if (!result.confirmed) {
        return;  // el usuario cerró la ventana
    }

    std::cout << "Bienvenido: " << result.username << std::endl;

    protocol.send(result.username);

    sender.start();
    receiver.start();

    // Esperar LOGIN_OK del servidor
    std::string respuesta = server_queue.pop();
    std::cout << "Servidor respondio: " << respuesta << std::endl;

    events_queue.close();  // desbloquea al sender que está esperando en pop()
    protocol.close();      // desbloquea al receiver que está esperando en recv()
    sender.join();
    receiver.join();

    // ── Acá arranca el juego ───────────────────────────────
    // GameClient game(result.username);
    // game.run();
}
