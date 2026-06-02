#pragma once

#include <string>

#include <SDL2pp/SDL2pp.hh>

#include "texture_cache.h"

struct CharCreationResult {
    int skinId;
    bool confirmed;
};

class CharCreationScreen {
public:
    CharCreationScreen(SDL2pp::Renderer& renderer, const std::string& assetsPath);
    CharCreationResult run();

private:
    SDL2pp::Renderer& renderer;
    SDL2pp::Texture background;
    TextureCache cache;

    int selectedSkin = 0;
    bool running = true;
    bool confirmed = false;

    float scaleX = 1.0f, scaleY = 1.0f;
    int bgX = 0, bgY = 0;
    int displayW = 0, displayH = 0;

    // ── Skins disponibles (archivos dentro de AO_IMGS/Skins/) ────────────
    static constexpr int NUM_SKINS = 10;
    static const char* const SKIN_FILES[NUM_SKINS];

    // ── Posiciones de los recuadros en el PNG 1024×1024 ──────────────────
    // Cada par (left, right) son los bordes exteriores detectados por pixel.
    // El contenido interior empieza 4px adentro de cada borde.
    static constexpr int BOX_COUNT = 10;
    static constexpr int COLS = 5;
    static constexpr int BOX_LEFT[5] = {215, 347, 479, 611, 743};
    static constexpr int BOX_RIGHT[5] = {280, 412, 544, 676, 808};
    static constexpr int BOX_TOP = 250;
    static constexpr int BOX_BOTTOM = 330;
    static constexpr int BOX_TOP2 = 410;
    static constexpr int BOX_BOTTOM2 = 490;

    // Padding interior (excluye el borde de 4px)
    static constexpr int BOX_PAD = 4;

    // ── Constantes del sprite ─────────────────────────────────────────────
    static constexpr int SPRITE_W = 27;
    static constexpr int SPRITE_H = 49;
    static constexpr int HEAD_CELL_W = 27;
    static constexpr int HEAD_CELL_H = 64;
    static constexpr int DEFAULT_HEAD = 4;  // columna en Cabezas.png
    static constexpr float SPRITE_SCL = 0.8f;

    // ── Color para el recuadro seleccionado ──────────────────────────────
    static constexpr SDL_Color HIGHLIGHT = {200, 170, 50, 160};

    // Botón JUGAR (coordenadas en el PNG 1024×1024)
    static constexpr int BTN_JUGAR_X = 565;
    static constexpr int BTN_JUGAR_Y = 708;
    static constexpr int BTN_JUGAR_W = 315;
    static constexpr int BTN_JUGAR_H = 44;

    // Convierte coordenadas del PNG a coordenadas de pantalla.
    SDL2pp::Rect pngToScreen(int x, int y, int w, int h) const;

    // Devuelve el rect de pantalla del recuadro i (zona exterior, usada para clicks).
    SDL2pp::Rect getBoxRect(int i) const;

    void handleEvents();
    void handleMouseClick(int mouseX, int mouseY);
    void render();
    void renderSkinInBox(int skinIdx, const SDL2pp::Rect& boxRect);
};
