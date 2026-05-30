#pragma once

#include <string>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "../common/DTOs.h"


class LoginScreen {
public:
    explicit LoginScreen(SDL2pp::Renderer& renderer, const std::string& assetsPath);
    LoginResult run();

private:
    SDL2pp::Renderer& renderer;
    SDL2pp::SDLTTF ttf;
    SDL2pp::Font font;
    SDL2pp::Texture background;

    std::vector<char> inputText;
    bool running = true;
    bool confirmed = false;


    static constexpr int PNG_W = 512;
    static constexpr int PNG_H = 512;
    static constexpr int FIELD_X = 120;  // inicio campo en PNG
    static constexpr int FIELD_Y = 198;  // inicio campo en PNG
    static constexpr int FIELD_W = 122;  // ancho campo
    static constexpr int FIELD_H = 20;   // alto campo
    static constexpr int MAX_LENGTH = 20;
    static constexpr float SCALE = 1.3f;
    static constexpr int PNG_W_SCL = (int)(PNG_W * SCALE);
    static constexpr int PNG_H_SCL = (int)(PNG_H * SCALE);


    float cursorTimer = 0.0f;
    bool cursorVisible = true;
    static constexpr float CURSOR_BLINK = 0.5f;

    void handleEvents();
    void handleKeyDown(const SDL_KeyboardEvent& key);
    void handleTextInput(const SDL_TextInputEvent& text);
    void update(float dt);
    void render();


    SDL2pp::Rect pngToScreen(int x, int y, int w, int h);
};
