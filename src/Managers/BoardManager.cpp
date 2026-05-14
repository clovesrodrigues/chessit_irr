#include "Managers/BoardManager.h"

#include "Core/Logger.h"

namespace chessit {

bool BoardManager::LoadPositions(const std::string& filePath) {
    try {
        PositionParser parser;
        positions_ = parser.ParseFile(filePath);
        return true;
    } catch (const std::exception& e) {
        Logger::Error(e.what());
        return false;
    }
}

const irr::core::vector3df* BoardManager::GetSquarePosition(const std::string& square) const {
    const auto it = positions_.squares.find(square);
    if (it == positions_.squares.end()) return nullptr;
    return &it->second;
}

bool BoardManager::HasSquare(const std::string& square) const {
    return positions_.squares.find(square) != positions_.squares.end();
}

std::string BoardManager::FindNearestSquare(const irr::core::vector3df& worldPosition, float maxDistance) const {
    std::string nearest;
    float bestDistanceSq = maxDistance * maxDistance;
    for (const auto& square : positions_.squares) {
        const float dx = square.second.X - worldPosition.X;
        const float dz = square.second.Z - worldPosition.Z;
        const float distanceSq = dx * dx + dz * dz;
        if (distanceSq <= bestDistanceSq) {
            bestDistanceSq = distanceSq;
            nearest = square.first;
        }
    }
    return nearest;
}

} // namespace chessit
