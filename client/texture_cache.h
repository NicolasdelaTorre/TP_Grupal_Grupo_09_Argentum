#pragma once

#include <SDL2pp/SDL2pp.hh>
#include <unordered_map>
#include <string>
#include <stdexcept>



class TextureCache {
public:
    TextureCache(SDL2pp::Renderer& renderer, const std::string& basePath)
        : renderer(renderer), basePath(basePath) {}

    SDL2pp::Texture& get(const std::string& filename) {
        auto it = cache.find(filename);
        if (it != cache.end()) return it->second;

        std::string path = basePath + "/" + filename;
        cache.emplace(filename, SDL2pp::Texture(renderer, SDL2pp::Surface(path)));
        return cache.at(filename);
    }

private:
    SDL2pp::Renderer& renderer;
    std::string       basePath;
    std::unordered_map<std::string, SDL2pp::Texture> cache;
};