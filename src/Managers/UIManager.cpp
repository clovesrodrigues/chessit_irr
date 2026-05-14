#include "Managers/UIManager.h"

#include <filesystem>

namespace chessit {

void UIManager::Initialize(irr::gui::IGUIEnvironment* guiEnvironment, irr::video::IVideoDriver* driver, const std::string& mediaDir, SoundManager* soundManager, AIManager* aiManager) {
    guiEnvironment_ = guiEnvironment;
    (void)driver;
    if (guiEnvironment_) {
        guiEnvironment_->addStaticText(L"ChessIt 3D", irr::core::rect<irr::s32>(16, 16, 220, 48), false, true);
        const std::string configPath = (std::filesystem::path(mediaDir) / "config.ini").string();
        audioMenu_.Initialize(guiEnvironment_, soundManager, aiManager, configPath);
    }
}

bool UIManager::OnEvent(const irr::SEvent& event) {
    return audioMenu_.OnEvent(event);
}

void UIManager::Update() {}

} // namespace chessit
