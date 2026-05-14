#pragma once

#include <irrlicht.h>
#include <string>
#include <unordered_map>

namespace chessit {

struct CameraConfig {
    irr::core::vector3df position{0.0f, -48.0f, 0.0f};
    float fovY = 1.256637f;
    float aspect = 1.777777f;
};

struct BoardPositions {
    irr::core::vector3df boardPosition{-1.609478f, 4.852117f, 2.316586f};
    CameraConfig camera;
    std::unordered_map<std::string, irr::core::vector3df> squares;
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
