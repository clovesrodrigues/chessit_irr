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
    void DrawOverlay();
    void ShowGameOver(bool playerWon);
    void HideGameOver();
    bool IsGameOverVisible() const { return gameOverVisible_; }
    bool ConsumeNewGameRequested();
    UIAudioMenu& GetAudioMenu() { return audioMenu_; }

private:
    static constexpr irr::s32 NewGameButtonId = 0x5101;

    irr::gui::IGUIEnvironment* guiEnvironment_ = nullptr;
    irr::video::IVideoDriver* driver_ = nullptr;
    SoundManager* soundManager_ = nullptr;
    std::string mediaDir_;
    UIAudioMenu audioMenu_;
    irr::video::ITexture* gameOverTexture_ = nullptr;
    irr::gui::IGUIButton* newGameButton_ = nullptr;
    bool gameOverVisible_ = false;
    bool playerWon_ = false;
    bool newGameRequested_ = false;
    float gameOverZoom_ = 0.1f;
};

} // namespace chessit
