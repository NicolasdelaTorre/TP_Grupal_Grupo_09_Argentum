#include "client.h"

#include <iostream>

#include <SDL.h>
#include <SDL2pp/SDL2pp.hh>
#include <SDL2pp/SDLTTF.hh>
#include <SDL_image.h>

#include "../common/Communication/events/client_events.h"
#include "../common/Communication/events/server_events.h"
#include "../common/Communication/message_types.h"

#include "GameScreen.h"
#include "char_creation_screen.h"
#include "head_selection_screen.h"
#include "login_screen.h"


Client::Client(const char* hostname, const char* port, bool fullscreen):
        protocol(hostname, port),
        sender(protocol, clientEvents),
        receiver(protocol, serverEvents),
        fullscreen(fullscreen) {}


void Client::run() {
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

    // Handshake sincrónico (login + mapa) antes de arrancar los hilos.
    // Raza/clase hardcodeadas hasta que haya UI para elegirlas.
    std::string name(result.username.begin(), result.username.end());
    protocol.send(UserArrivalEvent(name, RaceCode::ELF, ClassCode::MAGE));

    auto ev = protocol.receiveEvent();
    Player_ player;

    // Usuario nuevo: char creation + skin/head al server.
    if (auto* op = dynamic_cast<OpcodeOnlyEvent*>(ev.get());
        op && op->getOpcode() == static_cast<uint8_t>(ServerMsg::LOGIN_FAIL)) {
        std::cerr << "Login failed (server rejected)" << std::endl;
        return;
    }

    if (auto* op = dynamic_cast<OpcodeOnlyEvent*>(ev.get());
        op && op->getOpcode() == static_cast<uint8_t>(ServerMsg::FIRST_LOGIN)) {
        HeadSelectionScreen headSelection(renderer, "AO_IMGS");
        HeadSelectionResult headResult = headSelection.run();
        if (!headResult.confirmed)
            return;
        CharCreationScreen charCreation(renderer, "AO_IMGS", headResult.headId);
        CharCreationResult charResult = charCreation.run();
        if (!charResult.confirmed)
            return;
        protocol.send(SkinSelectedEvent(static_cast<uint8_t>(charResult.skinId)));
        player.skin = charResult.skinId;
        ev = protocol.receiveEvent();
    }

    auto* loginOk = dynamic_cast<LoginOkEvent*>(ev.get());
    if (!loginOk) {
        std::cerr << "Unexpected response from server (expected LOGIN_OK)" << std::endl;
        return;
    }
    Position spawn{loginOk->getSpawnX(), loginOk->getSpawnY()};
    std::cout << "Login OK — spawn at (" << spawn.x << ", " << spawn.y << ")" << std::endl;

    auto mapEv = protocol.receiveEvent();
    auto* mapData = dynamic_cast<MapEvent*>(mapEv.get());
    if (!mapData) {
        std::cerr << "Expected MAP after LOGIN_OK" << std::endl;
        return;
    }
    std::cout << "Map received (" << mapData->getWidth() << "x" << mapData->getHeight() << ")"
              << std::endl;

    // Arrancamos los hilos
    sender.start();
    receiver.start();

    GameScreen game(renderer, "AO_IMGS", clientEvents, serverEvents, *mapData, spawn, player);
    game.run();

    // Cleanup: cerramos queues/socket para desbloquear los threads.
    clientEvents.close();
    protocol.shutdown();
    sender.join();
    receiver.join();
}
