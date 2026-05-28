#include "client.h"

#include <iostream>

#include <SDL.h>
#include <SDL2pp/SDL2pp.hh>
#include <SDL2pp/SDLTTF.hh>
#include <SDL_image.h>

#include "GameScreen.h"
#include "login_screen.h"


client::client(const char* hostname, const char* port, bool fullscreen):
        protocol(Socket(hostname, port)),
        sender(protocol, events_queue),
        receiver(protocol, server_queue),
        fullscreen(fullscreen) {}


void client::run() {
    SDL2pp::SDL sdl(SDL_INIT_VIDEO);
    SDL2pp::SDLTTF ttf;
    SDL2pp::SDLImage img(IMG_INIT_PNG);

    SDL2pp::Window window("Argentum", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
                          SDL_WINDOW_SHOWN | (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
    SDL2pp::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);

    LoginScreen login(renderer, "AO_IMGS");
    LoginResult result = login.run();
    if (!result.confirmed)
        return;

    // Handshake sincrónico (login + mapa) antes de arrancar los hilos para evitar races.
    protocol.send_username(result.username);

    ServerMsg type = protocol.recv_msg_type();
    if (type == ServerMsg::LOGIN_FAIL) {
        std::cerr << "Login failed (server rejected)" << std::endl;
        return;
    }
    if (type != ServerMsg::LOGIN_OK) {
        std::cerr << "Unexpected response from server (expected LOGIN_OK)" << std::endl;
        return;
    }
    Position spawn = protocol.recv_login_ok_payload();
    std::cout << "Login OK — spawn at (" << spawn.x << ", " << spawn.y << ")" << std::endl;

    type = protocol.recv_msg_type();
    if (type != ServerMsg::MAP) {
        std::cerr << "Expected MAP after LOGIN_OK" << std::endl;
        return;
    }
    ReceivedMap mapData = protocol.recv_map();
    std::cout << "Map received (" << mapData.width << "x" << mapData.height << ")" << std::endl;

    // Arrancamos los hilos
    sender.start();
    receiver.start();

    GameScreen game(renderer, "AO_IMGS", events_queue, server_queue, mapData, spawn);
    game.run();

    // Cleanup: cerramos queues/socket para desbloquear los threads
    events_queue.close();
    protocol.close();
    sender.join();
    receiver.join();
}
