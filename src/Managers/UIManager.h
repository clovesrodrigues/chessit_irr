#pragma once

#include <irrlicht.h>
#include <string>

namespace chessit {

class UIManager {
public:
    void Initialize(irr::gui::IGUIEnvironment* guiEnvironment, irr::video::IVideoDriver* driver, const std::string& mediaDir);
    void Update();

private:
    irr::gui::IGUIEnvironment* guiEnvironment_ = nullptr;
};

} // namespace chessit
