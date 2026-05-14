#pragma once

#include "Game/ChessTypes.h"

#include <string>
#include <vector>

namespace chessit {

class ONNXAIManager {
public:
    void Initialize();
    bool LoadModel(const std::string& modelPath);
    Move PredictMove() const;
    bool TrainDataExport(const std::string& outputPath) const;

private:
    std::string modelPath_;
    bool modelLoaded_ = false;
};

} // namespace chessit
