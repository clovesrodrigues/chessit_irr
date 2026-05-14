#pragma once

#include "Game/GameEvents.h"

#include <soloud.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace chessit {

class SoundManager {
public:
    bool Initialize(const std::string& mediaDir);
    void Shutdown();

    void PlaySFX(const std::string& relativePath);
    bool PlayMusic(const std::string& musicPathOrName);
    void StopMusic();
    void SetMusicVolume(float volume);
    void SetSFXVolume(float volume);
    float GetMusicVolume() const { return musicVolume_; }
    float GetSFXVolume() const { return sfxVolume_; }

    void HandleEvent(GameEventSound eventSound);
    const std::vector<std::string>& GetAvailableMusic() const { return availableMusic_; }
    const std::string& GetCurrentMusic() const { return currentMusic_; }
    const std::string& GetMediaDir() const { return mediaDir_; }

private:
    SoLoud::Wav* LoadSFX(const std::string& relativePath);
    SoLoud::WavStream* LoadMusic(const std::string& musicPath);
    void ScanMusicDirectory();
    std::string ResolveMediaPath(const std::string& relativeOrAbsolute) const;
    std::string EventToPath(GameEventSound eventSound) const;

    SoLoud::Soloud soloud_;
    bool initialized_ = false;
    std::string mediaDir_;
    float musicVolume_ = 0.45f;
    float sfxVolume_ = 0.80f;
    SoLoud::handle musicHandle_ = 0;
    std::string currentMusic_;
    std::vector<std::string> availableMusic_;
    std::unordered_map<std::string, std::unique_ptr<SoLoud::Wav>> sfxCache_;
    std::unordered_map<std::string, std::unique_ptr<SoLoud::WavStream>> musicCache_;
    std::unordered_set<std::string> failedSFX_;
    std::unordered_set<std::string> failedMusic_;
};

} // namespace chessit
