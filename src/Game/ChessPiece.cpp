#include "Game/ChessTypes.h"

namespace chessit {

const char* ToString(PieceType type) {
    switch (type) {
        case PieceType::King: return "King";
        case PieceType::Queen: return "Queen";
        case PieceType::Rook: return "Rook";
        case PieceType::Bishop: return "Bishop";
        case PieceType::Knight: return "Knight";
        case PieceType::Pawn: return "Pawn";
        default: return "Unknown";
    }
}

const char* ToString(PieceColor color) {
    switch (color) {
        case PieceColor::White: return "White";
        case PieceColor::Black: return "Black";
        default: return "None";
    }
}

} // namespace chessit
