#pragma once

#include "Managers/PieceManager.h"

#include <irrlicht.h>

namespace chessit {

class InputManager : public irr::IEventReceiver {
public:
    bool OnEvent(const irr::SEvent& event) override;
    void Initialize(irr::IrrlichtDevice* device, irr::scene::ISceneManager* sceneManager, PieceManager* pieceManager);
    void Update();

private:
    irr::IrrlichtDevice* device_ = nullptr;
    irr::scene::ISceneManager* sceneManager_ = nullptr;
    PieceManager* pieceManager_ = nullptr;
    irr::core::position2di lastMousePosition_{0, 0};
    bool leftMousePressed_ = false;
};

} // namespace chessit
