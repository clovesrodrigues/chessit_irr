#pragma once

#include "Game/ChessTypes.h"
#include "Managers/PieceManager.h"
#include "Rules/ChessRules.h"

#include <irrlicht.h>
#include <string>
#include <vector>

namespace chessit {
class AIManager;
class BillboardManager;
class BoardManager;
class SoundManager;
class UIManager;

class InputManager : public irr::IEventReceiver {
public:
    bool OnEvent(const irr::SEvent& event) override;
    void Initialize(irr::IrrlichtDevice* device,
                    irr::scene::ISceneManager* sceneManager,
                    PieceManager* pieceManager,
                    BoardManager* boardManager,
                    BillboardManager* billboardManager,
                    SoundManager* soundManager,
                    UIManager* uiManager,
                    AIManager* aiManager = nullptr);
    void Update(float deltaSeconds);

private:
    void HandleMouseMove();
    void UpdateCamera(float deltaSeconds);
    void LogActiveCameraPose() const;
    void HandleLeftClick();
    ChessPiece* PickPiece() const;
    std::string PickBoardSquare() const;
    void SelectPiece(ChessPiece* piece);
    void ClearSelection();
    void ShowValidMoveBillboards();
    bool IsValidTargetSquare(const std::string& square) const;

    irr::IrrlichtDevice* device_ = nullptr;
    irr::scene::ISceneManager* sceneManager_ = nullptr;
    PieceManager* pieceManager_ = nullptr;
    BoardManager* boardManager_ = nullptr;
    BillboardManager* billboardManager_ = nullptr;
    SoundManager* soundManager_ = nullptr;
    UIManager* uiManager_ = nullptr;
    AIManager* aiManager_ = nullptr;
    irr::core::position2di lastMousePosition_{0, 0};
    bool leftMousePressed_ = false;
    bool rightMouseDown_ = false;
    bool middleMouseDown_ = false;
    bool cameraMovedThisFrame_ = false;
    float mouseWheelDelta_ = 0.0f;
    irr::core::position2di mouseDelta_{0, 0};
    ChessPiece* selectedPiece_ = nullptr;
    ChessPiece* hoveredPiece_ = nullptr;
    std::string hoveredSquare_;
    std::vector<Move> validMoves_;
    ChessRules rules_;
};

} // namespace chessit
