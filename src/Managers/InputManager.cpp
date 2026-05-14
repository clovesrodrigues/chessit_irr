#include "Managers/InputManager.h"

#include "Game/GameEvents.h"
#include "Core/Logger.h"
#include "Managers/AIManager.h"
#include "Managers/BillboardManager.h"
#include "Managers/BoardManager.h"
#include "Managers/SoundManager.h"
#include "Managers/UIManager.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace chessit {

bool InputManager::OnEvent(const irr::SEvent& event) {
    if (uiManager_ && uiManager_->OnEvent(event)) return true;
    if (event.EventType == irr::EET_MOUSE_INPUT_EVENT) {
        const irr::core::position2di currentMousePosition(event.MouseInput.X, event.MouseInput.Y);
        if (event.MouseInput.Event == irr::EMIE_MOUSE_MOVED) {
            mouseDelta_ += currentMousePosition - lastMousePosition_;
        }
        lastMousePosition_ = currentMousePosition;

        switch (event.MouseInput.Event) {
            case irr::EMIE_LMOUSE_PRESSED_DOWN:
                leftMousePressed_ = true;
                break;
            case irr::EMIE_RMOUSE_PRESSED_DOWN:
                rightMouseDown_ = true;
                break;
            case irr::EMIE_RMOUSE_LEFT_UP:
                rightMouseDown_ = false;
                break;
            case irr::EMIE_MMOUSE_PRESSED_DOWN:
                middleMouseDown_ = true;
                break;
            case irr::EMIE_MMOUSE_LEFT_UP:
                middleMouseDown_ = false;
                break;
            case irr::EMIE_MOUSE_WHEEL:
                mouseWheelDelta_ += event.MouseInput.Wheel;
                break;
            default:
                break;
        }
    } else if (event.EventType == irr::EET_KEY_INPUT_EVENT && !event.KeyInput.PressedDown) {
        if (event.KeyInput.Key == irr::KEY_KEY_P) {
            LogActiveCameraPose();
            return true;
        }
    }
    return false;
}

void InputManager::Initialize(irr::IrrlichtDevice* device,
                              irr::scene::ISceneManager* sceneManager,
                              PieceManager* pieceManager,
                              BoardManager* boardManager,
                              BillboardManager* billboardManager,
                              SoundManager* soundManager,
                              UIManager* uiManager,
                              AIManager* aiManager) {
    device_ = device;
    sceneManager_ = sceneManager;
    pieceManager_ = pieceManager;
    boardManager_ = boardManager;
    billboardManager_ = billboardManager;
    soundManager_ = soundManager;
    uiManager_ = uiManager;
    aiManager_ = aiManager;
    if (pieceManager_) rules_.SetBoardState(&pieceManager_->GetBoardState());
}

void InputManager::Update(float deltaSeconds) {
    UpdateCamera(deltaSeconds);
    HandleMouseMove();
    if (leftMousePressed_) {
        HandleLeftClick();
        leftMousePressed_ = false;
    }
}

void InputManager::UpdateCamera(float deltaSeconds) {
    (void)deltaSeconds;
    irr::scene::ICameraSceneNode* camera = sceneManager_ ? sceneManager_->getActiveCamera() : nullptr;
    if (!camera) {
        mouseDelta_ = irr::core::position2di(0, 0);
        mouseWheelDelta_ = 0.0f;
        return;
    }

    irr::core::vector3df position = camera->getPosition();
    irr::core::vector3df target = camera->getTarget();
    irr::core::vector3df direction = target - position;
    const float distance = std::max(0.1f, direction.getLength());
    direction.normalize();

    bool moved = false;
    if (rightMouseDown_ && (mouseDelta_.X != 0 || mouseDelta_.Y != 0)) {
        float yaw = std::atan2(direction.X, direction.Z);
        float pitch = std::asin(std::max(-0.99f, std::min(0.99f, direction.Y)));
        yaw += static_cast<float>(mouseDelta_.X) * 0.005f;
        pitch += static_cast<float>(mouseDelta_.Y) * 0.005f;
        const float pitchLimit = 1.45f;
        pitch = std::max(-pitchLimit, std::min(pitchLimit, pitch));

        const float cosPitch = std::cos(pitch);
        direction = irr::core::vector3df(std::sin(yaw) * cosPitch, std::sin(pitch), std::cos(yaw) * cosPitch);
        direction.normalize();
        target = position + direction * distance;
        moved = true;
    }

    if (middleMouseDown_ && (mouseDelta_.X != 0 || mouseDelta_.Y != 0)) {
        const irr::core::vector3df worldUp(0.0f, 1.0f, 0.0f);
        irr::core::vector3df right = direction.crossProduct(worldUp);
        if (right.getLength() < 0.001f) right = irr::core::vector3df(1.0f, 0.0f, 0.0f);
        right.normalize();
        irr::core::vector3df up = right.crossProduct(direction);
        up.normalize();
        const float panScale = std::max(0.02f, distance * 0.0015f);
        const irr::core::vector3df pan = (right * static_cast<float>(-mouseDelta_.X) + up * static_cast<float>(mouseDelta_.Y)) * panScale;
        position += pan;
        target += pan;
        moved = true;
    }

    if (std::abs(mouseWheelDelta_) > 0.0001f) {
        const float zoomScale = std::max(0.25f, distance * 0.08f);
        const irr::core::vector3df zoom = direction * (mouseWheelDelta_ * zoomScale);
        position += zoom;
        target += zoom;
        moved = true;
    }

    if (moved) {
        camera->setPosition(position);
        camera->setTarget(target);
        cameraMovedThisFrame_ = true;
    } else if (cameraMovedThisFrame_) {
        LogActiveCameraPose();
        cameraMovedThisFrame_ = false;
    }

    mouseDelta_ = irr::core::position2di(0, 0);
    mouseWheelDelta_ = 0.0f;
}

void InputManager::LogActiveCameraPose() const {
    const irr::scene::ICameraSceneNode* camera = sceneManager_ ? sceneManager_->getActiveCamera() : nullptr;
    if (!camera) return;

    const irr::core::vector3df position = camera->getPosition();
    const irr::core::vector3df target = camera->getTarget();
    std::ostringstream message;
    message << std::fixed << std::setprecision(6)
            << "CAMERA " << position.X << ", " << position.Y << ", " << position.Z
            << " TARGET " << target.X << ", " << target.Y << ", " << target.Z
            << " FOVY " << camera->getFOV()
            << " ASPECT" << camera->getAspectRatio();
    Logger::Info(message.str());
}

void InputManager::HandleMouseMove() {
    ChessPiece* piece = PickPiece();
    if (piece != hoveredPiece_) {
        if (hoveredPiece_ && billboardManager_) billboardManager_->HideBillboard("hover_piece");
        hoveredPiece_ = piece;
        if (hoveredPiece_ && hoveredPiece_->node && billboardManager_) {
            irr::core::vector3df pos = hoveredPiece_->node->getPosition();
            pos.Y += 0.85f;
            billboardManager_->ShowBillboard("hover_piece", pos, 1.25f);
            if (soundManager_) soundManager_->HandleEvent(GameEventSound::PieceHover);
        }
    } else if (hoveredPiece_ && hoveredPiece_->node && billboardManager_) {
        irr::core::vector3df pos = hoveredPiece_->node->getPosition();
        pos.Y += 0.85f;
        billboardManager_->ShowBillboard("hover_piece", pos, 1.25f);
    }

    const std::string square = PickBoardSquare();
    const bool squareIsValid = !square.empty() && IsValidTargetSquare(square);
    if ((!squareIsValid && !hoveredSquare_.empty()) || (squareIsValid && square != hoveredSquare_)) {
        if (!hoveredSquare_.empty() && billboardManager_) billboardManager_->HideBillboard("hover_square");
        hoveredSquare_.clear();
    }
    if (squareIsValid && square != hoveredSquare_) {
        hoveredSquare_ = square;
        if (const irr::core::vector3df* squarePos = boardManager_->GetSquarePosition(square)) {
            irr::core::vector3df pos = *squarePos;
            pos.Y += 0.12f;
            if (billboardManager_) billboardManager_->ShowBillboard("hover_square", pos, 1.05f);
            if (soundManager_) soundManager_->HandleEvent(GameEventSound::SquareHover);
        }
    } else if (squareIsValid && billboardManager_) {
        if (const irr::core::vector3df* squarePos = boardManager_->GetSquarePosition(square)) {
            irr::core::vector3df pos = *squarePos;
            pos.Y += 0.12f;
            billboardManager_->ShowBillboard("hover_square", pos, 1.05f);
        }
    }
}

void InputManager::HandleLeftClick() {
    ChessPiece* piece = PickPiece();
    if (piece && piece->color == PieceColor::White) {
        SelectPiece(piece);
        return;
    }

    const std::string targetSquare = PickBoardSquare();
    if (selectedPiece_ && IsValidTargetSquare(targetSquare)) {
        bool captured = false;
        if (pieceManager_->MovePiece(selectedPiece_, targetSquare, *boardManager_, &captured)) {
            if (soundManager_) soundManager_->HandleEvent(captured ? GameEventSound::PlayerCapture : GameEventSound::PieceMove);
            ClearSelection();
            if (aiManager_) aiManager_->MakeComputerMove();
            return;
        }
        ClearSelection();
        return;
    }

    if (!piece) ClearSelection();
}

ChessPiece* InputManager::PickPiece() const {
    if (!sceneManager_ || !pieceManager_) return nullptr;
    irr::scene::ISceneCollisionManager* collision = sceneManager_->getSceneCollisionManager();
    if (!collision) return nullptr;
    irr::scene::ISceneNode* node = collision->getSceneNodeFromScreenCoordinatesBB(lastMousePosition_, PieceManager::PieceNodeIdBase(), true);
    return pieceManager_->GetPieceByNode(node);
}

std::string InputManager::PickBoardSquare() const {
    if (!sceneManager_ || !boardManager_) return {};
    irr::scene::ISceneCollisionManager* collision = sceneManager_->getSceneCollisionManager();
    irr::scene::ICameraSceneNode* camera = sceneManager_->getActiveCamera();
    if (!collision || !camera) return {};

    const irr::core::line3df ray = collision->getRayFromScreenCoordinates(lastMousePosition_, camera);
    const float boardY = boardManager_->GetPositions().boardPosition.Y + 0.05f;
    const float dy = ray.end.Y - ray.start.Y;
    if (std::abs(dy) < 0.0001f) return {};
    const float t = (boardY - ray.start.Y) / dy;
    if (t < 0.0f || t > 1.0f) return {};
    const irr::core::vector3df point = ray.start + (ray.end - ray.start) * t;
    return boardManager_->FindNearestSquare(point);
}

void InputManager::SelectPiece(ChessPiece* piece) {
    selectedPiece_ = piece;
    validMoves_ = rules_.GetPossibleMoves(selectedPiece_);
    ShowValidMoveBillboards();
}

void InputManager::ClearSelection() {
    selectedPiece_ = nullptr;
    validMoves_.clear();
    hoveredSquare_.clear();
    if (billboardManager_) {
        billboardManager_->HideGroupWithPrefix("valid_");
        billboardManager_->HideBillboard("hover_square");
    }
}

void InputManager::ShowValidMoveBillboards() {
    if (!billboardManager_ || !boardManager_) return;
    billboardManager_->HideGroupWithPrefix("valid_");
    for (const Move& move : validMoves_) {
        if (const irr::core::vector3df* squarePos = boardManager_->GetSquarePosition(move.toSquare)) {
            irr::core::vector3df pos = *squarePos;
            pos.Y += 0.10f;
            billboardManager_->ShowBillboard("valid_" + move.toSquare, pos, 0.82f);
        }
    }
}

bool InputManager::IsValidTargetSquare(const std::string& square) const {
    return std::any_of(validMoves_.begin(), validMoves_.end(), [&square](const Move& move) { return move.toSquare == square; });
}

} // namespace chessit
