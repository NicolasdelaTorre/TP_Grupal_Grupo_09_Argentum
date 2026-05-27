#include "login_screen.h"
#include <SDL2/SDL.h>

static constexpr SDL_Color CLR_TEXT   = {220, 220, 220, 255};
static constexpr SDL_Color CLR_CURSOR = {255, 255, 255, 255};

LoginScreen::LoginScreen(SDL2pp::Renderer& renderer,
                         const std::string& assetsPath)
    : renderer(renderer)
    , ttf()
    , font(assetsPath + "/font.ttf", (int)(11 * SCALE)) 
    , background(renderer, SDL2pp::Surface(assetsPath + "/login.png"))
{}

LoginResult LoginScreen::run() {
    SDL_StartTextInput();

    Uint32 lastTime = SDL_GetTicks();

    while (running) {
        Uint32 now = SDL_GetTicks();
        float  dt  = (now - lastTime) / 1000.0f;
        lastTime   = now;

        handleEvents();
        update(dt);
        render();
        SDL_Delay(16);
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
                running = false; break;
            case SDL_KEYDOWN:
                handleKeyDown(e.key); break;
            case SDL_TEXTINPUT:
                handleTextInput(e.text); break;
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
            running = false;
            break;
    }
}

void LoginScreen::handleTextInput(const SDL_TextInputEvent& text) {
    if ((int)inputText.size() >= MAX_LENGTH) return;
    for (char c : std::string(text.text))
        if (std::isalnum(c) || c == '_')
            inputText += c;
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
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();

    // Fondo: PNG centrado en pantalla
    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer.Get(), &screenW, &screenH);
    int bgX = screenW / 2 - PNG_W_SCL / 2 + 75;
    int bgY = screenH / 2 - PNG_H_SCL / 2 + 100;
    renderer.Copy(background, SDL2pp::NullOpt,
              SDL2pp::Rect(bgX, bgY, PNG_W_SCL, PNG_H_SCL));

    // Campo de nombre — posicionado sobre el PNG
    SDL2pp::Rect fieldRect = pngToScreen(FIELD_X, FIELD_Y, FIELD_W, FIELD_H);

    // Texto ingresado
    std::string display = inputText + (cursorVisible ? "|" : " ");
    if (!display.empty()) {
        try {
            auto surface = font.RenderUTF8_Blended(display, CLR_TEXT);
            SDL2pp::Texture tex(renderer, surface);
            auto [tw, th] = tex.GetSize();

            // Centrado verticalmente en el campo
            int textY = fieldRect.y + (fieldRect.h - th) / 2;
            int textX = fieldRect.x + 4;

            renderer.Copy(tex, SDL2pp::NullOpt,
                          SDL2pp::Rect(textX, textY,
                                       std::min(tw, FIELD_W - 8), th));
        } catch (...) {}
    }

    renderer.Present();
}

// ── Helper ────────────────────────────────────────────────────

SDL2pp::Rect LoginScreen::pngToScreen(int x, int y, int w, int h) {
    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer.Get(), &screenW, &screenH);
    int bgX = screenW / 2 - PNG_W_SCL / 2;
    int bgY = screenH / 2 - PNG_H_SCL / 2;
    return SDL2pp::Rect(
        bgX + (int)(x * SCALE),
        bgY + (int)(y * SCALE),
        (int)(w * SCALE),
        (int)(h * SCALE)
    );
}
