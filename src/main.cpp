#include "Core/Engine.h"
#include "Core/Logger.h"

#include <filesystem>
#include <string>
#include <vector>

namespace {
bool IsValidMediaDirectory(const std::filesystem::path& path) {
    return std::filesystem::exists(path / "CHESSIT_POSITIONS.txt") &&
           std::filesystem::exists(path / "env.irr") &&
           std::filesystem::exists(path / "BOARDER.obj");
}

std::string AbsolutePath(const std::filesystem::path& path) {
    return std::filesystem::absolute(path).lexically_normal().string();
}

std::string ResolveMediaDirectory(int argc, char** argv) {
    std::vector<std::filesystem::path> candidates;

    if (argc > 1) {
        candidates.emplace_back(argv[1]);
    }

    if (argc > 0 && argv[0] && *argv[0]) {
        const std::filesystem::path executablePath = std::filesystem::absolute(argv[0]).lexically_normal();
        const std::filesystem::path executableDir = executablePath.parent_path();
        candidates.emplace_back(executableDir / ".." / "media");
        candidates.emplace_back(executableDir / "media");
    }

    candidates.emplace_back("media");
    candidates.emplace_back(".." / std::filesystem::path("media"));

#ifdef CHESSIT_MEDIA_DIR
    candidates.emplace_back(CHESSIT_MEDIA_DIR);
#endif

    for (const auto& candidate : candidates) {
        if (IsValidMediaDirectory(candidate)) {
            return AbsolutePath(candidate);
        }
    }

    return AbsolutePath("media");
}
} // namespace

int main(int argc, char** argv) {
    const std::string mediaDir = ResolveMediaDirectory(argc, argv);
    chessit::Logger::Info("Using media directory: " + mediaDir);

    chessit::Engine engine;
    if (!engine.Initialize(mediaDir)) {
        chessit::Logger::Error("ChessIt 3D could not be initialized.");
        return 1;
    }

    engine.Run();
    return 0;
}
