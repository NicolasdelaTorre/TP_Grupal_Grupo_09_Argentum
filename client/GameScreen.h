#pragma once

#include <string>

#include <SDL2pp/SDL2pp.hh>

#include "../common/queue.h"
#include "map_renderer.h"

static constexpr float FEET_OFFSET = 1.0f;
static constexpr float HEAD_OFFSET = 0.5f;

class GameScreen {
public:
    GameScreen(SDL2pp::Renderer& renderer,
               const std::string& assetsPath,
               Queue<std::string>& events_queue);

    // Retorna false cuando el jugador quiere salir
    bool run();

private:
    SDL2pp::Renderer& renderer;
    TextureCache cache;
    MapRenderer mapRenderer;
    GameMap map;
    Player player;

    // Queue compartida con el sender: cada cruce de tile se pushea como
    // "TOP"/"BOTTOM"/"LEFT"/"RIGHT" para que el servidor reciba el movimiento.
    Queue<std::string>& events_queue;
    int lastTileX;
    int lastTileY;

    // ── Input ─────────────────────────────────────────────────
    bool handleEvents(float dt);

    // ── Update ────────────────────────────────────────────────
    void update(float dt);

    // ── Render ────────────────────────────────────────────────
    void render();

    // Detecta cuando el jugador cruza a un tile distinto y notifica al server
    void notifyTileChange();
};
