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

    // Pantalla de login: el usuario ingresa su nombre
    LoginScreen login(renderer, "AO_IMGS");
    LoginResult result = login.run();
    if (!result.confirmed)
        return;

    // Enviamos el username al servidor y arrancamos los threads de red
    protocol.send_username(result.username);
    sender.start();
    receiver.start();

    // Esperamos la respuesta del servidor: primero LOGIN_OK / LOGIN_FAIL
    std::string loginResponse = server_queue.pop();
    if (loginResponse != "LOGIN_OK") {
        std::cerr << "Login failed: " << loginResponse << std::endl;
        events_queue.close();
        protocol.close();
        sender.join();
        receiver.join();
        return;
    }

    // Después del LOGIN_OK el servidor manda el mapa. Por ahora lo descartamos;
    // el cliente sigue usando un mapa hardcodeado en makeTestMap().
    server_queue.pop();

    // Login aceptado — corremos el juego pasándole la queue para que las teclas
    // de movimiento crucen al sender → servidor.
    GameScreen game(renderer, "AO_IMGS", events_queue);
    game.run();

    // Cleanup ordenado: cerramos queues/socket para desbloquear los threads
    events_queue.close();
    protocol.close();
    sender.join();
    receiver.join();
}
