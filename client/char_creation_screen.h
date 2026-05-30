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

    float scale;
    int bgX, bgY;
    int displayW, displayH;  // tamaño real del PNG en pantalla (aspect ratio preservado)

    // ── Skins disponibles (archivos dentro de AO_IMGS/Skins/) ────────────
    static constexpr int NUM_SKINS = 5;
    static const char* const SKIN_FILES[NUM_SKINS];

    // ── Posiciones de los recuadros en el PNG 1024×1024 ──────────────────
    // Cada par (left, right) son los bordes exteriores detectados por pixel.
    // El contenido interior empieza 4px adentro de cada borde.
    static constexpr int BOX_COUNT = 5;
    static constexpr int BOX_LEFT[5] = {215, 347, 479, 611, 743};
    static constexpr int BOX_RIGHT[5] = {284, 416, 548, 680, 812};
    static constexpr int BOX_TOP = 185;
    static constexpr int BOX_BOTTOM = 245;

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

    // Convierte coordenadas del PNG a coordenadas de pantalla.
    SDL2pp::Rect pngToScreen(int x, int y, int w, int h) const;

    // Devuelve el rect de pantalla del recuadro i (zona exterior, usada para clicks).
    SDL2pp::Rect getBoxRect(int i) const;

    void handleEvents();
    void handleMouseClick(int mouseX, int mouseY);
    void render();
    void renderSkinInBox(int skinIdx, const SDL2pp::Rect& boxRect);
};
