#include "client.h"

#include <iostream>
#include <optional>

#include <SDL.h>
#include <SDL2pp/Mixer.hh>
#include <SDL2pp/Music.hh>
#include <SDL2pp/SDL2pp.hh>
#include <SDL2pp/SDLTTF.hh>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include "../common/Communication/events/client_events.h"
#include "../common/Communication/events/server_events.h"
#include "../common/Communication/message_types.h"

#include "character_creation_screen.h"
#include "GameScreen.h"
#include "head_selection_screen.h"
#include "login_screen.h"
#include "sound_manager.h"


Client::Client(const char* hostname, const char* port, bool fullscreen):
        protocol(hostname, port),
        sender(protocol, clientEvents),
        receiver(protocol, serverEvents),
        fullscreen(fullscreen) {}


void Client::run() {
    SDL2pp::SDL sdl(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL2pp::SDLTTF ttf;
    SDL2pp::SDLImage img(IMG_INIT_PNG);

    // Audio: abrimos el dispositivo y dejamos sonando la música de fondo en loop
    // durante toda la sesión (login + juego). mixer/music viven a nivel de run()
    // (no static) para que se destruyan ANTES que `sdl`: el ~Mixer hace
    // Mix_CloseAudio y necesita el subsistema de audio todavía vivo. Si el audio
    // falla, seguimos sin sonido en vez de abortar el cliente.
    SDL2pp::Mixer mixer(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT,
                        MIX_DEFAULT_CHANNELS, 4096);
    SoundManager sound(mixer);

    SDL2pp::Window window("Argentum", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 900, 600,
                          SDL_WINDOW_SHOWN | (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
    SDL2pp::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);

    LoginScreen login(renderer, "AO_IMGS");
    LoginResult result = login.run();
    if (!result.confirmed)
        return;
    std::string name(result.username.begin(), result.username.end());

    // Handshake fase 1: mandamos solo el nombre. El server decide si entramos
    // directo (jugador ya existe) o tenemos que pasar por creacion.
    protocol.send(UserArrivalEvent(name));

    auto ev = protocol.receiveEvent();
    Player_ player;

    if (auto* op = dynamic_cast<OpcodeOnlyEvent*>(ev.get());
        op && op->getOpcode() == static_cast<uint8_t>(ServerMsg::LOGIN_FAIL)) {
        std::cerr << "Login failed (server rejected)" << std::endl;
        return;
    }

    if (auto* op = dynamic_cast<OpcodeOnlyEvent*>(ev.get());
        op && op->getOpcode() == static_cast<uint8_t>(ServerMsg::FIRST_LOGIN)) {
        // Jugador nuevo: pasamos por creacion + head, mandamos todo junto.
        CharacterCreationScreen charCreation(renderer, "AO_IMGS");
        CharacterCreationResult cc = charCreation.run();
        if (!cc.confirmed)
            return;
        HeadSelectionScreen headSelection(renderer, "AO_IMGS");
        HeadSelectionResult headResult = headSelection.run();
        if (!headResult.confirmed)
            return;
        protocol.send(CharacterCreatedEvent(cc.race, cc.class_,
                                            static_cast<uint8_t>(headResult.headId),
                                            static_cast<uint8_t>(SKIN_DEFAULT)));
        player.skin = SKIN_DEFAULT;
        player.headId = headResult.headId;
        ev = protocol.receiveEvent();
    }

    auto* loginOk = dynamic_cast<LoginOkEvent*>(ev.get());
    if (!loginOk) {
        std::cerr << "Unexpected response from server (expected LOGIN_OK)" << std::endl;
        return;
    }
    Position spawn{loginOk->getSpawnX(), loginOk->getSpawnY()};
    // El server manda nuestro skin+head persistidos. Para player nuevo coinciden
    // con lo que mandamos en CharacterCreated; para player existente vienen del binario.
    player.skin = loginOk->getSkin();
    player.headId = static_cast<int>(loginOk->getHead());
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

    GameScreen game(renderer, "AO_IMGS", clientEvents, serverEvents, *mapData, spawn, player, sound);
    game.run();

    // Cleanup: cerramos queues/socket para desbloquear los threads.
    clientEvents.close();
    protocol.shutdown();
    sender.join();
    receiver.join();
}
