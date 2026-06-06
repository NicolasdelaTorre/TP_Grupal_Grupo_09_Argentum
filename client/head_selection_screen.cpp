#include "head_selection_screen.h"

#include <algorithm>

#include <SDL2/SDL.h>

HeadSelectionScreen::HeadSelectionScreen(SDL2pp::Renderer& renderer, const std::string& assetsPath):
        renderer(renderer),
        background(renderer, SDL2pp::Surface(assetsPath + "/Pantallas/Seleccion_personaje.png")),
        cache(renderer, assetsPath) {
    int winW, winH;
    SDL_GetRendererOutputSize(renderer.Get(), &winW, &winH);

    scaleX = static_cast<float>(winW) / 1024.0f;
    scaleY = static_cast<float>(winH) / 768.0f;
    displayW = winW;
    displayH = winH;
    bgX = bgY = 0;
}

HeadSelectionResult HeadSelectionScreen::run() {
    while (running) {
        handleEvents();
        render();
        SDL_Delay(16);
    }
    return {selectedHead, confirmed};
}

SDL2pp::Rect HeadSelectionScreen::pngToScreen(int x, int y, int w, int h) const {
    return SDL2pp::Rect(bgX + static_cast<int>(x * scaleX), bgY + static_cast<int>(y * scaleY),
                        static_cast<int>(w * scaleX), static_cast<int>(h * scaleY));
}

SDL2pp::Rect HeadSelectionScreen::getBoxRect(int i) const {
    int col = i % COLS;
    int x = BOX_LEFT[col];
    int w = BOX_RIGHT[col] - BOX_LEFT[col];
    int top = (i < COLS) ? BOX_TOP : BOX_TOP2;
    int h = (i < COLS) ? BOX_BOTTOM - BOX_TOP : BOX_BOTTOM2 - BOX_TOP2;
    return pngToScreen(x, top, w, h);
}

void HeadSelectionScreen::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                switch (e.key.keysym.sym) {
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        confirmed = true;
                        running = false;
                        break;
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                    case SDLK_LEFT:
                        selectedHead = (selectedHead - 1 + NUM_HEADS) % NUM_HEADS;
                        break;
                    case SDLK_RIGHT:
                        selectedHead = (selectedHead + 1) % NUM_HEADS;
                        break;
                    case SDLK_DOWN:
                        selectedHead = (selectedHead + COLS) % NUM_HEADS;
                        break;
                    case SDLK_UP:
                        selectedHead = (selectedHead - COLS + NUM_HEADS) % NUM_HEADS;
                        break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (e.button.button == SDL_BUTTON_LEFT)
                    handleMouseClick(e.button.x, e.button.y);
                break;
        }
    }
}

void HeadSelectionScreen::handleMouseClick(int mouseX, int mouseY) {
    SDL2pp::Rect jugar = pngToScreen(BTN_JUGAR_X, BTN_JUGAR_Y, BTN_JUGAR_W, BTN_JUGAR_H);
    if (mouseX >= jugar.x && mouseX < jugar.x + jugar.w && mouseY >= jugar.y &&
        mouseY < jugar.y + jugar.h) {
        confirmed = true;
        running = false;
        return;
    }

    for (int i = 0; i < NUM_HEADS; i++) {
        SDL2pp::Rect box = getBoxRect(i);
        if (mouseX >= box.x && mouseX < box.x + box.w && mouseY >= box.y &&
            mouseY < box.y + box.h) {
            if (selectedHead == i) {
                confirmed = true;
                running = false;
            } else {
                selectedHead = i;
            }
            return;
        }
    }
}

void HeadSelectionScreen::render() {
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();

    renderer.Copy(background, SDL2pp::NullOpt, SDL2pp::Rect(bgX, bgY, displayW, displayH));

    for (int i = 0; i < NUM_HEADS; i++) {
        SDL2pp::Rect box = getBoxRect(i);

        if (i == selectedHead) {
            renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
            renderer.SetDrawColor(HIGHLIGHT.r, HIGHLIGHT.g, HIGHLIGHT.b, HIGHLIGHT.a);
            renderer.FillRect(box);
            renderer.SetDrawBlendMode(SDL_BLENDMODE_NONE);
        }

        renderHeadInBox(i, box);
    }

    renderer.Present();
}

void HeadSelectionScreen::renderHeadInBox(int headIdx, const SDL2pp::Rect& boxRect) {
    int pad = static_cast<int>(BOX_PAD * std::min(scaleX, scaleY));
    int innerX = boxRect.x + pad;
    int innerY = boxRect.y + pad;
    int innerW = boxRect.w - pad * 2;
    int innerH = boxRect.h - pad * 2;

    float headScale = std::min(static_cast<float>(innerW) / HEAD_CELL_W,
                               static_cast<float>(innerH) / HEAD_CELL_H);
    int drawW = static_cast<int>(HEAD_CELL_W * headScale);
    int drawH = static_cast<int>(HEAD_CELL_H * headScale);

    int drawX = innerX + (innerW - drawW) / 2;
    int drawY = innerY + (innerH - drawH) / 2;

    // Row 0 = DOWN direction
    SDL2pp::Rect src(headIdx * HEAD_CELL_W, 0, HEAD_CELL_W, HEAD_CELL_H);
    SDL2pp::Rect dst(drawX, drawY, drawW, drawH);

    try {
        renderer.Copy(cache.get("/Skins/Cabezas.png"), src, dst);
    } catch (...) {}
}
