#include "Controller/AudioManager.h"
#include "Model/Core/LogManager.h"
#include "ext/json.hpp"
#include <iostream>
#include <fstream>

using json = nlohmann::json;

namespace controller {

AudioManager::AudioManager() {
    initDatabase();
}

void AudioManager::initDatabase() {
    std::ifstream f("assets/audio/audio_meta.json");
    if (!f.is_open()) {
        model::LogManager::instance().warning("[AudioManager] Missing optional asset: audio_meta.json. Falling back to defaults");
        musicDB["menu"] = {"01. Ground Theme", true};
        musicDB["game_over"] = {"09. Game Over Theme", false};
        return;
    }

    try {
        json j;
        f >> j;
        for (auto& el : j.items()) {
            MusicMetadata meta;
            meta.filename = el.value()["file"];
            meta.loop = el.value()["loop"];
            musicDB[el.key()] = meta;
        }
    } catch (const std::exception& e) {
        model::LogManager::instance().error(std::string("[AudioManager] Failed to load sound: ") + e.what());
    }
}

void AudioManager::setMasterVolume(float volume) {
    masterVol = volume;
    applyMusicVolume();
    applySFXVolume();
}

void AudioManager::setMusicVolume(float volume) {
    musicVol = volume;
    applyMusicVolume();
}

void AudioManager::setSFXVolume(float volume) {
    sfxVol = volume;
    applySFXVolume();
}

void AudioManager::applyMusicVolume() {
    float finalMusicVol = (masterVol / 100.f) * (musicVol / 100.f) * 100.f;
    currentMusic.setVolume(finalMusicVol);
}

void AudioManager::applySFXVolume() {
    float finalSfxVol = (masterVol / 100.f) * (sfxVol / 100.f) * 100.f;
    for (auto& sound : activeSounds) {
        sound.setVolume(finalSfxVol);
    }
}

void AudioManager::playMusic(const std::string& trackId) {
    if (currentMusicName == trackId) return;
    
    auto it = musicDB.find(trackId);
    if (it == musicDB.end()) {
        model::LogManager::instance().warning("[AudioManager] Missing optional asset: music track '" + trackId + "' not found");
        return;
    }

    std::string path = "assets/audio/music/" + it->second.filename + ".mp3";
    if (currentMusic.openFromFile(path)) {
        currentMusicName = trackId;
        currentMusic.setLooping(it->second.loop);
        applyMusicVolume();
        currentMusic.play();
    } else {
        model::LogManager::instance().error("[AudioManager] Failed to load sound: " + path);
    }
}

void AudioManager::stopMusic() {
    currentMusic.stop();
    currentMusicName = "";
}

void AudioManager::playSound(const std::string& name) {
    // Assuming assets are in assets/audio/sfx/
    std::string path = "assets/audio/sfx/" + name + ".mp3";
    
    auto it = soundBuffers.find(name);
    if (it == soundBuffers.end()) {
        sf::SoundBuffer buffer;
        if (buffer.loadFromFile(path)) {
            soundBuffers[name] = buffer;
            it = soundBuffers.find(name);
        } else {
            model::LogManager::instance().error("[AudioManager] Failed to load sound: " + path);
            return;
        }
    }
    
    activeSounds.emplace_back(it->second);
    sf::Sound& sound = activeSounds.back();
    float finalSfxVol = (masterVol / 100.f) * (sfxVol / 100.f) * 100.f;
    sound.setVolume(finalSfxVol);
    sound.play();
}

void AudioManager::update() {
    // Remove stopped sounds from the list
    activeSounds.remove_if([](const sf::Sound& s) {
        return s.getStatus() == sf::Sound::Status::Stopped;
    });
}

}
