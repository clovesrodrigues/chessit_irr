#include "Managers/UIManager.h"

#include "Game/GameEvents.h"
#include "Managers/SoundManager.h"

#include <algorithm>
#include <filesystem>

namespace chessit {

void UIManager::Initialize(irr::gui::IGUIEnvironment* guiEnvironment, irr::video::IVideoDriver* driver, const std::string& mediaDir, SoundManager* soundManager, AIManager* aiManager) {
    guiEnvironment_ = guiEnvironment;
    driver_ = driver;
    soundManager_ = soundManager;
    mediaDir_ = mediaDir;
    if (guiEnvironment_) {
        guiEnvironment_->addStaticText(L"_ ChessIt 3D by Cloves Rodrigues _ ", irr::core::rect<irr::s32>(16, 16, 320, 58), false, true);
        const std::string configPath = (std::filesystem::path(mediaDir) / "config.ini").string();
        audioMenu_.Initialize(guiEnvironment_, soundManager, aiManager, configPath);
    }
}

bool UIManager::OnEvent(const irr::SEvent& event) {
    if (gameOverVisible_ && event.EventType == irr::EET_GUI_EVENT &&
        event.GUIEvent.EventType == irr::gui::EGET_BUTTON_CLICKED && event.GUIEvent.Caller &&
        event.GUIEvent.Caller->getID() == NewGameButtonId) {
        newGameRequested_ = true;
        return true;
    }
    return audioMenu_.OnEvent(event);
}

void UIManager::Update() {
    audioMenu_.UpdateAIStatus();
    if (!gameOverVisible_) return;
    gameOverZoom_ += (1.0f - gameOverZoom_) * 0.08f;
    gameOverZoom_ = std::min(1.0f, gameOverZoom_);
}

void UIManager::DrawOverlay() {
    if (!gameOverVisible_ || !driver_) return;

    const irr::core::dimension2du screen = driver_->getScreenSize();
    driver_->draw2DRectangle(irr::video::SColor(160, 0, 0, 0), irr::core::rect<irr::s32>(0, 0, screen.Width, screen.Height));

    if (!gameOverTexture_) return;

    const irr::core::dimension2du textureSize = gameOverTexture_->getOriginalSize();
    const float maxWidth = static_cast<float>(screen.Width) * 0.62f;
    const float maxHeight = static_cast<float>(screen.Height) * 0.52f;
    const float fitScale = std::min(maxWidth / static_cast<float>(textureSize.Width), maxHeight / static_cast<float>(textureSize.Height));
    const float scale = std::max(0.1f, fitScale * gameOverZoom_);
    const irr::s32 width = static_cast<irr::s32>(static_cast<float>(textureSize.Width) * scale);
    const irr::s32 height = static_cast<irr::s32>(static_cast<float>(textureSize.Height) * scale);
    const irr::s32 left = static_cast<irr::s32>(screen.Width / 2) - width / 2;
    const irr::s32 top = static_cast<irr::s32>(screen.Height / 2) - height / 2 - 40;

    driver_->draw2DImage(gameOverTexture_,
                         irr::core::rect<irr::s32>(left, top, left + width, top + height),
                         irr::core::rect<irr::s32>(0, 0, textureSize.Width, textureSize.Height),
                         nullptr,
                         nullptr,
                         true);
}

void UIManager::ShowGameOver(bool playerWon) {
    if (!guiEnvironment_ || !driver_) return;

    gameOverVisible_ = true;
    playerWon_ = playerWon;
    gameOverZoom_ = 0.1f;
    const std::string imageFile = playerWon_ ? "you-win.png" : "you-lose.png";
    gameOverTexture_ = driver_->getTexture((std::filesystem::path(mediaDir_) / imageFile).string().c_str());

    const irr::core::dimension2du screen = driver_->getScreenSize();
    const irr::s32 buttonWidth = 190;
    const irr::s32 buttonHeight = 42;
    const irr::s32 left = static_cast<irr::s32>(screen.Width / 2) - buttonWidth / 2;
    const irr::s32 top = static_cast<irr::s32>(screen.Height * 0.72f);
    if (!newGameButton_) {
        newGameButton_ = guiEnvironment_->addButton(irr::core::rect<irr::s32>(left, top, left + buttonWidth, top + buttonHeight), nullptr, NewGameButtonId, L"Novo jogo");
    } else {
        newGameButton_->setRelativePosition(irr::core::rect<irr::s32>(left, top, left + buttonWidth, top + buttonHeight));
        newGameButton_->setVisible(true);
        newGameButton_->setEnabled(true);
    }

    if (soundManager_) soundManager_->HandleEvent(playerWon_ ? GameEventSound::PlayerWin : GameEventSound::PlayerLose);
}

void UIManager::HideGameOver() {
    gameOverVisible_ = false;
    gameOverTexture_ = nullptr;
    gameOverZoom_ = 0.1f;
    newGameRequested_ = false;
    if (newGameButton_) {
        newGameButton_->setVisible(false);
        newGameButton_->setEnabled(false);
    }
}

bool UIManager::ConsumeNewGameRequested() {
    const bool requested = newGameRequested_;
    newGameRequested_ = false;
    return requested;
}

} // namespace chessit
