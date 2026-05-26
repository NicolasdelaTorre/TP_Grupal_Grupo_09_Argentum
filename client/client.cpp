#include "login_screen.h"
#include "GameScreen.h"
#include <SDL2/SDL_image.h>
#include "client.h"

#include <SDL.h>
#include <SDL2pp/SDL2pp.hh>
#include <SDL2pp/SDLTTF.hh>




client::client(const char* hostname, const char* port, bool fullscreen):
        protocol(Socket(hostname, port)),
        sender(protocol, events_queue),
        receiver(protocol, server_queue),
        fullscreen(fullscreen) {}



void client::run() {


    SDL2pp::SDL    sdl(SDL_INIT_VIDEO);
    SDL2pp::SDLTTF ttf;
    SDL2pp::SDLImage img(IMG_INIT_PNG);

    SDL2pp::Window window("Argentum", SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN | (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
    SDL2pp::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Login
    LoginScreen login(renderer, "AO_IMGS");
    LoginResult result = login.run();
    if (!result.confirmed) return;

    
    GameScreen game(renderer, "AO_IMGS");
    game.run();

    return;
}