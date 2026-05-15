#pragma once

#include "Game/GameEvents.h"

#include <random>
#include <string>

namespace chessit {
class BoardManager;
class ONNXAIManager;
class PieceManager;
class SoundManager;

class AIManager {
public:
    void Initialize(BoardManager* boardManager,
                    PieceManager* pieceManager,
                    SoundManager* soundManager = nullptr,
                    ONNXAIManager* onnxAIManager = nullptr);
    void Update();
    bool MakeComputerMove();
    void NotifyComputerMove(bool capturedPlayerPiece);
    AIDifficulty GetDifficulty() const;
    void SetDifficulty(AIDifficulty difficulty);
    bool IsNeuralAIAvailable() const;
    bool WasLastMoveNeural() const { return lastMoveUsedNeural_; }
    std::string GetAIStatusText() const;
    std::string GetAIModeText() const;
    std::string GetLastMoveSourceText() const;

private:
    BoardManager* boardManager_ = nullptr;
    PieceManager* pieceManager_ = nullptr;
    SoundManager* soundManager_ = nullptr;
    ONNXAIManager* onnxAIManager_ = nullptr;
    bool lastMoveUsedNeural_ = false;
    bool hasComputerMoved_ = false;
    std::mt19937 rng_{std::random_device{}()};
    AIDifficulty difficulty_ = AIDifficulty::Medium;
};

} // namespace chessit
