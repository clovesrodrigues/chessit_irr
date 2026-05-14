#include "Managers/SoundManager.h"

#include "Core/Logger.h"

namespace chessit {

bool SoundManager::Initialize(const std::string& mediaDir) {
    (void)mediaDir;
    const SoLoud::result result = soloud_.init();
    initialized_ = result == SoLoud::SO_NO_ERROR;
    if (!initialized_) {
        Logger::Warning("Soloud initialization failed; game will continue without audio.");
    }
    return initialized_;
}

void SoundManager::Shutdown() {
    if (initialized_) {
        soloud_.deinit();
        initialized_ = false;
    }
}

} // namespace chessit
