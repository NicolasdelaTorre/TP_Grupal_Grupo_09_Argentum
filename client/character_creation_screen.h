#pragma once

#include <string>

#include <SDL2pp/SDL2pp.hh>
#include <SDL2pp/SDLTTF.hh>

#include "../common/DTOs.h"  // RaceCode, ClassCode

// Resultado de la pantalla de creación de personaje. confirmed=false si el
// jugador canceló (cerró la ventana o salió desde "VOLVER" en la primer fase).
struct CharacterCreationResult {
    RaceCode race = RaceCode::HUMAN;
    ClassCode class_ = ClassCode::MAGE;
    bool confirmed = false;
};

// Pantalla previa al ingreso: primero se elige RAZA, luego CLASE, sobre el mismo
// fondo (Pre-seleccion_personaje.png). "CREAR PERSONAJE" avanza/confirma y
// "VOLVER" retrocede de fase (o cancela en la fase de raza). Las dos etiquetas
// de los botones de abajo ya vienen dibujadas en el PNG.
class CharacterCreationScreen {
public:
    CharacterCreationScreen(SDL2pp::Renderer& renderer, const std::string& assetsPath);
    CharacterCreationResult run();

private:
    enum class Phase { RACE, CLASS };

    SDL2pp::Renderer& renderer;
    SDL2pp::SDLTTF ttf;
    SDL2pp::Font font;
    SDL2pp::Texture background;

    Phase phase = Phase::RACE;
    int selectedRace = 0;
    int selectedClass = 0;
    bool running = true;
    bool confirmed = false;

    float scaleX = 1.0f, scaleY = 1.0f;
    int displayW = 0, displayH = 0;

    // Coordenadas en el espacio del PNG (512x512), escaladas a la ventana.
    static constexpr int BG_W = 512;
    static constexpr int BG_H = 512;
    static constexpr int NUM_OPTIONS = 4;

    // Lista vertical de opciones en el panel central.
    static constexpr int OPT_W = 260;
    static constexpr int OPT_H = 44;
    static constexpr int OPT_X = (BG_W - OPT_W) / 2;
    static constexpr int OPT_TOP = 110;
    static constexpr int OPT_VSPACE = 52;
    static constexpr int TITLE_CX = BG_W / 2;
    static constexpr int TITLE_CY = 70;

    // Botones inferiores ya dibujados en el fondo (sólo se usan sus áreas).
    static constexpr int BTN_BACK_X = 58, BTN_BACK_Y = 347, BTN_BACK_W = 147, BTN_BACK_H = 36;
    static constexpr int BTN_CREATE_X = 292, BTN_CREATE_Y = 347, BTN_CREATE_W = 186,
                         BTN_CREATE_H = 36;

    static constexpr SDL_Color HIGHLIGHT = {200, 170, 50, 160};
    static constexpr SDL_Color TEXT_CLR = {235, 225, 200, 255};
    static constexpr SDL_Color TITLE_CLR = {255, 215, 120, 255};

    SDL2pp::Rect bgToScreen(int x, int y, int w, int h) const;
    SDL2pp::Rect optionRect(int i) const;
    bool inBgRect(int mx, int my, int x, int y, int w, int h) const;
    int selectedIndex() const { return phase == Phase::RACE ? selectedRace : selectedClass; }
    const char* const* currentLabels() const;
    const char* currentTitle() const;

    void handleEvents();
    void handleMouseClick(int mx, int my);
    void advanceOrConfirm();
    void goBack();
    void render();
    void drawCenteredText(const std::string& text, int bgCx, int bgCy, SDL_Color color);
};
