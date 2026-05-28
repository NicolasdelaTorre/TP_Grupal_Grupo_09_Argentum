#include "char_creation_screen.h"

#include <algorithm>

#include <SDL2/SDL.h>

// Skins disponibles en AO_IMGS/Skins/ (excluye Cabezas.png y Gorros.png)
const char* const CharCreationScreen::SKIN_FILES[NUM_SKINS] = {
    "skin_default.png",
    "Caballero_blanco.png",
    "Gladiador_azul.png",
    "Hechicero.png",
    "Hechicera.png",
};

CharCreationScreen::CharCreationScreen(SDL2pp::Renderer& renderer, const std::string& assetsPath):
        renderer(renderer),
        background(renderer, SDL2pp::Surface(assetsPath + "/Pantallas/Seleccion_personaje.png")),
        cache(renderer, assetsPath) {
    int winW, winH;
    SDL_GetRendererOutputSize(renderer.Get(), &winW, &winH);

    // Escala uniforme: el PNG 1024×1024 cabe dentro de la ventana sin distorsión,
    // dejando barras negras en los bordes si la relación de aspecto es distinta.
    scale    = std::min(static_cast<float>(winW), static_cast<float>(winH)) / 1024.0f;
    displayW = static_cast<int>(1024 * scale);
    displayH = static_cast<int>(1024 * scale);
    bgX      = (winW - displayW) / 2;
    bgY      = (winH - displayH) / 2;
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
    return SDL2pp::Rect(
        bgX + static_cast<int>(x * scale),
        bgY + static_cast<int>(y * scale),
        static_cast<int>(w * scale),
        static_cast<int>(h * scale));
}

SDL2pp::Rect CharCreationScreen::getBoxRect(int i) const {
    int x = BOX_LEFT[i];
    int w = BOX_RIGHT[i] - BOX_LEFT[i];
    int h = BOX_BOTTOM - BOX_TOP;
    return pngToScreen(x, BOX_TOP, w, h);
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
                        running   = false;
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
    for (int i = 0; i < BOX_COUNT; i++) {
        SDL2pp::Rect box = getBoxRect(i);
        if (mouseX >= box.x && mouseX < box.x + box.w &&
            mouseY >= box.y && mouseY < box.y + box.h) {
            if (selectedSkin == i) {
                // Doble click sobre el mismo box confirma.
                confirmed = true;
                running   = false;
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
    int pad   = static_cast<int>(BOX_PAD * scale);
    int innerX = boxRect.x + pad;
    int innerY = boxRect.y + pad;
    int innerW = boxRect.w - pad * 2;
    int innerH = boxRect.h - pad * 2;

    // Tamaño del sprite escalado
    int bodyW = static_cast<int>(SPRITE_W   * SPRITE_SCL);
    int bodyH = static_cast<int>(SPRITE_H   * SPRITE_SCL);
    int headW = static_cast<int>(HEAD_CELL_W * SPRITE_SCL);
    int headH = static_cast<int>(HEAD_CELL_H * SPRITE_SCL);

    // Desplazamiento de la cabeza respecto al cuerpo (igual que en map_renderer)
    int headAboveBody = static_cast<int>((HEAD_CELL_H / 4 + 3) * SPRITE_SCL);

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
