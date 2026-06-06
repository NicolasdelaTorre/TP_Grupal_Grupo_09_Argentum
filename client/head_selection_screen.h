#pragma once

#include <string>

#include <SDL2pp/SDL2pp.hh>

#include "texture_cache.h"

struct HeadSelectionResult {
    int headId;
    bool confirmed;
};

class HeadSelectionScreen {
public:
    HeadSelectionScreen(SDL2pp::Renderer& renderer, const std::string& assetsPath);
    HeadSelectionResult run();

private:
    SDL2pp::Renderer& renderer;
    SDL2pp::Texture background;
    TextureCache cache;

    int selectedHead = 0;
    bool running = true;
    bool confirmed = false;

    float scaleX = 1.0f, scaleY = 1.0f;
    int bgX = 0, bgY = 0;
    int displayW = 0, displayH = 0;

    static constexpr int NUM_HEADS = 10;
    static constexpr int COLS = 5;
    static constexpr int BOX_LEFT[5] = {215, 347, 479, 611, 743};
    static constexpr int BOX_RIGHT[5] = {280, 412, 544, 676, 808};
    static constexpr int BOX_TOP = 250;
    static constexpr int BOX_BOTTOM = 330;
    static constexpr int BOX_TOP2 = 410;
    static constexpr int BOX_BOTTOM2 = 490;
    static constexpr int BOX_PAD = 4;

    static constexpr int HEAD_CELL_W = 27;
    static constexpr int HEAD_CELL_H = 64;

    static constexpr SDL_Color HIGHLIGHT = {200, 170, 50, 160};

    static constexpr int BTN_JUGAR_X = 565;
    static constexpr int BTN_JUGAR_Y = 708;
    static constexpr int BTN_JUGAR_W = 315;
    static constexpr int BTN_JUGAR_H = 44;

    SDL2pp::Rect pngToScreen(int x, int y, int w, int h) const;
    SDL2pp::Rect getBoxRect(int i) const;

    void handleEvents();
    void handleMouseClick(int mouseX, int mouseY);
    void render();
    void renderHeadInBox(int headIdx, const SDL2pp::Rect& boxRect);
};
