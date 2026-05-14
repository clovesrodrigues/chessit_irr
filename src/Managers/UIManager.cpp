#include "Managers/UIManager.h"

namespace chessit {

void UIManager::Initialize(irr::gui::IGUIEnvironment* guiEnvironment, irr::video::IVideoDriver* driver, const std::string& mediaDir) {
    guiEnvironment_ = guiEnvironment;
    (void)driver;
    (void)mediaDir;
    if (guiEnvironment_) {
        guiEnvironment_->addStaticText(L"ChessIt 3D", irr::core::rect<irr::s32>(16, 16, 220, 48), false, true);
    }
}

void UIManager::Update() {}

} // namespace chessit
