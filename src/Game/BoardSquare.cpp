#include "Game/BoardSquare.h"

namespace chessit {

const std::array<BoardSquarePosition, 64>& GetBoardSquareCatalog() {
    static const std::array<BoardSquarePosition, 64> catalog = {{
        BoardSquarePosition{BoardSquare::A1, "A1", irr::core::vector3df(-4.129540f, 4.900000f, -6.499246f)},
        BoardSquarePosition{BoardSquare::A2, "A2", irr::core::vector3df(-4.129540f, 4.900000f, -5.219247f)},
        BoardSquarePosition{BoardSquare::A3, "A3", irr::core::vector3df(-4.129540f, 4.900000f, -3.939248f)},
        BoardSquarePosition{BoardSquare::A4, "A4", irr::core::vector3df(-4.129540f, 4.900000f, -2.723248f)},
        BoardSquarePosition{BoardSquare::A5, "A5", irr::core::vector3df(-4.129540f, 4.900000f, -1.443249f)},
        BoardSquarePosition{BoardSquare::A6, "A6", irr::core::vector3df(-4.129540f, 4.900000f, -0.227248f)},
        BoardSquarePosition{BoardSquare::A7, "A7", irr::core::vector3df(-4.129540f, 4.900000f, 1.052752f)},
        BoardSquarePosition{BoardSquare::A8, "A8", irr::core::vector3df(-4.129540f, 4.900000f, 2.332752f)},
        BoardSquarePosition{BoardSquare::B1, "B1", irr::core::vector3df(-2.813484f, 4.900000f, -6.499248f)},
        BoardSquarePosition{BoardSquare::B2, "B2", irr::core::vector3df(-2.813484f, 4.900000f, -5.155248f)},
        BoardSquarePosition{BoardSquare::B3, "B3", irr::core::vector3df(-2.813484f, 4.900000f, -3.939248f)},
        BoardSquarePosition{BoardSquare::B4, "B4", irr::core::vector3df(-2.813484f, 4.900000f, -2.723248f)},
        BoardSquarePosition{BoardSquare::B5, "B5", irr::core::vector3df(-2.813484f, 4.900000f, -1.443249f)},
        BoardSquarePosition{BoardSquare::B6, "B6", irr::core::vector3df(-2.813484f, 4.900000f, -0.227249f)},
        BoardSquarePosition{BoardSquare::B7, "B7", irr::core::vector3df(-2.813484f, 4.900000f, 1.052752f)},
        BoardSquarePosition{BoardSquare::B8, "B8", irr::core::vector3df(-2.813484f, 4.900000f, 2.332752f)},
        BoardSquarePosition{BoardSquare::C1, "C1", irr::core::vector3df(-1.597479f, 4.900000f, -6.499248f)},
        BoardSquarePosition{BoardSquare::C2, "C2", irr::core::vector3df(-1.597479f, 4.900000f, -5.219248f)},
        BoardSquarePosition{BoardSquare::C3, "C3", irr::core::vector3df(-1.597479f, 4.900000f, -3.979852f)},
        BoardSquarePosition{BoardSquare::C4, "C4", irr::core::vector3df(-1.597479f, 4.900000f, -2.699853f)},
        BoardSquarePosition{BoardSquare::C5, "C5", irr::core::vector3df(-1.597479f, 4.900000f, -1.445385f)},
        BoardSquarePosition{BoardSquare::C6, "C6", irr::core::vector3df(-1.597479f, 4.900000f, -0.165385f)},
        BoardSquarePosition{BoardSquare::C7, "C7", irr::core::vector3df(-1.597479f, 4.900000f, 1.050615f)},
        BoardSquarePosition{BoardSquare::C8, "C8", irr::core::vector3df(-1.597479f, 4.900000f, 2.330615f)},
        BoardSquarePosition{BoardSquare::D1, "D1", irr::core::vector3df(-0.337396f, 4.900000f, -6.493154f)},
        BoardSquarePosition{BoardSquare::D2, "D2", irr::core::vector3df(-0.337396f, 4.900000f, -5.213155f)},
        BoardSquarePosition{BoardSquare::D3, "D3", irr::core::vector3df(-0.337396f, 4.900000f, -3.933155f)},
        BoardSquarePosition{BoardSquare::D4, "D4", irr::core::vector3df(-0.337396f, 4.900000f, -2.653156f)},
        BoardSquarePosition{BoardSquare::D5, "D5", irr::core::vector3df(-0.337396f, 4.900000f, -1.437113f)},
        BoardSquarePosition{BoardSquare::D6, "D6", irr::core::vector3df(-0.337396f, 4.900000f, -0.197992f)},
        BoardSquarePosition{BoardSquare::D7, "D7", irr::core::vector3df(-0.337396f, 4.900000f, 1.018008f)},
        BoardSquarePosition{BoardSquare::D8, "D8", irr::core::vector3df(-0.337396f, 4.900000f, 2.330615f)},
        BoardSquarePosition{BoardSquare::E1, "E1", irr::core::vector3df(0.878604f, 4.900000f, -6.493154f)},
        BoardSquarePosition{BoardSquare::E2, "E2", irr::core::vector3df(0.878604f, 4.900000f, -5.213155f)},
        BoardSquarePosition{BoardSquare::E3, "E3", irr::core::vector3df(0.878604f, 4.900000f, -3.974020f)},
        BoardSquarePosition{BoardSquare::E4, "E4", irr::core::vector3df(0.878604f, 4.900000f, -2.709323f)},
        BoardSquarePosition{BoardSquare::E5, "E5", irr::core::vector3df(0.878604f, 4.900000f, -1.468144f)},
        BoardSquarePosition{BoardSquare::E6, "E6", irr::core::vector3df(0.878604f, 4.900000f, -0.188144f)},
        BoardSquarePosition{BoardSquare::E7, "E7", irr::core::vector3df(0.878604f, 4.900000f, 1.027856f)},
        BoardSquarePosition{BoardSquare::E8, "E8", irr::core::vector3df(0.878604f, 4.900000f, 2.307856f)},
        BoardSquarePosition{BoardSquare::F1, "F1", irr::core::vector3df(2.147397f, 4.900000f, -6.461479f)},
        BoardSquarePosition{BoardSquare::F2, "F2", irr::core::vector3df(2.147397f, 4.900000f, -5.181479f)},
        BoardSquarePosition{BoardSquare::F3, "F3", irr::core::vector3df(2.147397f, 4.900000f, -3.965480f)},
        BoardSquarePosition{BoardSquare::F4, "F4", irr::core::vector3df(2.147397f, 4.900000f, -2.727724f)},
        BoardSquarePosition{BoardSquare::F5, "F5", irr::core::vector3df(2.147397f, 4.900000f, -1.468144f)},
        BoardSquarePosition{BoardSquare::F6, "F6", irr::core::vector3df(2.147397f, 4.900000f, -0.188144f)},
        BoardSquarePosition{BoardSquare::F7, "F7", irr::core::vector3df(2.147397f, 4.900000f, 1.027856f)},
        BoardSquarePosition{BoardSquare::F8, "F8", irr::core::vector3df(2.147397f, 4.900000f, 2.307856f)},
        BoardSquarePosition{BoardSquare::G1, "G1", irr::core::vector3df(3.427397f, 4.900000f, -6.461479f)},
        BoardSquarePosition{BoardSquare::G2, "G2", irr::core::vector3df(3.427397f, 4.900000f, -5.181479f)},
        BoardSquarePosition{BoardSquare::G3, "G3", irr::core::vector3df(3.427397f, 4.900000f, -3.965479f)},
        BoardSquarePosition{BoardSquare::G4, "G4", irr::core::vector3df(3.427397f, 4.900000f, -2.745985f)},
        BoardSquarePosition{BoardSquare::G5, "G5", irr::core::vector3df(3.427397f, 4.900000f, -1.465985f)},
        BoardSquarePosition{BoardSquare::G6, "G6", irr::core::vector3df(3.427397f, 4.900000f, -0.185985f)},
        BoardSquarePosition{BoardSquare::G7, "G7", irr::core::vector3df(3.427397f, 4.900000f, 1.030015f)},
        BoardSquarePosition{BoardSquare::G8, "G8", irr::core::vector3df(3.427397f, 4.900000f, 2.310015f)},
        BoardSquarePosition{BoardSquare::H1, "H1", irr::core::vector3df(4.642811f, 4.900000f, -6.462828f)},
        BoardSquarePosition{BoardSquare::H2, "H2", irr::core::vector3df(4.642811f, 4.900000f, -5.182828f)},
        BoardSquarePosition{BoardSquare::H3, "H3", irr::core::vector3df(4.642811f, 4.900000f, -3.966829f)},
        BoardSquarePosition{BoardSquare::H4, "H4", irr::core::vector3df(4.642811f, 4.900000f, -2.686830f)},
        BoardSquarePosition{BoardSquare::H5, "H5", irr::core::vector3df(4.642811f, 4.900000f, -1.437381f)},
        BoardSquarePosition{BoardSquare::H6, "H6", irr::core::vector3df(4.642811f, 4.900000f, -0.185985f)},
        BoardSquarePosition{BoardSquare::H7, "H7", irr::core::vector3df(4.642811f, 4.900000f, 1.030015f)},
        BoardSquarePosition{BoardSquare::H8, "H8", irr::core::vector3df(4.642811f, 4.900000f, 2.310015f)}
    }};
    return catalog;
}

std::unordered_map<std::string, irr::core::vector3df> CreateBoardSquarePositionMap() {
    std::unordered_map<std::string, irr::core::vector3df> squares;
    squares.reserve(GetBoardSquareCatalog().size());
    for (const BoardSquarePosition& item : GetBoardSquareCatalog()) {
        squares[item.code] = item.position;
    }
    return squares;
}

const irr::core::vector3df* GetCatalogPosition(BoardSquare square) {
    for (const BoardSquarePosition& item : GetBoardSquareCatalog()) {
        if (item.square == square) return &item.position;
    }
    return nullptr;
}

const char* ToString(BoardSquare square) {
    for (const BoardSquarePosition& item : GetBoardSquareCatalog()) {
        if (item.square == square) return item.code;
    }
    return "";
}

} // namespace chessit
