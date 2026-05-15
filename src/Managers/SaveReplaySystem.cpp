#include "Managers/SaveReplaySystem.h"

#include "Core/Logger.h"

#include <cctype>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace chessit {
namespace {
constexpr const char* kManagedEventTag = "[Event \"ChessIt 3D Current Game\"]";

std::string TrimRight(std::string value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
        value.pop_back();
    }
    return value;
}

std::string CurrentPgnDate() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif
    std::ostringstream date;
    date << std::put_time(&localTime, "%Y.%m.%d");
    return date.str();
}

char PieceLetter(PieceType type) {
    switch (type) {
        case PieceType::King: return 'K';
        case PieceType::Queen: return 'Q';
        case PieceType::Rook: return 'R';
        case PieceType::Bishop: return 'B';
        case PieceType::Knight: return 'N';
        default: return '\0';
    }
}
} // namespace

void SaveReplaySystem::Initialize() {
    gamesPath_ = ResolveGamesPath();
    initialized_ = true;
    RefreshStableFilePrefix();
    moves_.clear();
    RewriteCurrentGame();
}

void SaveReplaySystem::StartNewGame() {
    if (!initialized_) Initialize();
    moves_.clear();
    RewriteCurrentGame();
}

void SaveReplaySystem::RecordMove(PieceColor color,
                                  PieceType pieceType,
                                  const std::string& fromSquare,
                                  const std::string& toSquare,
                                  bool isCapture,
                                  PieceType promotion) {
    if (!initialized_) Initialize();
    if (fromSquare.empty() || toSquare.empty()) return;

    moves_.push_back({color, pieceType, fromSquare, toSquare, isCapture, promotion});
    RewriteCurrentGame();
}

std::filesystem::path SaveReplaySystem::ResolveGamesPath() const {
    const std::filesystem::path parentTraining = std::filesystem::path("..") / "training";
    const std::filesystem::path localTraining = "training";

    if (std::filesystem::exists(parentTraining) || !std::filesystem::exists(localTraining)) {
        return parentTraining / "games.pgn";
    }
    return localTraining / "games.pgn";
}

void SaveReplaySystem::RefreshStableFilePrefix() {
    stableFilePrefix_.clear();

    std::ifstream input(gamesPath_);
    if (!input) return;

    std::ostringstream buffer;
    buffer << input.rdbuf();
    std::string content = buffer.str();

    const std::size_t managedBlock = content.rfind(kManagedEventTag);
    if (managedBlock != std::string::npos) {
        content.erase(managedBlock);
    }

    stableFilePrefix_ = TrimRight(content);
}

void SaveReplaySystem::RewriteCurrentGame() const {
    std::error_code error;
    std::filesystem::create_directories(gamesPath_.parent_path(), error);
    if (error) {
        Logger::Warning("Could not create PGN training folder: " + gamesPath_.parent_path().string());
        return;
    }

    std::ofstream output(gamesPath_, std::ios::trunc);
    if (!output) {
        Logger::Warning("Could not open PGN file for writing: " + gamesPath_.string());
        return;
    }

    if (!stableFilePrefix_.empty()) output << stableFilePrefix_ << "\n\n";
    output << BuildCurrentGamePgn();
    Logger::Info("Saved current game replay to " + gamesPath_.string());
}

std::string SaveReplaySystem::BuildCurrentGamePgn() const {
    std::ostringstream pgn;
    pgn << kManagedEventTag << '\n'
        << "[Site \"ChessIt 3D\"]\n"
        << "[Date \"" << CurrentPgnDate() << "\"]\n"
        << "[Round \"-\"]\n"
        << "[White \"Player\"]\n"
        << "[Black \"Computer\"]\n"
        << "[Result \"*\"]\n"
        << "[PlyCount \"" << moves_.size() << "\"]\n\n";

    for (std::size_t i = 0; i < moves_.size(); ++i) {
        if (i % 2 == 0) pgn << (i / 2 + 1) << ". ";
        pgn << FormatMove(moves_[i]) << ' ';
    }
    pgn << "*\n";
    return pgn.str();
}

std::string SaveReplaySystem::FormatMove(const SavedMove& move) const {
    std::ostringstream notation;
    const char pieceLetter = PieceLetter(move.pieceType);
    if (pieceLetter != '\0') notation << pieceLetter;
    notation << move.fromSquare << (move.isCapture ? 'x' : '-') << move.toSquare;

    if (move.promotion != PieceType::Unknown) {
        const char promotionLetter = PieceLetter(move.promotion);
        if (promotionLetter != '\0') notation << '=' << promotionLetter;
    }

    return notation.str();
}

} // namespace chessit
