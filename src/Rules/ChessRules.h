#pragma once

#include "Game/ChessTypes.h"
#include "Managers/PieceManager.h"

#include <string>
#include <vector>

namespace chessit {

class ChessRules {
public:
    explicit ChessRules(const PieceManager::BoardState* boardState = nullptr);

    void SetBoardState(const PieceManager::BoardState* boardState);
    std::vector<Move> GetPossibleMoves(const ChessPiece* piece) const;
    bool IsSquareEmpty(const std::string& square) const;
    bool IsEnemyPiece(const std::string& square, PieceColor color) const;
    bool IsValidMove(const ChessPiece* piece, const std::string& toSquare) const;
    bool IsCheck(PieceColor color) const;
    bool IsCheckMate(PieceColor color) const;

private:
    void AddSlidingMoves(const ChessPiece* piece, const std::vector<std::pair<int, int>>& directions, std::vector<Move>& moves) const;
    void AddStepMove(const ChessPiece* piece, int fileDelta, int rankDelta, std::vector<Move>& moves) const;
    static bool ParseSquare(const std::string& square, int& file, int& rank);
    static std::string MakeSquare(int file, int rank);
    static bool InBounds(int file, int rank);

    const PieceManager::BoardState* boardState_ = nullptr;
};

} // namespace chessit
