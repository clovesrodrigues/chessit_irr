#pragma once

#include "Parsing/PositionParser.h"

#include <string>

namespace chessit {

class BoardManager {
public:
    bool LoadPositions(const std::string& filePath);
    const BoardPositions& GetPositions() const { return positions_; }
    const irr::core::vector3df* GetSquarePosition(const std::string& square) const;
    bool HasSquare(const std::string& square) const;
    std::string FindNearestSquare(const irr::core::vector3df& worldPosition, float maxDistance = 0.85f) const;

private:
    BoardPositions positions_;
};

} // namespace chessit
