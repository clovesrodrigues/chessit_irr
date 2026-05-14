#include "Managers/SoundManager.h"

#include "Core/Logger.h"

#include <filesystem>

namespace chessit {

bool SoundManager::Initialize(const std::string& mediaDir) {
    const SoLoud::result result = soloud_.init();
    initialized_ = result == SoLoud::SO_NO_ERROR;
    if (!initialized_) {
        Logger::Warning("Soloud initialization failed; game will continue without audio.");
        return false;
    }

    // Load available music and SFX
    std::string musicDir = JoinPath(mediaDir, "sounds");
    std::string sfxDir = JoinPath(mediaDir, "sound_efx");

    LoadAvailableMusic(musicDir);
    LoadAvailableSFX(sfxDir);

    Logger::Info("SoundManager initialized successfully.");
    return true;
}

void SoundManager::Shutdown() {
    if (initialized_) {
        StopMusic();
        soloud_.deinit();
        initialized_ = false;
    }
}

std::string SoundManager::JoinPath(const std::string& base, const std::string& file) const {
    return (std::filesystem::path(base) / file).string();
}

void SoundManager::LoadAvailableMusic(const std::string& musicDir) {
    if (!std::filesystem::exists(musicDir)) {
        Logger::Warning("Music directory not found: " + musicDir);
        return;
    }

    try {
        for (const auto& entry : std::filesystem::directory_iterator(musicDir)) {
            if (!entry.is_regular_file()) continue;

            std::string ext = entry.path().extension().string();
            // Convert to lowercase for comparison
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            // Support common audio formats
            if (ext == ".wav" || ext == ".ogg" || ext == ".mp3" || ext == ".flac") {
                availableMusic_.push_back(entry.path().string());
            }
        }
        Logger::Info("Loaded " + std::to_string(availableMusic_.size()) + " music tracks from: " + musicDir);
    } catch (const std::exception& e) {
        Logger::Warning("Error loading music directory: " + std::string(e.what()));
    }
}

void SoundManager::LoadAvailableSFX(const std::string& sfxDir) {
    if (!std::filesystem::exists(sfxDir)) {
        Logger::Warning("Sound effects directory not found: " + sfxDir);
        return;
    }

    try {
        for (const auto& entry : std::filesystem::directory_iterator(sfxDir)) {
            if (!entry.is_regular_file()) continue;

            std::string ext = entry.path().extension().string();
            // Convert to lowercase for comparison
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            // Support common audio formats
            if (ext == ".wav" || ext == ".ogg" || ext == ".mp3" || ext == ".flac") {
                availableSFX_.push_back(entry.path().string());
            }
        }
        Logger::Info("Loaded " + std::to_string(availableSFX_.size()) + " sound effects from: " + sfxDir);
    } catch (const std::exception& e) {
        Logger::Warning("Error loading sound effects directory: " + std::string(e.what()));
    }
}

void SoundManager::PlayMusic(const std::string& musicPath) {
    if (!initialized_) {
        Logger::Warning("SoundManager not initialized, cannot play music.");
        return;
    }

    if (musicPath.empty()) {
        Logger::Warning("Music path is empty.");
        return;
    }

    // Stop current music if playing
    StopMusic();

    // Check if file exists
    if (!std::filesystem::exists(musicPath)) {
        Logger::Warning("Music file not found: " + musicPath);
        return;
    }

    try {
        // Try to load and play the music
        SoLoud::Wav music;
        SoLoud::result result = music.load(musicPath.c_str());
        if (result != SoLoud::SO_NO_ERROR) {
            Logger::Warning("Failed to load music file: " + musicPath);
            return;
        }

        // Set music to loop
        music.setLooping(1);

        // Play music with current volume
        musicHandle_ = soloud_.play(music);
        soloud_.setVolume(musicHandle_, musicVolume_);

        currentMusic_ = musicPath;
        Logger::Info("Now playing: " + musicPath);
    } catch (const std::exception& e) {
        Logger::Warning("Error playing music: " + std::string(e.what()));
    }
}

void SoundManager::StopMusic() {
    if (initialized_ && musicHandle_ != 0) {
        soloud_.stop(musicHandle_);
        musicHandle_ = 0;
        currentMusic_ = "";
    }
}

void SoundManager::SetMusicVolume(float volume) {
    musicVolume_ = volume;
    if (musicVolume_ < 0.0f) musicVolume_ = 0.0f;
    if (musicVolume_ > 1.0f) musicVolume_ = 1.0f;

    if (initialized_ && musicHandle_ != 0) {
        soloud_.setVolume(musicHandle_, musicVolume_);
    }
}

void SoundManager::PlaySFX(const std::string& sfxPath) {
    if (!initialized_) {
        Logger::Warning("SoundManager not initialized, cannot play SFX.");
        return;
    }

    if (sfxPath.empty()) {
        Logger::Warning("SFX path is empty.");
        return;
    }

    // Check if file exists
    if (!std::filesystem::exists(sfxPath)) {
        Logger::Warning("SFX file not found: " + sfxPath);
        return;
    }

    try {
        // Try to load and play the SFX
        SoLoud::Wav sfx;
        SoLoud::result result = sfx.load(sfxPath.c_str());
        if (result != SoLoud::SO_NO_ERROR) {
            Logger::Warning("Failed to load SFX file: " + sfxPath);
            return;
        }

        // Play SFX with current volume
        SoLoud::handle handle = soloud_.play(sfx);
        soloud_.setVolume(handle, sfxVolume_);

        Logger::Info("Playing SFX: " + sfxPath);
    } catch (const std::exception& e) {
        Logger::Warning("Error playing SFX: " + std::string(e.what()));
    }
}

void SoundManager::SetSFXVolume(float volume) {
    sfxVolume_ = volume;
    if (sfxVolume_ < 0.0f) sfxVolume_ = 0.0f;
    if (sfxVolume_ > 1.0f) sfxVolume_ = 1.0f;
}

} // namespace chessit
