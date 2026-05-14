#include "Managers/InputManager.h"

namespace chessit {

bool InputManager::OnEvent(const irr::SEvent& event) {
    if (event.EventType == irr::EET_MOUSE_INPUT_EVENT) {
        lastMousePosition_ = irr::core::position2di(event.MouseInput.X, event.MouseInput.Y);
        leftMousePressed_ = event.MouseInput.Event == irr::EMIE_LMOUSE_PRESSED_DOWN;
    }
    return false;
}

void InputManager::Initialize(irr::IrrlichtDevice* device, irr::scene::ISceneManager* sceneManager, PieceManager* pieceManager) {
    device_ = device;
    sceneManager_ = sceneManager;
    pieceManager_ = pieceManager;
    (void)device_;
    (void)sceneManager_;
    (void)pieceManager_;
}

void InputManager::Update() {
    if (leftMousePressed_) {
        leftMousePressed_ = false;
    }
}

} // namespace chessit
