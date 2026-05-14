#pragma once

#include "Game/ChessTypes.h"
#include "Managers/BoardManager.h"

#include <irrlicht.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace chessit {

class PieceManager {
public:
    using BoardState = std::map<std::string, ChessPiece*>;

    bool LoadInitialPieces(irr::scene::ISceneManager* sceneManager, const BoardManager& boardManager, const std::string& mediaDir);
    const std::vector<std::unique_ptr<ChessPiece>>& GetPieces() const { return pieces_; }
    const BoardState& GetBoardState() const { return boardState_; }
    ChessPiece* GetPieceAt(const std::string& square) const;

private:
    struct PieceSpawn {
        std::string name;
        PieceType type;
        PieceColor color;
        std::string asset;
        std::string square;
    };

    static std::vector<PieceSpawn> CreateInitialLayout();
    static std::string JoinPath(const std::string& base, const std::string& file);

    std::vector<std::unique_ptr<ChessPiece>> pieces_;
    BoardState boardState_;
};

} // namespace chessit
