#include "Managers/AIManager.h"

#include "Game/GameEvents.h"
#include "Managers/SoundManager.h"

namespace chessit {

void AIManager::Initialize(BoardManager* boardManager, PieceManager* pieceManager, SoundManager* soundManager) {
    boardManager_ = boardManager;
    pieceManager_ = pieceManager;
    soundManager_ = soundManager;
}

void AIManager::Update() {}


AIDifficulty AIManager::GetDifficulty() const {
    return difficulty_;
}

void AIManager::SetDifficulty(AIDifficulty difficulty) {
    difficulty_ = difficulty;
}

void AIManager::NotifyComputerMove(bool capturedPlayerPiece) {
    if (!soundManager_) return;
    soundManager_->HandleEvent(capturedPlayerPiece ? GameEventSound::ComputerCapture : GameEventSound::PieceMove);
}

} // namespace chessit
