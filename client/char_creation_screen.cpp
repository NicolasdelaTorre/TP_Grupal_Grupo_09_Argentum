#include "char_creation_screen.h"

#include <algorithm>

#include <SDL2/SDL.h>

// Skins disponibles en AO_IMGS/Skins/ (excluye Cabezas.png y Gorros.png)
const char* const CharCreationScreen::SKIN_FILES[NUM_SKINS] = {
        "skin_default.png", "Caballero_blanco.png", "Gladiador_azul.png",   "Hechicero.png",
        "Hechicera.png",    "skin_default.png",     "Caballero_blanco.png", "Gladiador_azul.png",
        "Hechicero.png",    "Hechicera.png",
};

CharCreationScreen::CharCreationScreen(SDL2pp::Renderer& renderer, const std::string& assetsPath):
        renderer(renderer),
        background(renderer, SDL2pp::Surface(assetsPath + "/Pantallas/Seleccion_personaje.png")),
        cache(renderer, assetsPath) {
    int winW, winH;
    SDL_GetRendererOutputSize(renderer.Get(), &winW, &winH);

    scaleX = static_cast<float>(winW) / 1024.0f;
    scaleY = static_cast<float>(winH) / 768.0f;
    displayW = winW;
    displayH = winH;
    bgX = 0;
    bgY = 0;
}

CharCreationResult CharCreationScreen::run() {
    while (running) {
        handleEvents();
        render();
        SDL_Delay(16);
    }
    return {selectedSkin, confirmed};
}

// ── Helpers ───────────────────────────────────────────────────────────────

SDL2pp::Rect CharCreationScreen::pngToScreen(int x, int y, int w, int h) const {
    return SDL2pp::Rect(bgX + static_cast<int>(x * scaleX), bgY + static_cast<int>(y * scaleY),
                        static_cast<int>(w * scaleX), static_cast<int>(h * scaleY));
}

SDL2pp::Rect CharCreationScreen::getBoxRect(int i) const {
    int col = i % COLS;
    int x = BOX_LEFT[col];
    int w = BOX_RIGHT[col] - BOX_LEFT[col];
    int top = (i < COLS) ? BOX_TOP : BOX_TOP2;
    int h = (i < COLS) ? BOX_BOTTOM - BOX_TOP : BOX_BOTTOM2 - BOX_TOP2;
    return pngToScreen(x, top, w, h);
}

// ── Eventos ───────────────────────────────────────────────────────────────

void CharCreationScreen::handleEvents() {
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
                        selectedSkin = (selectedSkin - 1 + NUM_SKINS) % NUM_SKINS;
                        break;
                    case SDLK_RIGHT:
                        selectedSkin = (selectedSkin + 1) % NUM_SKINS;
                        break;
                    case SDLK_DOWN:
                        selectedSkin = (selectedSkin + COLS) % NUM_SKINS;
                        break;
                    case SDLK_UP:
                        selectedSkin = (selectedSkin - COLS + NUM_SKINS) % NUM_SKINS;
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

void CharCreationScreen::handleMouseClick(int mouseX, int mouseY) {
    // Botón JUGAR
    SDL2pp::Rect jugar = pngToScreen(BTN_JUGAR_X, BTN_JUGAR_Y, BTN_JUGAR_W, BTN_JUGAR_H);
    if (mouseX >= jugar.x && mouseX < jugar.x + jugar.w && mouseY >= jugar.y &&
        mouseY < jugar.y + jugar.h) {
        confirmed = true;
        running = false;
        return;
    }

    for (int i = 0; i < BOX_COUNT; i++) {
        SDL2pp::Rect box = getBoxRect(i);
        if (mouseX >= box.x && mouseX < box.x + box.w && mouseY >= box.y &&
            mouseY < box.y + box.h) {
            if (selectedSkin == i) {
                confirmed = true;
                running = false;
            } else {
                selectedSkin = i;
            }
            return;
        }
    }
}

// ── Render ────────────────────────────────────────────────────────────────

void CharCreationScreen::render() {
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();

    renderer.Copy(background, SDL2pp::NullOpt, SDL2pp::Rect(bgX, bgY, displayW, displayH));

    for (int i = 0; i < BOX_COUNT; i++) {
        SDL2pp::Rect box = getBoxRect(i);

        // Resaltar el recuadro seleccionado con un overlay semitransparente.
        if (i == selectedSkin) {
            renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
            renderer.SetDrawColor(HIGHLIGHT.r, HIGHLIGHT.g, HIGHLIGHT.b, HIGHLIGHT.a);
            renderer.FillRect(box);
            renderer.SetDrawBlendMode(SDL_BLENDMODE_NONE);
        }

        renderSkinInBox(i, box);
    }

    renderer.Present();
}

void CharCreationScreen::renderSkinInBox(int skinIdx, const SDL2pp::Rect& boxRect) {
    // Zona interior del recuadro (excluye el borde)
    int pad = static_cast<int>(BOX_PAD * std::min(scaleX, scaleY));
    int innerX = boxRect.x + pad;
    int innerY = boxRect.y + pad;
    int innerW = boxRect.w - pad * 2;
    int innerH = boxRect.h - pad * 2;

    // Escala dinámica: el sprite ocupa el máximo espacio posible dentro del recuadro
    float totalSrcH = (HEAD_CELL_H / 4.0f + 3.0f) + SPRITE_H;
    float spriteScale =
            std::min(static_cast<float>(innerW) / SPRITE_W, static_cast<float>(innerH) / totalSrcH);
    int bodyW = static_cast<int>(SPRITE_W * spriteScale);
    int bodyH = static_cast<int>(SPRITE_H * spriteScale);
    int headW = static_cast<int>(HEAD_CELL_W * spriteScale);
    int headH = static_cast<int>(HEAD_CELL_H * spriteScale);

    // Desplazamiento de la cabeza respecto al cuerpo (igual que en map_renderer)
    int headAboveBody = static_cast<int>((HEAD_CELL_H / 4 + 3) * spriteScale);

    // Altura visual total: desde el top de la cabeza hasta el bottom del cuerpo
    int totalH = headAboveBody + bodyH;

    // Centrar el bloque cabeza+cuerpo dentro del interior del recuadro
    int startX = innerX + (innerW - bodyW) / 2;
    int startY = innerY + (innerH - totalH) / 2;

    int headY = startY;
    int bodyY = startY + headAboveBody;
    int headX = startX + (bodyW - headW) / 2;  // ancho igual, queda centrado

    // Cuerpo: dirección DOWN (row 0), frame 0 (col 0)
    SDL2pp::Rect bodySrc(0, 0, SPRITE_W, SPRITE_H);
    SDL2pp::Rect bodyDst(startX, bodyY, bodyW, bodyH);

    // Cabeza: dirección DOWN (row 0), headId DEFAULT_HEAD (col)
    SDL2pp::Rect headSrc(DEFAULT_HEAD * HEAD_CELL_W, 0, HEAD_CELL_W, HEAD_CELL_H);
    SDL2pp::Rect headDst(headX, headY, headW, headH);

    try {
        auto& bodyTex = cache.get(std::string("/Skins/") + SKIN_FILES[skinIdx]);
        auto& headTex = cache.get("/Skins/Cabezas.png");
        renderer.Copy(bodyTex, bodySrc, bodyDst);
        renderer.Copy(headTex, headSrc, headDst);
    } catch (...) {
        // Si la textura no cargó, simplemente no renderizamos ese slot.
    }
}
