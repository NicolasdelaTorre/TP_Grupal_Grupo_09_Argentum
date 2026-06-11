#pragma once

#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <SDL2pp/SDL2pp.hh>


class TextureCache {
public:
    TextureCache(SDL2pp::Renderer& renderer, const std::string& basePath):
            renderer(renderer), basePath(basePath) {}

    SDL2pp::Texture& get(const std::string& filename) {
        auto it = cache.find(filename);
        if (it != cache.end())
            return it->second;

        std::string path = basePath + "/" + filename;
        
        try {
            cache.emplace(filename, SDL2pp::Texture(renderer, SDL2pp::Surface(path)));
        } catch (const std::exception& e) {
            std::cerr << "[TextureCache] no se pudo cargar '" << path << "': " << e.what()
                      << " — usando placeholder" << std::endl;
            cache.emplace(filename, makePlaceholder());
        }
        return cache.at(filename);
    }

private:
    SDL2pp::Renderer& renderer;
    std::string basePath;
    std::unordered_map<std::string, SDL2pp::Texture> cache;

    // Textura magenta 16x16 para marcar visualmente un asset que no cargó.
    SDL2pp::Texture makePlaceholder() {
        SDL2pp::Surface surf(0, 16, 16, 32, 0x000000FFu, 0x0000FF00u, 0x00FF0000u, 0xFF000000u);
        Uint32 magenta = SDL_MapRGBA(surf.Get()->format, 255, 0, 255, 255);
        surf.FillRect(SDL2pp::NullOpt, magenta);
        return SDL2pp::Texture(renderer, surf);
    }
};
