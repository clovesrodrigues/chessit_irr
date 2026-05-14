#pragma once

namespace chessit {

enum class GameEventSound {
    PieceHover,
    SquareHover,
    PieceMove,
    PlayerCapture,
    ComputerCapture,
    PlayerWin,
    PlayerLose
};

enum class AIDifficulty {
    Easy,
    Medium,
    Hard,
    Expert
};

} // namespace chessit
