#include "Managers/AIManager.h"

namespace chessit {

void AIManager::Initialize(BoardManager* boardManager, PieceManager* pieceManager) {
    boardManager_ = boardManager;
    pieceManager_ = pieceManager;
}

void AIManager::Update() {}

} // namespace chessit
