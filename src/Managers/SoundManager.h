#pragma once

#include <soloud.h>
#include <string>

namespace chessit {

class SoundManager {
public:
    bool Initialize(const std::string& mediaDir);
    void Shutdown();

private:
    SoLoud::Soloud soloud_;
    bool initialized_ = false;
};

} // namespace chessit
