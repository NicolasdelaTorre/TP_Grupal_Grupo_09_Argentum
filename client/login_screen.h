#pragma once

#include <string>

#include <SDL2/SDL_events.h>
#include <SDL2pp/Font.hh>
#include <SDL2pp/SDL2pp.hh>

// Resultado del login
struct LoginResult {
    std::string username;
    bool confirmed;  // true = presionó Enter, false = cerró la ventana
};

class LoginScreen {
public:
    explicit LoginScreen(SDL2pp::Renderer& renderer);

    // Corre el loop hasta que el usuario confirme o cierre.
    // Retorna el username ingresado.
    LoginResult run();

private:
    SDL2pp::Renderer& renderer;
    SDL2pp::Font fontTitle;
    SDL2pp::Font fontInput;
    SDL2pp::Font fontHint;

    std::string inputText;
    bool running;
    bool confirmed;

    static constexpr int SCREEN_W = 640;
    static constexpr int SCREEN_H = 400;
    static constexpr int MAX_USERNAME = 20;
    static constexpr float CURSOR_BLINK = 0.5f;  // segundos

    float cursorTimer = 0.0f;
    bool cursorVisible = true;

    void handleEvents();
    void handleKeyDown(const SDL_KeyboardEvent& key);
    void handleTextInput(const SDL_TextInputEvent& text);
    void update(float dt);
    void render();

    SDL2pp::Texture makeText(SDL2pp::Font& font, const std::string& text, SDL_Color color) const;
};
