#pragma once

#include "Game/ChessTypes.h"

#include <cstddef>
#include <string>
#include <vector>

namespace chessit {

class ONNXAIManager {
public:
    // Model contract shared with training/train_chessit_model.py.
    static constexpr std::size_t BoardChannels = 13;
    static constexpr std::size_t BoardRows = 8;
    static constexpr std::size_t BoardColumns = 8;
    static constexpr std::size_t BoardTensorSize = BoardChannels * BoardRows * BoardColumns;
    static constexpr std::size_t MovePolicySize = 64 * 64;
    static constexpr const char* ModelInputName = "board";
    static constexpr const char* MoveLogitsOutputName = "move_logits";
    static constexpr const char* ValueOutputName = "value";

    void Initialize();
    bool LoadModel(const std::string& modelPath);
    Move PredictMove() const;
    bool TrainDataExport(const std::string& outputPath) const;
    bool IsModelLoaded() const { return modelLoaded_; }
    const std::string& GetModelPath() const { return modelPath_; }

private:
    std::string modelPath_;
    bool modelLoaded_ = false;
};

} // namespace chessit
