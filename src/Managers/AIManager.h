#pragma once

namespace chessit {
class BoardManager;
class PieceManager;

class AIManager {
public:
    void Initialize(BoardManager* boardManager, PieceManager* pieceManager);
    void Update();

private:
    BoardManager* boardManager_ = nullptr;
    PieceManager* pieceManager_ = nullptr;
};

} // namespace chessit
