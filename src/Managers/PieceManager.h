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
    ChessPiece* GetPieceByNode(const irr::scene::ISceneNode* node) const;
    bool HasAliveKing(PieceColor color) const;
    bool MovePiece(ChessPiece* piece, const std::string& targetSquare, const BoardManager& boardManager, bool* capturedPiece = nullptr);
    static irr::s32 PieceNodeIdBase() { return 0x10000000; }

private:
    struct PieceSpawn {
        std::string name;
        PieceType type;
        PieceColor color;
        std::string asset;
        std::string square;
    };

    static std::vector<PieceSpawn> CreateInitialLayout();
    static irr::core::vector3df ComputeMeshAnchor(const irr::scene::IAnimatedMesh* mesh);
    static irr::core::vector3df ComputeNodePosition(const irr::core::vector3df& squarePosition, const irr::core::vector3df& meshAnchor);
    static std::string JoinPath(const std::string& base, const std::string& file);

    std::vector<std::unique_ptr<ChessPiece>> pieces_;
    std::map<irr::s32, ChessPiece*> piecesByNodeId_;
    BoardState boardState_;
};

} // namespace chessit
