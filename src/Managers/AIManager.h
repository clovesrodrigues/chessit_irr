#pragma once

#include "Game/GameEvents.h"

namespace chessit {
class BoardManager;
class PieceManager;
class SoundManager;

class AIManager {
public:
    void Initialize(BoardManager* boardManager, PieceManager* pieceManager, SoundManager* soundManager = nullptr);
    void Update();
    bool MakeComputerMove();
    void NotifyComputerMove(bool capturedPlayerPiece);
    AIDifficulty GetDifficulty() const;
    void SetDifficulty(AIDifficulty difficulty);

private:
    BoardManager* boardManager_ = nullptr;
    PieceManager* pieceManager_ = nullptr;
    SoundManager* soundManager_ = nullptr;
    AIDifficulty difficulty_ = AIDifficulty::Medium;
};

} // namespace chessit
