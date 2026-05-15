#include "Managers/AIManager.h"

#include "Game/GameEvents.h"
#include "Managers/BoardManager.h"
#include "Managers/PieceManager.h"
#include "Managers/SoundManager.h"
#include "Managers/ONNXAIManager.h"
#include "Rules/ChessRules.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cctype>
#include <limits>
#include <random>
#include <string>
#include <utility>
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

struct SquareCoord {
    int file = 0;
    int rank = 0;
};

bool ParseSquare(const std::string& square, SquareCoord& coord) {
    if (square.size() < 2) return false;
    const char fileChar = static_cast<char>(std::toupper(static_cast<unsigned char>(square[0])));
    const char rankChar = square[1];
    if (fileChar < 'A' || fileChar > 'H' || rankChar < '1' || rankChar > '8') return false;
    coord.file = fileChar - 'A' + 1;
    coord.rank = rankChar - '0';
    return true;
}

bool InBounds(int file, int rank) {
    return file >= 1 && file <= 8 && rank >= 1 && rank <= 8;
}

bool SameSquare(const std::string& lhs, const std::string& rhs) {
    SquareCoord a, b;
    return ParseSquare(lhs, a) && ParseSquare(rhs, b) && a.file == b.file && a.rank == b.rank;
}

std::size_t NeuralTopRange(AIDifficulty difficulty) {
    switch (difficulty) {
        case AIDifficulty::Easy: return 6;
        case AIDifficulty::Medium: return 4;
        case AIDifficulty::Hard: return 3;
        case AIDifficulty::Expert: return 2;
        default: return 3;
    }
}

int AggressiveMoveChance(AIDifficulty difficulty) {
    switch (difficulty) {
        case AIDifficulty::Easy: return 20;
        case AIDifficulty::Medium: return 28;
        case AIDifficulty::Hard: return 35;
        case AIDifficulty::Expert: return 42;
        default: return 30;
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

const ChessPiece* PieceAtAfterMove(const PieceManager::BoardState& boardState, const Move& move, const std::string& square) {
    if (SameSquare(square, move.fromSquare)) return nullptr;
    if (SameSquare(square, move.toSquare)) {
        const auto fromIt = boardState.find(move.fromSquare);
        return fromIt == boardState.end() ? nullptr : fromIt->second;
    }
    const auto it = boardState.find(square);
    if (it == boardState.end() || !it->second || !it->second->alive) return nullptr;
    if (SameSquare(square, move.toSquare)) return nullptr;
    return it->second;
}

bool OccupiedAfterMove(const PieceManager::BoardState& boardState, const Move& move, const std::string& square) {
    return PieceAtAfterMove(boardState, move, square) != nullptr;
}

std::string MakeSquare(int file, int rank) {
    return std::string(1, static_cast<char>('A' + file - 1)) + std::to_string(rank);
}

bool SlidingPieceAttacksAfterMove(const PieceManager::BoardState& boardState,
                                  const Move& move,
                                  SquareCoord from,
                                  SquareCoord target,
                                  const std::vector<std::pair<int, int>>& directions) {
    for (const auto& direction : directions) {
        int file = from.file + direction.first;
        int rank = from.rank + direction.second;
        while (InBounds(file, rank)) {
            if (file == target.file && rank == target.rank) return true;
            const std::string square = MakeSquare(file, rank);
            if (OccupiedAfterMove(boardState, move, square)) break;
            file += direction.first;
            rank += direction.second;
        }
    }
    return false;
}

bool PieceAttacksSquareAfterMove(const PieceManager::BoardState& boardState,
                                 const ChessPiece* piece,
                                 const Move& move,
                                 const std::string& targetSquare) {
    if (!piece || !piece->alive) return false;
    if (piece->color == PieceColor::Black && SameSquare(piece->currentSquare, move.fromSquare)) return false;
    if (SameSquare(piece->currentSquare, move.toSquare)) return false;

    SquareCoord from, target;
    if (!ParseSquare(piece->currentSquare, from) || !ParseSquare(targetSquare, target)) return false;

    const int df = target.file - from.file;
    const int dr = target.rank - from.rank;
    switch (piece->type) {
        case PieceType::Pawn: {
            const int direction = piece->color == PieceColor::White ? 1 : -1;
            return dr == direction && (df == 1 || df == -1);
        }
        case PieceType::Knight:
            return (std::abs(df) == 1 && std::abs(dr) == 2) || (std::abs(df) == 2 && std::abs(dr) == 1);
        case PieceType::Bishop:
            return SlidingPieceAttacksAfterMove(boardState, move, from, target, {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}});
        case PieceType::Rook:
            return SlidingPieceAttacksAfterMove(boardState, move, from, target, {{1, 0}, {-1, 0}, {0, 1}, {0, -1}});
        case PieceType::Queen:
            return SlidingPieceAttacksAfterMove(boardState, move, from, target, {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}});
        case PieceType::King:
            return std::abs(df) <= 1 && std::abs(dr) <= 1;
        default:
            return false;
    }
}

std::string FindKingSquareAfterMove(const PieceManager::BoardState& boardState, const Move& move, PieceColor color) {
    const auto fromIt = boardState.find(move.fromSquare);
    if (fromIt != boardState.end() && fromIt->second && fromIt->second->type == PieceType::King && fromIt->second->color == color) {
        return move.toSquare;
    }
    for (const auto& entry : boardState) {
        const ChessPiece* piece = entry.second;
        if (piece && piece->alive && piece->type == PieceType::King && piece->color == color) return entry.first;
    }
    return {};
}

bool LeavesKingAttacked(const PieceManager::BoardState& boardState, const Move& move, PieceColor color) {
    const std::string kingSquare = FindKingSquareAfterMove(boardState, move, color);
    if (kingSquare.empty()) return false;
    const PieceColor attackerColor = color == PieceColor::White ? PieceColor::Black : PieceColor::White;
    for (const auto& entry : boardState) {
        const ChessPiece* piece = entry.second;
        if (!piece || !piece->alive || piece->color != attackerColor) continue;
        if (PieceAttacksSquareAfterMove(boardState, piece, move, kingSquare)) return true;
    }
    return false;
}

int MoveAggressionScore(const PieceManager& pieceManager, const Move& move) {
    int score = 0;
    const ChessPiece* target = pieceManager.GetPieceAt(move.toSquare);
    if (target) score += PieceValue(target->type) * 120;
    if (move.promotion == PieceType::Queen) score += 450;

    SquareCoord from, to;
    if (ParseSquare(move.fromSquare, from) && ParseSquare(move.toSquare, to)) {
        score += (from.rank - to.rank) * 8; // black advances toward rank 1
        const int centerFile = std::min(to.file - 1, 8 - to.file);
        const int centerRank = std::min(to.rank - 1, 8 - to.rank);
        score += (centerFile + centerRank) * 2;
    }
    return score;
}

} // namespace

bool AIManager::MakeComputerMove() {
    lastMoveUsedNeural_ = false;
    if (!boardManager_ || !pieceManager_) return false;

    ChessRules rules(&pieceManager_->GetBoardState());
    ChessPiece* selectedPiece = nullptr;
    Move selectedMove;
    int bestAggressionScore = std::numeric_limits<int>::min();
    std::vector<Move> legalMoves;

    for (const auto& piecePtr : pieceManager_->GetPieces()) {
        ChessPiece* piece = piecePtr.get();
        if (!piece || !piece->alive || piece->color != PieceColor::Black) continue;

        const std::vector<Move> moves = rules.GetPossibleMoves(piece);
        legalMoves.insert(legalMoves.end(), moves.begin(), moves.end());
        for (const Move& move : moves) {
            if (LeavesKingAttacked(pieceManager_->GetBoardState(), move, PieceColor::Black)) continue;
            legalMoves.push_back(move);

            const int aggressionScore = MoveAggressionScore(*pieceManager_, move);
            if (!selectedPiece || aggressionScore > bestAggressionScore) {
                selectedPiece = piece;
                selectedMove = move;
                bestAggressionScore = aggressionScore;
            }
        }
    }

    if (onnxAIManager_ && onnxAIManager_->IsModelLoaded() && !legalMoves.empty()) {
        const std::size_t topRange = NeuralTopRange(difficulty_);
        std::uniform_int_distribution<std::size_t> offsetDistribution(0, topRange > 0 ? topRange - 1 : 0);
        std::uniform_int_distribution<int> chanceDistribution(1, 100);
        const bool preferAggressiveMove = chanceDistribution(rng_) <= AggressiveMoveChance(difficulty_);
        const std::size_t candidateOffset = preferAggressiveMove ? 0 : offsetDistribution(rng_);

        const Move neuralMove = onnxAIManager_->PredictMove(
            pieceManager_->GetBoardState(),
            legalMoves,
            PieceColor::Black,
            candidateOffset);
        if (!preferAggressiveMove && !neuralMove.fromSquare.empty() && !neuralMove.toSquare.empty()) {
            ChessPiece* neuralPiece = pieceManager_->GetPieceAt(neuralMove.fromSquare);
            if (neuralPiece && neuralPiece->alive && neuralPiece->color == PieceColor::Black) {
                selectedPiece = neuralPiece;
                selectedMove = neuralMove;
                lastMoveUsedNeural_ = true;
            }
        } else if (!selectedMove.fromSquare.empty() && !selectedMove.toSquare.empty()) {
            lastMoveUsedNeural_ = false;
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
            case AIDifficulty::Easy: mode += " / variavel"; break;
            case AIDifficulty::Medium: mode += " / variavel"; break;
            case AIDifficulty::Hard: mode += " / agressiva"; break;
            case AIDifficulty::Expert: mode += " / agressiva"; break;
        }
    }
    return mode;
}

std::string AIManager::GetLastMoveSourceText() const {
    if (!hasComputerMoved_) return "Aguardando jogada";
    return lastMoveUsedNeural_ ? "ONNX usado" : "Heuristica usada";
}

} // namespace chessit
