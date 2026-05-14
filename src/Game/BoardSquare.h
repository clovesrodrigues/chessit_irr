#pragma once

#include <irrlicht.h>

#include <array>
#include <cstddef>
#include <string>
#include <unordered_map>

namespace chessit {

enum class BoardSquare {
    A1, A2, A3, A4, A5, A6, A7, A8,
    B1, B2, B3, B4, B5, B6, B7, B8,
    C1, C2, C3, C4, C5, C6, C7, C8,
    D1, D2, D3, D4, D5, D6, D7, D8,
    E1, E2, E3, E4, E5, E6, E7, E8,
    F1, F2, F3, F4, F5, F6, F7, F8,
    G1, G2, G3, G4, G5, G6, G7, G8,
    H1, H2, H3, H4, H5, H6, H7, H8,
};

struct BoardSquarePosition {
    BoardSquare square;
    const char* code;
    irr::core::vector3df position;
};

const std::array<BoardSquarePosition, 64>& GetBoardSquareCatalog();
std::unordered_map<std::string, irr::core::vector3df> CreateBoardSquarePositionMap();
const irr::core::vector3df* GetCatalogPosition(BoardSquare square);
const char* ToString(BoardSquare square);

} // namespace chessit
