#include "Core/Engine.h"
#include "Core/Logger.h"

#include <filesystem>
#include <string>

namespace {
std::string ResolveMediaDirectory(int argc, char** argv) {
    if (argc > 1 && std::filesystem::exists(argv[1])) {
        return std::filesystem::absolute(argv[1]).string();
    }

#ifdef CHESSIT_MEDIA_DIR
    if (std::filesystem::exists(CHESSIT_MEDIA_DIR)) {
        return std::filesystem::absolute(CHESSIT_MEDIA_DIR).string();
    }
#endif

    if (std::filesystem::exists("media")) {
        return std::filesystem::absolute("media").string();
    }
    if (std::filesystem::exists("../media")) {
        return std::filesystem::absolute("../media").string();
    }
    return "media";
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
