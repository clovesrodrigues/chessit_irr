#pragma once

#include "Game/ChessTypes.h"
#include "Managers/PieceManager.h"

#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#ifdef CHESSIT_ENABLE_ONNX_RUNTIME
namespace Ort {
struct Env;
struct Session;
struct SessionOptions;
} // namespace Ort
#endif

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

    using BoardTensor = std::array<float, BoardTensorSize>;

    ONNXAIManager();
    ~ONNXAIManager();

    ONNXAIManager(const ONNXAIManager&) = delete;
    ONNXAIManager& operator=(const ONNXAIManager&) = delete;

    void Initialize();
    bool LoadModel(const std::string& modelPath);
    Move PredictMove(const PieceManager::BoardState& boardState,
                     const std::vector<Move>& legalMoves,
                     PieceColor sideToMove,
                     std::size_t candidateOffset = 0) const;
    Move PredictMove() const;
    bool TrainDataExport(const std::string& outputPath) const;
    bool IsModelLoaded() const { return modelLoaded_; }
    const std::string& GetModelPath() const { return modelPath_; }
    const std::string& GetStatusMessage() const { return statusMessage_; }

    static BoardTensor EncodeBoard(const PieceManager::BoardState& boardState, PieceColor sideToMove);
    static int MoveToPolicyIndex(const Move& move);

private:
    std::string modelPath_;
    std::string statusMessage_ = "ONNX not initialized";
    bool modelLoaded_ = false;

#ifdef CHESSIT_ENABLE_ONNX_RUNTIME
    std::unique_ptr<Ort::Env> env_;
    std::unique_ptr<Ort::SessionOptions> sessionOptions_;
    std::unique_ptr<Ort::Session> session_;
#endif
};

} // namespace chessit
