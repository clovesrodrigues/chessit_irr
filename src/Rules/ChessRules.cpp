#include "Rules/ChessRules.h"

#include <algorithm>

namespace chessit {

ChessRules::ChessRules(const PieceManager::BoardState* boardState) : boardState_(boardState) {}
void ChessRules::SetBoardState(const PieceManager::BoardState* boardState) { boardState_ = boardState; }

std::vector<Move> ChessRules::GetPossibleMoves(const ChessPiece* piece) const {
    std::vector<Move> moves;
    if (!piece || !piece->alive) return moves;

    switch (piece->type) {
        case PieceType::Rook:
            AddSlidingMoves(piece, {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}, moves);
            break;
        case PieceType::Bishop:
            AddSlidingMoves(piece, {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}}, moves);
            break;
        case PieceType::Queen:
            AddSlidingMoves(piece, {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}}, moves);
            break;
        case PieceType::King:
            for (int df = -1; df <= 1; ++df) for (int dr = -1; dr <= 1; ++dr) if (df || dr) AddStepMove(piece, df, dr, moves);
            break;
        case PieceType::Knight:
            for (const auto& delta : {std::pair<int, int>{1, 2}, {2, 1}, {-1, 2}, {-2, 1}, {1, -2}, {2, -1}, {-1, -2}, {-2, -1}}) {
                AddStepMove(piece, delta.first, delta.second, moves);
            }
            break;
        case PieceType::Pawn: {
            int file = 0, rank = 0;
            if (!ParseSquare(piece->currentSquare, file, rank)) break;
            const int direction = piece->color == PieceColor::White ? 1 : -1;
            const std::string one = MakeSquare(file, rank + direction);
            if (InBounds(file, rank + direction) && IsSquareEmpty(one)) {
                moves.push_back({piece->currentSquare, one, (rank + direction == 8 || rank + direction == 1) ? PieceType::Queen : PieceType::Unknown});
                const int startRank = piece->color == PieceColor::White ? 2 : 7;
                const std::string two = MakeSquare(file, rank + 2 * direction);
                if (piece->firstMove && rank == startRank && IsSquareEmpty(two)) {
                    moves.push_back({piece->currentSquare, two});
                }
            }
            for (int df : {-1, 1}) {
                const std::string target = MakeSquare(file + df, rank + direction);
                if (InBounds(file + df, rank + direction) && IsEnemyPiece(target, piece->color)) {
                    moves.push_back({piece->currentSquare, target, (rank + direction == 8 || rank + direction == 1) ? PieceType::Queen : PieceType::Unknown, true});
                }
            }
            break;
        }
        default:
            break;
    }
    return moves;
}

bool ChessRules::IsSquareEmpty(const std::string& square) const {
    return !boardState_ || boardState_->find(square) == boardState_->end() || !boardState_->at(square) || !boardState_->at(square)->alive;
}

bool ChessRules::IsEnemyPiece(const std::string& square, PieceColor color) const {
    if (!boardState_) return false;
    const auto it = boardState_->find(square);
    return it != boardState_->end() && it->second && it->second->alive && it->second->color != color;
}

bool ChessRules::IsValidMove(const ChessPiece* piece, const std::string& toSquare) const {
    const auto moves = GetPossibleMoves(piece);
    return std::any_of(moves.begin(), moves.end(), [&toSquare](const Move& move) { return move.toSquare == toSquare; });
}

bool ChessRules::IsCheck(PieceColor color) const {
    (void)color;
    return false;
}

bool ChessRules::IsCheckMate(PieceColor color) const {
    return IsCheck(color) && false;
}

void ChessRules::AddSlidingMoves(const ChessPiece* piece, const std::vector<std::pair<int, int>>& directions, std::vector<Move>& moves) const {
    int file = 0, rank = 0;
    if (!ParseSquare(piece->currentSquare, file, rank)) return;
    for (const auto& direction : directions) {
        int nextFile = file + direction.first;
        int nextRank = rank + direction.second;
        while (InBounds(nextFile, nextRank)) {
            const std::string square = MakeSquare(nextFile, nextRank);
            if (IsSquareEmpty(square)) {
                moves.push_back({piece->currentSquare, square});
            } else {
                if (IsEnemyPiece(square, piece->color)) moves.push_back({piece->currentSquare, square, PieceType::Unknown, true});
                break;
            }
            nextFile += direction.first;
            nextRank += direction.second;
        }
    }
}

void ChessRules::AddStepMove(const ChessPiece* piece, int fileDelta, int rankDelta, std::vector<Move>& moves) const {
    int file = 0, rank = 0;
    if (!ParseSquare(piece->currentSquare, file, rank)) return;
    const int nextFile = file + fileDelta;
    const int nextRank = rank + rankDelta;
    if (!InBounds(nextFile, nextRank)) return;
    const std::string square = MakeSquare(nextFile, nextRank);
    if (IsSquareEmpty(square) || IsEnemyPiece(square, piece->color)) {
        moves.push_back({piece->currentSquare, square, PieceType::Unknown, IsEnemyPiece(square, piece->color)});
    }
}

bool ChessRules::ParseSquare(const std::string& square, int& file, int& rank) {
    if (square.size() != 2) return false;
    file = square[0] - 'A' + 1;
    rank = square[1] - '0';
    return InBounds(file, rank);
}

std::string ChessRules::MakeSquare(int file, int rank) {
    return std::string(1, static_cast<char>('A' + file - 1)) + std::to_string(rank);
}

bool ChessRules::InBounds(int file, int rank) {
    return file >= 1 && file <= 8 && rank >= 1 && rank <= 8;
}

} // namespace chessit
