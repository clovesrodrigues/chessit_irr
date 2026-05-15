#include "Managers/AIManager.h"

#include "Game/GameEvents.h"
#include "Managers/BoardManager.h"
#include "Managers/PieceManager.h"
#include "Managers/SoundManager.h"
#include "Managers/ONNXAIManager.h"
#include "Rules/ChessRules.h"

#include <cstddef>
#include <limits>
#include <string>
#include <vector>

namespace chessit {

void AIManager::Initialize(BoardManager* boardManager,
                           PieceManager* pieceManager,
                           SoundManager* soundManager,
                           ONNXAIManager* onnxAIManager) {
    boardManager_ = boardManager;
    pieceManager_ = pieceManager;
    soundManager_ = soundManager;
    onnxAIManager_ = onnxAIManager;
    lastMoveUsedNeural_ = false;
    hasComputerMoved_ = false;
}

void AIManager::Update() {}

namespace {

std::size_t NeuralCandidateOffset(AIDifficulty difficulty) {
    switch (difficulty) {
        case AIDifficulty::Easy: return 3;
        case AIDifficulty::Medium: return 1;
        case AIDifficulty::Hard:
        case AIDifficulty::Expert:
        default: return 0;
    }
}

const char* DifficultyLabel(AIDifficulty difficulty) {
    switch (difficulty) {
        case AIDifficulty::Easy: return "Easy";
        case AIDifficulty::Medium: return "Medium";
        case AIDifficulty::Hard: return "Hard";
        case AIDifficulty::Expert: return "Expert";
        default: return "Unknown";
    }
}

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
    lastMoveUsedNeural_ = false;
    if (!boardManager_ || !pieceManager_) return false;

    ChessRules rules(&pieceManager_->GetBoardState());
    ChessPiece* selectedPiece = nullptr;
    Move selectedMove;
    int bestCaptureValue = std::numeric_limits<int>::min();
    std::vector<Move> legalMoves;

    for (const auto& piecePtr : pieceManager_->GetPieces()) {
        ChessPiece* piece = piecePtr.get();
        if (!piece || !piece->alive || piece->color != PieceColor::Black) continue;

        const std::vector<Move> moves = rules.GetPossibleMoves(piece);
        legalMoves.insert(legalMoves.end(), moves.begin(), moves.end());
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

    if (onnxAIManager_ && onnxAIManager_->IsModelLoaded()) {
        const Move neuralMove = onnxAIManager_->PredictMove(
            pieceManager_->GetBoardState(),
            legalMoves,
            PieceColor::Black,
            NeuralCandidateOffset(difficulty_));
        if (!neuralMove.fromSquare.empty() && !neuralMove.toSquare.empty()) {
            ChessPiece* neuralPiece = pieceManager_->GetPieceAt(neuralMove.fromSquare);
            if (neuralPiece && neuralPiece->alive && neuralPiece->color == PieceColor::Black) {
                selectedPiece = neuralPiece;
                selectedMove = neuralMove;
                lastMoveUsedNeural_ = true;
            }
        }
    }

    if (!selectedPiece) return false;

    bool capturedPlayerPiece = false;
    if (!pieceManager_->MovePiece(selectedPiece, selectedMove.toSquare, *boardManager_, &capturedPlayerPiece)) return false;
    hasComputerMoved_ = true;
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

bool AIManager::IsNeuralAIAvailable() const {
    return onnxAIManager_ && onnxAIManager_->IsModelLoaded();
}

std::string AIManager::GetAIStatusText() const {
    if (IsNeuralAIAvailable()) return "ONNX ATIVO";
    if (onnxAIManager_ && !onnxAIManager_->GetStatusMessage().empty()) return "ONNX OFF - fallback";
    return "IA classica";
}

std::string AIManager::GetAIModeText() const {
    std::string mode = IsNeuralAIAvailable() ? "ONNX" : "Classica";
    mode += " / ";
    mode += DifficultyLabel(difficulty_);
    if (IsNeuralAIAvailable()) {
        switch (difficulty_) {
            case AIDifficulty::Easy: mode += " / 4a opcao"; break;
            case AIDifficulty::Medium: mode += " / 2a opcao"; break;
            case AIDifficulty::Hard: mode += " / melhor"; break;
            case AIDifficulty::Expert: mode += " / melhor"; break;
        }
    }
    return mode;
}

std::string AIManager::GetLastMoveSourceText() const {
    if (!hasComputerMoved_) return "Aguardando jogada";
    return lastMoveUsedNeural_ ? "ONNX usado" : "Fallback usado";
}

} // namespace chessit
