#include "character_creation_screen.h"

#include <algorithm>

#include <SDL2/SDL.h>

namespace {
// El orden coincide con los enums RaceCode / ClassCode (DTOs.h), así que el
// índice seleccionado se castea directo al enum.
const char* kRaceLabels[] = {"Humano", "Elfo", "Enano", "Gnomo"};
const char* kClassLabels[] = {"Mago", "Clerigo", "Campeon", "Guerrero"};
}  // namespace

CharacterCreationScreen::CharacterCreationScreen(SDL2pp::Renderer& renderer,
                                                 const std::string& assetsPath):
        renderer(renderer),
        ttf(),
        font(assetsPath + "/font.ttf", 28),
        background(renderer,
                   SDL2pp::Surface(assetsPath + "/Pantallas/Pre-seleccion_personaje.png")) {
    int winW, winH;
    SDL_GetRendererOutputSize(renderer.Get(), &winW, &winH);
    scaleX = static_cast<float>(winW) / BG_W;
    scaleY = static_cast<float>(winH) / BG_H;
    displayW = winW;
    displayH = winH;
}

CharacterCreationResult CharacterCreationScreen::run() {
    while (running) {
        handleEvents();
        render();
        SDL_Delay(16);
    }
    return {static_cast<RaceCode>(selectedRace), static_cast<ClassCode>(selectedClass), confirmed};
}

const char* const* CharacterCreationScreen::currentLabels() const {
    return phase == Phase::RACE ? kRaceLabels : kClassLabels;
}

const char* CharacterCreationScreen::currentTitle() const {
    return phase == Phase::RACE ? "Elegi tu raza" : "Elegi tu clase";
}

SDL2pp::Rect CharacterCreationScreen::bgToScreen(int x, int y, int w, int h) const {
    return SDL2pp::Rect(static_cast<int>(x * scaleX), static_cast<int>(y * scaleY),
                        static_cast<int>(w * scaleX), static_cast<int>(h * scaleY));
}

SDL2pp::Rect CharacterCreationScreen::optionRect(int i) const {
    return bgToScreen(OPT_X, OPT_TOP + i * OPT_VSPACE, OPT_W, OPT_H);
}

bool CharacterCreationScreen::inBgRect(int mx, int my, int x, int y, int w, int h) const {
    SDL2pp::Rect r = bgToScreen(x, y, w, h);
    return mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h;
}

void CharacterCreationScreen::advanceOrConfirm() {
    if (phase == Phase::RACE) {
        phase = Phase::CLASS;
    } else {
        confirmed = true;
        running = false;
    }
}

void CharacterCreationScreen::goBack() {
    if (phase == Phase::CLASS) {
        phase = Phase::RACE;
    } else {
        // Cancelar desde la primer fase cierra la pantalla sin confirmar.
        running = false;
    }
}

void CharacterCreationScreen::handleEvents() {
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
                        advanceOrConfirm();
                        break;
                    case SDLK_ESCAPE:
                        goBack();
                        break;
                    case SDLK_UP: {
                        int& sel = (phase == Phase::RACE) ? selectedRace : selectedClass;
                        sel = (sel - 1 + NUM_OPTIONS) % NUM_OPTIONS;
                        break;
                    }
                    case SDLK_DOWN: {
                        int& sel = (phase == Phase::RACE) ? selectedRace : selectedClass;
                        sel = (sel + 1) % NUM_OPTIONS;
                        break;
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (e.button.button == SDL_BUTTON_LEFT)
                    handleMouseClick(e.button.x, e.button.y);
                break;
        }
    }
}

void CharacterCreationScreen::handleMouseClick(int mx, int my) {
    if (inBgRect(mx, my, BTN_CREATE_X, BTN_CREATE_Y, BTN_CREATE_W, BTN_CREATE_H)) {
        advanceOrConfirm();
        return;
    }
    if (inBgRect(mx, my, BTN_BACK_X, BTN_BACK_Y, BTN_BACK_W, BTN_BACK_H)) {
        goBack();
        return;
    }
    for (int i = 0; i < NUM_OPTIONS; i++) {
        if (inBgRect(mx, my, OPT_X, OPT_TOP + i * OPT_VSPACE, OPT_W, OPT_H)) {
            int& sel = (phase == Phase::RACE) ? selectedRace : selectedClass;
            if (sel == i)
                advanceOrConfirm();  // segundo click sobre lo ya elegido = confirmar/avanzar.
            else
                sel = i;
            return;
        }
    }
}

void CharacterCreationScreen::drawCenteredText(const std::string& text, int bgCx, int bgCy,
                                               SDL_Color color) {
    try {
        auto surface = font.RenderUTF8_Blended(text, color);
        SDL2pp::Texture tex(renderer, surface);
        float s = std::min(scaleX, scaleY);
        int w = static_cast<int>(tex.GetWidth() * s);
        int h = static_cast<int>(tex.GetHeight() * s);
        int x = static_cast<int>(bgCx * scaleX) - w / 2;
        int y = static_cast<int>(bgCy * scaleY) - h / 2;
        renderer.Copy(tex, SDL2pp::NullOpt, SDL2pp::Rect(x, y, w, h));
    } catch (...) {}
}

void CharacterCreationScreen::render() {
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();
    renderer.Copy(background, SDL2pp::NullOpt, SDL2pp::Rect(0, 0, displayW, displayH));

    drawCenteredText(currentTitle(), TITLE_CX, TITLE_CY, TITLE_CLR);

    const char* const* labels = currentLabels();
    int sel = selectedIndex();
    for (int i = 0; i < NUM_OPTIONS; i++) {
        SDL2pp::Rect box = optionRect(i);
        if (i == sel) {
            renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
            renderer.SetDrawColor(HIGHLIGHT.r, HIGHLIGHT.g, HIGHLIGHT.b, HIGHLIGHT.a);
            renderer.FillRect(box);
            renderer.SetDrawColor(HIGHLIGHT.r, HIGHLIGHT.g, HIGHLIGHT.b, 255);
            renderer.DrawRect(box);
            renderer.SetDrawBlendMode(SDL_BLENDMODE_NONE);
        }
        drawCenteredText(labels[i], OPT_X + OPT_W / 2, OPT_TOP + i * OPT_VSPACE + OPT_H / 2,
                         TEXT_CLR);
    }

    renderer.Present();
}
