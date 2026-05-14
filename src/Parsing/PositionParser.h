#pragma once

#include "Game/BoardSquare.h"

#include <irrlicht.h>
#include <string>
#include <unordered_map>

namespace chessit {

struct CameraConfig {
    irr::core::vector3df position{-0.967126f, 48.647911f, 0.808066f};
    irr::core::vector3df target{0.0f, 0.0f, 0.0f};
    bool hasTarget = false;
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
    static float ParseFloatAfterToken(const std::string& text, const std::string& token, float fallback);
};

} // namespace chessit
