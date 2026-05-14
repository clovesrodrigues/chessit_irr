#pragma once

#include "Game/BoardSquare.h"

#include <irrlicht.h>
#include <string>
#include <unordered_map>

namespace chessit {

struct CameraConfig {
    irr::core::vector3df position{-1.609478f, 12.500000f, -10.500000f};
    irr::core::vector3df target{-1.609478f, 4.900000f, 2.316586f};
    bool hasTarget = true;
    float fovY = 1.256637f;
    float aspect = 1.777777f;
};

struct BoardPositions {
    irr::core::vector3df boardPosition{-1.609478f, 4.852117f, 2.316586f};
    CameraConfig camera;
    bool squareCoordinatesAreBoardLocal = true;
    std::unordered_map<std::string, irr::core::vector3df> squares = CreateBoardSquarePositionMap();
};

class PositionParser {
public:
    BoardPositions ParseFile(const std::string& filePath) const;

private:
    static std::string NormalizeLine(std::string line);
    static irr::core::vector3df ParseVector(const std::string& text, std::size_t startPos);
    static void NormalizeBoardLocalSquares(BoardPositions& positions);
    static float ParseFloatAfterToken(const std::string& text, const std::string& token, float fallback);
};

} // namespace chessit
