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

} // namespace chessit
