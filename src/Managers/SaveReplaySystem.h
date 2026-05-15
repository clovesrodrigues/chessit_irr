#pragma once

#include "Game/ChessTypes.h"

#include <filesystem>
#include <string>
#include <vector>

namespace chessit {

class SaveReplaySystem {
public:
    void Initialize();
    void StartNewGame();
    void RecordMove(PieceColor color,
                    PieceType pieceType,
                    const std::string& fromSquare,
                    const std::string& toSquare,
                    bool isCapture,
                    PieceType promotion = PieceType::Unknown);

private:
    struct SavedMove {
        PieceColor color = PieceColor::None;
        PieceType pieceType = PieceType::Unknown;
        std::string fromSquare;
        std::string toSquare;
        bool isCapture = false;
        PieceType promotion = PieceType::Unknown;
    };

    std::filesystem::path ResolveGamesPath() const;
    void RefreshStableFilePrefix();
    void RewriteCurrentGame() const;
    std::string BuildCurrentGamePgn() const;
    std::string FormatMove(const SavedMove& move) const;

    std::filesystem::path gamesPath_;
    std::string stableFilePrefix_;
    std::vector<SavedMove> moves_;
    bool initialized_ = false;
};

} // namespace chessit
