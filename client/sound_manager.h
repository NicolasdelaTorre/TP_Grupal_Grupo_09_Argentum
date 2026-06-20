#pragma once

#include <optional>

#include <SDL2pp/Chunk.hh>
#include <SDL2pp/Mixer.hh>
#include <SDL2pp/Music.hh>

class SoundManager {
    SDL2pp::Mixer& mixer;
    SDL2pp::Music music;
    SDL2pp::Chunk swordChunk;
    SDL2pp::Chunk explosionChunk;

public:
    explicit SoundManager(SDL2pp::Mixer& mixer);

    void sword_sound();      // golpe de espada
    void explosion_sound();  // golpe de báculo (magia)
};
