#include "Managers/AIManager.h"

#include "Game/GameEvents.h"
#include "Managers/BoardManager.h"
#include "Managers/PieceManager.h"
#include "Managers/SoundManager.h"
#include "Rules/ChessRules.h"

#include <limits>
#include <vector>

namespace chessit {

void AIManager::Initialize(BoardManager* boardManager, PieceManager* pieceManager, SoundManager* soundManager) {
    boardManager_ = boardManager;
    pieceManager_ = pieceManager;
    soundManager_ = soundManager;
}

void AIManager::Update() {}

namespace {

int PieceValue(PieceType type) {
    switch (type) {
        case PieceType::Pawn: return 1;
        case PieceType::Knight: return 3;
        case PieceType::Bishop: return 3;
        case PieceType::Rook: return 5;
        case PieceType::Queen: return 9;
        case PieceType::King: return 100;
        default: return 0;
    }
}

} // namespace

bool AIManager::MakeComputerMove() {
    if (!boardManager_ || !pieceManager_) return false;

    ChessRules rules(&pieceManager_->GetBoardState());
    ChessPiece* selectedPiece = nullptr;
    Move selectedMove;
    int bestCaptureValue = std::numeric_limits<int>::min();

    for (const auto& piecePtr : pieceManager_->GetPieces()) {
        ChessPiece* piece = piecePtr.get();
        if (!piece || !piece->alive || piece->color != PieceColor::Black) continue;

        const std::vector<Move> moves = rules.GetPossibleMoves(piece);
        for (const Move& move : moves) {
            const ChessPiece* target = pieceManager_->GetPieceAt(move.toSquare);
            const int captureValue = target ? PieceValue(target->type) : 0;
            if (!selectedPiece || captureValue > bestCaptureValue) {
                selectedPiece = piece;
                selectedMove = move;
                bestCaptureValue = captureValue;
            }
        }
    }

    if (!selectedPiece) return false;

    bool capturedPlayerPiece = false;
    if (!pieceManager_->MovePiece(selectedPiece, selectedMove.toSquare, *boardManager_, &capturedPlayerPiece)) return false;
    NotifyComputerMove(capturedPlayerPiece);
    return true;
}

void AIManager::NotifyComputerMove(bool capturedPlayerPiece) {
    if (!soundManager_) return;
    soundManager_->HandleEvent(capturedPlayerPiece ? GameEventSound::ComputerCapture : GameEventSound::PieceMove);
}

AIDifficulty AIManager::GetDifficulty() const {
    return difficulty_;
}

void AIManager::SetDifficulty(AIDifficulty difficulty) {
    difficulty_ = difficulty;
}

} // namespace chessit
