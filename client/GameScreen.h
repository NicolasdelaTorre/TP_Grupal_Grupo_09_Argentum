#pragma once

#include <string>

#include <SDL2pp/SDL2pp.hh>

#include "map_renderer.h"

static constexpr float FEET_OFFSET = 1.0f;
static constexpr float HEAD_OFFSET = 0.5f;

class GameScreen {
public:
    GameScreen(SDL2pp::Renderer& renderer, const std::string& assetsPath);

    // Retorna false cuando el jugador quiere salir
    bool run();

private:
    SDL2pp::Renderer& renderer;
    TextureCache cache;
    MapRenderer mapRenderer;
    GameMap map;
    Player player;

    // ── Input ─────────────────────────────────────────────────
    bool handleEvents(float dt);

    // ── Update ────────────────────────────────────────────────
    void update(float dt);

    // ── Render ────────────────────────────────────────────────
    void render();
};
