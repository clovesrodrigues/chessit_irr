#include "Managers/SoundManager.h"

#include "Core/Logger.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <filesystem>

namespace chessit {
namespace {
float Clamp01(float value) { return std::max(0.0f, std::min(1.0f, value)); }
}

bool SoundManager::Initialize(const std::string& mediaDir) {
    mediaDir_ = mediaDir;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
#if defined(_WIN32)
    const SoLoud::result result = soloud_.init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::WINMM);
#else
    const SoLoud::result result = soloud_.init();
#endif
    initialized_ = result == SoLoud::SO_NO_ERROR;
    if (!initialized_) {
        Logger::Warning(std::string("Soloud initialization failed; game will continue without audio. Reason: ") + soloud_.getErrorString(result));
        return false;
    }

    const std::filesystem::path sfxDir = std::filesystem::path(mediaDir_) / "sound_efx";
    if (!std::filesystem::exists(sfxDir)) {
        Logger::Warning("SFX directory not found: " + sfxDir.string());
    }

    ScanMusicDirectory();
    if (!availableMusic_.empty()) {
        const std::size_t index = static_cast<std::size_t>(std::rand()) % availableMusic_.size();
        PlayMusic(availableMusic_[index]);
    } else {
        Logger::Warning("No music files found in: " + (std::filesystem::path(mediaDir_) / "sounds").string());
    }
    Logger::Info("SoundManager initialized with " + std::to_string(availableMusic_.size()) + " music track(s).");
    return true;
}

void SoundManager::Shutdown() {
    if (initialized_) {
        StopMusic();
        sfxCache_.clear();
        musicCache_.clear();
        failedSFX_.clear();
        failedMusic_.clear();
        soloud_.deinit();
        initialized_ = false;
    }
}

void SoundManager::PlaySFX(const std::string& relativePath) {
    if (!initialized_) return;
    SoLoud::Wav* sound = LoadSFX(relativePath);
    if (!sound) return;
    soloud_.play(*sound, sfxVolume_);
}

bool SoundManager::PlayMusic(const std::string& musicPathOrName) {
    if (!initialized_) return false;
    const std::string musicPath = ResolveMediaPath(musicPathOrName);
    SoLoud::WavStream* music = LoadMusic(musicPath);
    if (!music) return false;

    StopMusic();
    music->setLooping(true);
    musicHandle_ = soloud_.play(*music, musicVolume_);
    soloud_.setLooping(musicHandle_, true);
    currentMusic_ = musicPath;
    Logger::Info("Now playing music: " + musicPath);
    return true;
}

void SoundManager::StopMusic() {
    if (initialized_ && musicHandle_ != 0) {
        soloud_.stop(musicHandle_);
        musicHandle_ = 0;
    }
}

void SoundManager::SetMusicVolume(float volume) {
    musicVolume_ = Clamp01(volume);
    if (initialized_ && musicHandle_ != 0) soloud_.setVolume(musicHandle_, musicVolume_);
}

void SoundManager::SetSFXVolume(float volume) {
    sfxVolume_ = Clamp01(volume);
}

void SoundManager::HandleEvent(GameEventSound eventSound) {
    const std::string path = EventToPath(eventSound);
    if (!path.empty()) PlaySFX(path);
}

SoLoud::Wav* SoundManager::LoadSFX(const std::string& relativePath) {
    const std::string path = ResolveMediaPath(relativePath);
    const auto found = sfxCache_.find(path);
    if (found != sfxCache_.end()) return found->second.get();
    if (failedSFX_.find(path) != failedSFX_.end()) return nullptr;

    auto wav = std::make_unique<SoLoud::Wav>();
    const SoLoud::result result = wav->load(path.c_str());
    if (result != SoLoud::SO_NO_ERROR) {
        Logger::Warning(std::string("Could not load SFX: ") + path + " (" + soloud_.getErrorString(result) + ")");
        failedSFX_.insert(path);
        return nullptr;
    }
    SoLoud::Wav* raw = wav.get();
    sfxCache_[path] = std::move(wav);
    return raw;
}

SoLoud::WavStream* SoundManager::LoadMusic(const std::string& musicPath) {
    const auto found = musicCache_.find(musicPath);
    if (found != musicCache_.end()) return found->second.get();
    if (failedMusic_.find(musicPath) != failedMusic_.end()) return nullptr;

    auto stream = std::make_unique<SoLoud::WavStream>();
    const SoLoud::result result = stream->load(musicPath.c_str());
    if (result != SoLoud::SO_NO_ERROR) {
        Logger::Warning(std::string("Could not load music: ") + musicPath + " (" + soloud_.getErrorString(result) + ")");
        failedMusic_.insert(musicPath);
        return nullptr;
    }
    SoLoud::WavStream* raw = stream.get();
    musicCache_[musicPath] = std::move(stream);
    return raw;
}

void SoundManager::ScanMusicDirectory() {
    availableMusic_.clear();
    const std::filesystem::path musicDir = std::filesystem::path(mediaDir_) / "sounds";
    if (!std::filesystem::exists(musicDir)) return;

    for (const auto& entry : std::filesystem::directory_iterator(musicDir)) {
        if (!entry.is_regular_file()) continue;
        std::string extension = entry.path().extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (extension == ".mp3" || extension == ".ogg" || extension == ".wav" || extension == ".flac") {
            availableMusic_.push_back(entry.path().string());
        }
    }
    std::sort(availableMusic_.begin(), availableMusic_.end());
}

std::string SoundManager::ResolveMediaPath(const std::string& relativeOrAbsolute) const {
    const std::filesystem::path path(relativeOrAbsolute);
    if (path.is_absolute() || std::filesystem::exists(path)) return path.string();
    return (std::filesystem::path(mediaDir_) / relativeOrAbsolute).string();
}

std::string SoundManager::EventToPath(GameEventSound eventSound) const {
    switch (eventSound) {
        case GameEventSound::PieceHover: return "sound_efx/select.wav";
        case GameEventSound::SquareHover: return "sound_efx/select.wav";
        case GameEventSound::PieceMove: return "sound_efx/pop.wav";
        case GameEventSound::PlayerCapture: return "sound_efx/drop.wav";
        case GameEventSound::ComputerCapture: return "sound_efx/lost_char.wav";
        case GameEventSound::PlayerWin: return "sound_efx/game_win.wav";
        case GameEventSound::PlayerLose: return "sound_efx/game_lost.wav";
    }
    return {};
}

} // namespace chessit
