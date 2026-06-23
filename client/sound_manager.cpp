#include "sound_manager.h"

#include <iostream>

namespace {
constexpr int SFX_CHANNELS = 16;  // máximo de SFX simultáneos (la música va aparte)
}  // namespace

SoundManager::SoundManager(SDL2pp::Mixer& mixer):
        mixer(mixer), music("client/Audio/Musica_fondo.ogg"), 
        swordChunk("client/Audio/Sword_sound.mp3"),
        explosionChunk("client/Audio/explosion.mp3") {
    this->mixer.SetMusicVolume(20);
    this->mixer.PlayMusic(music, -1);  // -1 = loop infinito
    this->mixer.AllocateChannels(SFX_CHANNELS);
    this->mixer.SetVolume(-1, 30);  

}

void SoundManager::sword_sound() {
    try {
        mixer.PlayChannel(-1, swordChunk, 0);
    } catch (const std::exception& e) {
        std::cerr << "Sound suppress" << std::endl;
    }
}

void SoundManager::explosion_sound() {
    try {
        mixer.PlayChannel(-1, explosionChunk, 0);
    } catch (const std::exception& e) {
        std::cerr << "Sound suppress" << std::endl;
    }
}
