#include "Managers/ONNXAIManager.h"

#include "Core/Logger.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#ifdef CHESSIT_ENABLE_ONNX_RUNTIME
#include <onnxruntime_cxx_api.h>
#endif

namespace chessit {

namespace {

int PiecePlane(PieceType type, PieceColor color) {
    const int colorOffset = color == PieceColor::White ? 0 : 6;
    switch (type) {
        case PieceType::Pawn: return colorOffset + 0;
        case PieceType::Knight: return colorOffset + 1;
        case PieceType::Bishop: return colorOffset + 2;
        case PieceType::Rook: return colorOffset + 3;
        case PieceType::Queen: return colorOffset + 4;
        case PieceType::King: return colorOffset + 5;
        default: return -1;
    }
}

bool ParseSquareIndex(const std::string& square, int& index) {
    if (square.size() < 2) return false;

    const char fileChar = static_cast<char>(std::tolower(static_cast<unsigned char>(square[0])));
    const char rankChar = square[1];
    if (fileChar < 'a' || fileChar > 'h' || rankChar < '1' || rankChar > '8') return false;

    const int file = fileChar - 'a';
    const int rank = rankChar - '1';
    index = rank * 8 + file;
    return true;
}

#ifdef CHESSIT_ENABLE_ONNX_RUNTIME
#ifdef _WIN32
std::wstring ToOrtPath(const std::string& path) {
    return std::wstring(path.begin(), path.end());
}
#else
const std::string& ToOrtPath(const std::string& path) {
    return path;
}
#endif
#endif

} // namespace

ONNXAIManager::ONNXAIManager() = default;
ONNXAIManager::~ONNXAIManager() = default;

void ONNXAIManager::Initialize() {
#ifdef CHESSIT_ENABLE_ONNX_RUNTIME
    try {
        if (!env_) env_ = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "ChessItONNX");
        if (!sessionOptions_) {
            sessionOptions_ = std::make_unique<Ort::SessionOptions>();
            sessionOptions_->SetIntraOpNumThreads(1);
            sessionOptions_->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);
        }
    } catch (const Ort::Exception& ex) {
        statusMessage_ = std::string("ONNX Runtime init failed: ") + ex.what();
        Logger::Error(statusMessage_);
        modelLoaded_ = false;
    }
#else
    statusMessage_ = "ONNX Runtime disabled in this build";
    Logger::Info(statusMessage_);
#endif
}

bool ONNXAIManager::LoadModel(const std::string& modelPath) {
    modelPath_ = modelPath;
    modelLoaded_ = false;
    statusMessage_ = "ONNX model not loaded";

#ifdef CHESSIT_ENABLE_ONNX_RUNTIME
    try {
        Initialize();
        if (!env_ || !sessionOptions_) {
            statusMessage_ = "ONNX Runtime session options unavailable";
            return false;
        }

        const auto ortPath = ToOrtPath(modelPath_);
        session_ = std::make_unique<Ort::Session>(*env_, ortPath.c_str(), *sessionOptions_);
        modelLoaded_ = true;
        statusMessage_ = "ONNX loaded: " + modelPath_;
        Logger::Info(statusMessage_);
    } catch (const Ort::Exception& ex) {
        session_.reset();
        statusMessage_ = std::string("Failed to load ONNX model '") + modelPath_ + "': " + ex.what();
        Logger::Error(statusMessage_);
    } catch (const std::exception& ex) {
        session_.reset();
        statusMessage_ = std::string("Failed to load ONNX model '") + modelPath_ + "': " + ex.what();
        Logger::Error(statusMessage_);
    }
#else
    statusMessage_ = "ONNX Runtime disabled; model not loaded: " + modelPath_;
    Logger::Info(statusMessage_);
#endif

    return modelLoaded_;
}

Move ONNXAIManager::PredictMove(const PieceManager::BoardState& boardState,
                                const std::vector<Move>& legalMoves,
                                PieceColor sideToMove,
                                std::size_t candidateOffset) const {
    if (legalMoves.empty()) return Move{};

#ifdef CHESSIT_ENABLE_ONNX_RUNTIME
    if (!modelLoaded_ || !session_) return Move{};

    try {
        BoardTensor boardTensor = EncodeBoard(boardState, sideToMove);
        const std::array<int64_t, 4> inputShape = {1, static_cast<int64_t>(BoardChannels), static_cast<int64_t>(BoardRows), static_cast<int64_t>(BoardColumns)};
        Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            memoryInfo,
            boardTensor.data(),
            boardTensor.size(),
            inputShape.data(),
            inputShape.size());

        const char* inputNames[] = {ModelInputName};
        const char* outputNames[] = {MoveLogitsOutputName, ValueOutputName};
        std::vector<Ort::Value> outputs = session_->Run(
            Ort::RunOptions{nullptr},
            inputNames,
            &inputTensor,
            1,
            outputNames,
            2);

        if (outputs.empty() || !outputs[0].IsTensor()) return Move{};

        const float* moveLogits = outputs[0].GetTensorData<float>();
        std::vector<std::pair<float, Move>> scoredMoves;
        scoredMoves.reserve(legalMoves.size());

        for (const Move& move : legalMoves) {
            const int policyIndex = MoveToPolicyIndex(move);
            if (policyIndex < 0 || static_cast<std::size_t>(policyIndex) >= MovePolicySize) continue;
            scoredMoves.push_back({moveLogits[policyIndex], move});
        }

        if (scoredMoves.empty()) return Move{};

        std::sort(scoredMoves.begin(), scoredMoves.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.first > rhs.first;
        });

        const std::size_t selectedIndex = std::min(candidateOffset, scoredMoves.size() - 1);
        return scoredMoves[selectedIndex].second;
    } catch (const Ort::Exception& ex) {
        Logger::Error(std::string("ONNX inference failed: ") + ex.what());
    } catch (const std::exception& ex) {
        Logger::Error(std::string("ONNX inference failed: ") + ex.what());
    }
#else
    (void)boardState;
    (void)sideToMove;
#endif

    return Move{};
}

Move ONNXAIManager::PredictMove() const {
    return Move{};
}

bool ONNXAIManager::TrainDataExport(const std::string& outputPath) const {
    Logger::Info("ONNX training data export placeholder: " + outputPath);
    return true;
}

ONNXAIManager::BoardTensor ONNXAIManager::EncodeBoard(const PieceManager::BoardState& boardState, PieceColor sideToMove) {
    BoardTensor tensor{};

    for (const auto& entry : boardState) {
        const ChessPiece* piece = entry.second;
        if (!piece || !piece->alive) continue;

        int squareIndex = 0;
        if (!ParseSquareIndex(entry.first, squareIndex)) continue;

        const int plane = PiecePlane(piece->type, piece->color);
        if (plane < 0) continue;

        const std::size_t offset = static_cast<std::size_t>(plane) * BoardRows * BoardColumns + static_cast<std::size_t>(squareIndex);
        if (offset < tensor.size()) tensor[offset] = 1.0f;
    }

    const float sideValue = sideToMove == PieceColor::White ? 1.0f : -1.0f;
    const std::size_t sidePlaneOffset = 12 * BoardRows * BoardColumns;
    std::fill(tensor.begin() + static_cast<std::ptrdiff_t>(sidePlaneOffset), tensor.end(), sideValue);

    return tensor;
}

int ONNXAIManager::MoveToPolicyIndex(const Move& move) {
    int fromIndex = 0;
    int toIndex = 0;
    if (!ParseSquareIndex(move.fromSquare, fromIndex) || !ParseSquareIndex(move.toSquare, toIndex)) return -1;
    return fromIndex * 64 + toIndex;
}

} // namespace chessit
