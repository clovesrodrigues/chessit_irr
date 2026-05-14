#include "Managers/ONNXAIManager.h"

#include "Core/Logger.h"

namespace chessit {

void ONNXAIManager::Initialize() {}

bool ONNXAIManager::LoadModel(const std::string& modelPath) {
    modelPath_ = modelPath;
    modelLoaded_ = true;
    Logger::Info("ONNX model registered for future inference: " + modelPath_);
    return modelLoaded_;
}

Move ONNXAIManager::PredictMove() const {
    return Move{};
}

bool ONNXAIManager::TrainDataExport(const std::string& outputPath) const {
    Logger::Info("ONNX training data export placeholder: " + outputPath);
    return true;
}

} // namespace chessit
