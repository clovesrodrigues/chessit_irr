#include "Parsing/PositionParser.h"

#include "Core/Logger.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <limits>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace chessit {

BoardPositions PositionParser::ParseFile(const std::string& filePath) const {
    std::ifstream input(filePath);
    if (!input.is_open()) {
        throw std::runtime_error("Unable to open positions file: " + filePath);
    }

    BoardPositions positions;
    std::string line;
    const std::regex squarePattern(R"(^\s*([A-H][1-8])\s*(-?[0-9])?)", std::regex::icase);

    while (std::getline(input, line)) {
        line = NormalizeLine(line);
        if (line.empty()) continue;

        if (line.rfind("BOARD", 0) == 0) {
            positions.boardPosition = ParseVector(line, 5);
            continue;
        }

        if (line.rfind("SQUARE_SPACE", 0) == 0) {
            std::string upper = line;
            std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
            positions.squareCoordinatesAreBoardLocal = upper.find("WORLD") == std::string::npos;
            continue;
        }

        if (line.rfind("CAMERA", 0) == 0) {
            positions.camera.position = ParseVector(line, 6);
            const auto targetPos = line.find("TARGET");
            if (targetPos != std::string::npos) {
                positions.camera.target = ParseVector(line, targetPos + 6);
                positions.camera.hasTarget = true;
            }
            positions.camera.fovY = ParseFloatAfterToken(line, "FOVY", positions.camera.fovY);
            positions.camera.aspect = ParseFloatAfterToken(line, "ASPECT", positions.camera.aspect);
            continue;
        }

        std::smatch match;
        if (std::regex_search(line, match, squarePattern)) {
            std::string square = match[1].str();
            std::transform(square.begin(), square.end(), square.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
            positions.squares[square] = ParseVector(line, square.size());
        }
    }

    if (positions.squareCoordinatesAreBoardLocal) {
        NormalizeBoardLocalSquares(positions);
    }

    if (positions.camera.position.Y < positions.boardPosition.Y - 5.0f) {
        positions.camera.position.Y = std::abs(positions.camera.position.Y);
        Logger::Warning("Camera Y was below the board; mirrored it above the board for Irrlicht's Y-up coordinate system.");
    }

    if (positions.squares.size() != 64) {
        Logger::Warning("Positions file contains " + std::to_string(positions.squares.size()) + " chess squares; expected 64.");
    }

    Logger::Info("Loaded board coordinates from " + filePath + " (" + std::to_string(positions.squares.size()) + " squares).");
    return positions;
}

std::string PositionParser::NormalizeLine(std::string line) {
    line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
    if (line.size() >= 3 && static_cast<unsigned char>(line[0]) == 0xEF &&
        static_cast<unsigned char>(line[1]) == 0xBB && static_cast<unsigned char>(line[2]) == 0xBF) {
        line.erase(0, 3);
    }
    const auto comment = line.find('#');
    if (comment != std::string::npos) line.erase(comment);
    const auto first = line.find_first_not_of(" \t");
    if (first == std::string::npos) return {};
    const auto last = line.find_last_not_of(" \t");
    return line.substr(first, last - first + 1);
}

irr::core::vector3df PositionParser::ParseVector(const std::string& text, std::size_t startPos) {
    static const std::regex numberPattern(R"((-?\d+(?:\.\d+)?))");
    std::string tail = text.substr(std::min(startPos, text.size()));
    std::sregex_iterator it(tail.begin(), tail.end(), numberPattern);
    std::sregex_iterator end;

    float values[3] = {0.0f, 0.0f, 0.0f};
    for (int i = 0; i < 3 && it != end; ++i, ++it) {
        values[i] = std::stof((*it)[1].str());
    }
    return irr::core::vector3df(values[0], values[1], values[2]);
}

void PositionParser::NormalizeBoardLocalSquares(BoardPositions& positions) {
    if (positions.squares.empty()) return;

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();
    for (const auto& square : positions.squares) {
        minX = std::min(minX, square.second.X);
        maxX = std::max(maxX, square.second.X);
        minZ = std::min(minZ, square.second.Z);
        maxZ = std::max(maxZ, square.second.Z);
    }

    constexpr float playableHalfExtent = 4.375f;
    const float xSpan = std::max(0.0001f, maxX - minX);
    const float zSpan = std::max(0.0001f, maxZ - minZ);
    for (auto& square : positions.squares) {
        const float normalizedX = ((square.second.X - minX) / xSpan) * (playableHalfExtent * 2.0f) - playableHalfExtent;
        const float normalizedZ = ((square.second.Z - minZ) / zSpan) * (playableHalfExtent * 2.0f) - playableHalfExtent;
        square.second.X = positions.boardPosition.X + normalizedX;
        square.second.Z = positions.boardPosition.Z + normalizedZ;
    }
}

float PositionParser::ParseFloatAfterToken(const std::string& text, const std::string& token, float fallback) {
    const auto pos = text.find(token);
    if (pos == std::string::npos) return fallback;
    static const std::regex numberPattern(R"((-?\d+(?:\.\d+)?))");
    const std::string tail = text.substr(pos + token.size());
    std::smatch match;
    if (!std::regex_search(tail, match, numberPattern)) return fallback;
    return std::stof(match[1].str());
}

} // namespace chessit
