#pragma once

#include "Managers/UIAudioMenu.h"

#include <irrlicht.h>
#include <string>

namespace chessit {
class AIManager;
class SoundManager;

class UIManager {
public:
    void Initialize(irr::gui::IGUIEnvironment* guiEnvironment, irr::video::IVideoDriver* driver, const std::string& mediaDir, SoundManager* soundManager, AIManager* aiManager);
    bool OnEvent(const irr::SEvent& event);
    void Update();
    UIAudioMenu& GetAudioMenu() { return audioMenu_; }

private:
    irr::gui::IGUIEnvironment* guiEnvironment_ = nullptr;
    UIAudioMenu audioMenu_;
};

} // namespace chessit
