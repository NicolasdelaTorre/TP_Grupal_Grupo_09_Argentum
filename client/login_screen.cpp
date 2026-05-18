#include "login_screen.h"
#include <SDL2/SDL.h>
#include <algorithm>

// Colores
static constexpr SDL_Color CLR_BG       = {15,  15,  30,  255};
static constexpr SDL_Color CLR_TITLE    = {226, 201, 126, 255};  // dorado
static constexpr SDL_Color CLR_BOX_BG   = {25,  25,  55,  255};
static constexpr SDL_Color CLR_BOX_BD   = {80,  80,  160, 255};
static constexpr SDL_Color CLR_BOX_ACTIVE = {226, 201, 126, 255};
static constexpr SDL_Color CLR_TEXT     = {220, 220, 220, 255};
static constexpr SDL_Color CLR_HINT     = {100, 100, 140, 255};
static constexpr SDL_Color CLR_CURSOR   = {226, 201, 126, 255};

LoginScreen::LoginScreen(SDL2pp::Renderer& renderer)
    : renderer(renderer)
    , fontTitle("../common/Alegreya-Sans-AO-Bold.ttf", 28)  // ajustá la ruta a tu fuente
    , fontInput("../common/Alegreya-Sans-AO-Bold.ttf", 20)
    , fontHint ("../common/Alegreya-Sans-AO-Bold.ttf", 14)
    , running(true)
    , confirmed(false)
{}

LoginResult LoginScreen::run() {
    SDL_StartTextInput();  // habilita eventos SDL_TEXTINPUT

    Uint32 lastTime = SDL_GetTicks();

    while (running) {
        Uint32 now = SDL_GetTicks();
        float  dt  = (now - lastTime) / 1000.0f;
        lastTime   = now;

        handleEvents();
        update(dt);
        render();

        SDL_Delay(16);  // ~60 FPS
    }

    SDL_StopTextInput();

    return { inputText, confirmed };
}

// ── Eventos ───────────────────────────────────────────────────

void LoginScreen::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                running   = false;
                confirmed = false;
                break;
            case SDL_KEYDOWN:
                handleKeyDown(e.key);
                break;
            case SDL_TEXTINPUT:
                handleTextInput(e.text);
                break;
        }
    }
}

void LoginScreen::handleKeyDown(const SDL_KeyboardEvent& key) {
    switch (key.keysym.sym) {

        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (!inputText.empty()) {
                confirmed = true;
                running   = false;
            }
            break;

        case SDLK_BACKSPACE:
            if (!inputText.empty())
                inputText.pop_back();
            break;

        case SDLK_ESCAPE:
            running   = false;
            confirmed = false;
            break;
    }
}

void LoginScreen::handleTextInput(const SDL_TextInputEvent& text) {
    if (inputText.size() >= MAX_USERNAME) return;

    const std::string newChar(text.text);

    for (const char c : newChar) {
        if (std::isalnum(c) || c == '_') {
            inputText += c;
        }
    }
}

// ── Update ────────────────────────────────────────────────────

void LoginScreen::update(float dt) {
    cursorTimer += dt;
    if (cursorTimer >= CURSOR_BLINK) {
        cursorTimer  -= CURSOR_BLINK;
        cursorVisible = !cursorVisible;
    }
}

// ── Render ────────────────────────────────────────────────────

void LoginScreen::render() {
    // Fondo
    renderer.SetDrawColor(CLR_BG.r, CLR_BG.g, CLR_BG.b, 255);
    renderer.Clear();

    const int cx = SCREEN_W / 2;

    // ── Título ────────────────────────────────────────────────
    {
        auto tex = makeText(fontTitle, "Argentum Online", CLR_TITLE);
        auto [w, h] = tex.GetSize();
        renderer.Copy(tex, SDL2pp::NullOpt,
                      SDL2pp::Rect(cx - w/2, 80, w, h));
    }

    // ── Subtítulo ─────────────────────────────────────────────
    {
        auto tex = makeText(fontHint, "Ingresá tu nombre de usuario", CLR_HINT);
        auto [w, h] = tex.GetSize();
        renderer.Copy(tex, SDL2pp::NullOpt,
                      SDL2pp::Rect(cx - w/2, 130, w, h));
    }

    // ── Caja de input ─────────────────────────────────────────
    const int boxW = 300;
    const int boxH = 48;
    const int boxX = cx - boxW / 2;
    const int boxY = 180;

    // Fondo de la caja
    renderer.SetDrawColor(CLR_BOX_BG.r, CLR_BOX_BG.g, CLR_BOX_BG.b, 255);
    renderer.FillRect(SDL2pp::Rect(boxX, boxY, boxW, boxH));

    // Borde (dorado si tiene texto, gris si está vacío)
    SDL_Color borderColor = inputText.empty() ? CLR_BOX_BD : CLR_BOX_ACTIVE;
    renderer.SetDrawColor(borderColor.r, borderColor.g, borderColor.b, 255);
    renderer.DrawRect(SDL2pp::Rect(boxX, boxY, boxW, boxH));

    // Texto ingresado + cursor
    std::string displayText = inputText + (cursorVisible ? "|" : " ");
    if (!displayText.empty()) {
        auto tex = makeText(fontInput, displayText, CLR_TEXT);
        auto [w, h] = tex.GetSize();
        int  textY  = boxY + (boxH - h) / 2;
        // Clampear al ancho de la caja con padding
        int  textX  = boxX + 12;
        renderer.Copy(tex, SDL2pp::NullOpt,
                      SDL2pp::Rect(textX, textY, std::min(w, boxW - 24), h));
    }

    // Placeholder si está vacío
    if (inputText.empty()) {
        auto tex = makeText(fontInput, "ej: Gandalf_99", CLR_HINT);
        auto [w, h] = tex.GetSize();
        renderer.Copy(tex, SDL2pp::NullOpt,
                      SDL2pp::Rect(boxX + 12, boxY + (boxH-h)/2, w, h));
    }

    // ── Hint Enter ────────────────────────────────────────────
    {
        std::string hint = inputText.empty()
            ? "Escribí tu nombre y presioná Enter"
            : "Presioná Enter para ingresar";
        SDL_Color hintColor = inputText.empty() ? CLR_HINT : CLR_TITLE;
        auto tex = makeText(fontHint, hint, hintColor);
        auto [w, h] = tex.GetSize();
        renderer.Copy(tex, SDL2pp::NullOpt,
                      SDL2pp::Rect(cx - w/2, 250, w, h));
    }

    // ── Límite de caracteres ──────────────────────────────────
    {
        std::string counter = std::to_string(inputText.size())
                            + "/" + std::to_string(MAX_USERNAME);
        auto tex = makeText(fontHint, counter, CLR_HINT);
        auto [w, h] = tex.GetSize();
        renderer.Copy(tex, SDL2pp::NullOpt,
                      SDL2pp::Rect(boxX + boxW - w - 4, boxY + boxH + 6, w, h));
    }

    renderer.Present();
}

// ── Helper ────────────────────────────────────────────────────

SDL2pp::Texture LoginScreen::makeText(SDL2pp::Font& font,
                                       const std::string& text,
                                       const SDL_Color color) const
{
    return SDL2pp::Texture(renderer,
        font.RenderUTF8_Blended(text, color));
}
