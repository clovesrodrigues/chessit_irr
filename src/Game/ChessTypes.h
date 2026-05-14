#pragma once

#include <irrlicht.h>
#include <string>

namespace chessit {

enum class PieceType { King, Queen, Rook, Bishop, Knight, Pawn, Unknown };
enum class PieceColor { White, Black, None };

struct Move {
    std::string fromSquare;
    std::string toSquare;
    PieceType promotion = PieceType::Unknown;
    bool isCapture = false;
    bool isCastle = false;
};

struct ChessPiece {
    std::string name;
    PieceType type = PieceType::Unknown;
    PieceColor color = PieceColor::None;
    irr::core::vector3df position;
    irr::core::vector3df meshAnchor;
    irr::scene::ISceneNode* node = nullptr;
    std::string currentSquare;
    bool alive = true;
    bool firstMove = true;
};

const char* ToString(PieceType type);
const char* ToString(PieceColor color);

} // namespace chessit
