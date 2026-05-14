#include "Managers/PieceManager.h"

#include "Core/Logger.h"

#include <filesystem>

namespace chessit {

bool PieceManager::LoadInitialPieces(irr::scene::ISceneManager* sceneManager, const BoardManager& boardManager, const std::string& mediaDir) {
    if (!sceneManager) {
        Logger::Error("PieceManager received a null Irrlicht scene manager.");
        return false;
    }

    pieces_.clear();
    boardState_.clear();

    bool allLoaded = true;
    for (const PieceSpawn& spawn : CreateInitialLayout()) {
        const irr::core::vector3df* squarePosition = boardManager.GetSquarePosition(spawn.square);
        if (!squarePosition) {
            Logger::Error("Missing board coordinate for square " + spawn.square + ".");
            allLoaded = false;
            continue;
        }

        const std::string assetPath = JoinPath(mediaDir, spawn.asset);
        irr::scene::IAnimatedMesh* mesh = sceneManager->getMesh(assetPath.c_str());
        if (!mesh) {
            Logger::Error("Could not load piece mesh: " + assetPath);
            allLoaded = false;
            continue;
        }

        irr::scene::ISceneNode* node = sceneManager->addMeshSceneNode(mesh);
        if (!node) {
            Logger::Error("Could not create scene node for piece: " + spawn.name);
            allLoaded = false;
            continue;
        }

        node->setName(spawn.name.c_str());
        node->setPosition(*squarePosition);
        node->setMaterialFlag(irr::video::EMF_LIGHTING, false);
        node->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true);

        auto piece = std::make_unique<ChessPiece>();
        piece->name = spawn.name;
        piece->type = spawn.type;
        piece->color = spawn.color;
        piece->position = *squarePosition;
        piece->node = node;
        piece->currentSquare = spawn.square;
        boardState_[spawn.square] = piece.get();
        pieces_.push_back(std::move(piece));
    }

    Logger::Info("Initial chess layout loaded with " + std::to_string(pieces_.size()) + " pieces.");
    return allLoaded;
}

ChessPiece* PieceManager::GetPieceAt(const std::string& square) const {
    const auto it = boardState_.find(square);
    return it == boardState_.end() ? nullptr : it->second;
}

std::vector<PieceManager::PieceSpawn> PieceManager::CreateInitialLayout() {
    std::vector<PieceSpawn> layout = {
        {"ROCK_WHITE1", PieceType::Rook, PieceColor::White, "ROCK_WHITE1.obj", "A1"},
        {"KNIGTH_WHITE1", PieceType::Knight, PieceColor::White, "KNIGTH_WHITE1.obj", "B1"},
        {"BISPO_WHITE1", PieceType::Bishop, PieceColor::White, "BISPO_WHITE1.obj", "C1"},
        {"KING_WHITE", PieceType::King, PieceColor::White, "KING_WHITE.obj", "D1"},
        {"QUEEN_WHITE", PieceType::Queen, PieceColor::White, "QUEEN_WHITE.obj", "E1"},
        {"BISPO_WHITE2", PieceType::Bishop, PieceColor::White, "BISPO_WHITE2.obj", "F1"},
        {"KNIGTH_WHITE2", PieceType::Knight, PieceColor::White, "KNIGTH_WHITE2.obj", "G1"},
        {"ROCK_WHITE2", PieceType::Rook, PieceColor::White, "ROCK_WHITE2.obj", "H1"},
        {"PAWN_BLACK1", PieceType::Pawn, PieceColor::Black, "PAWN_BLACK1.obj", "A7"},
        {"PAWN_BLACK2", PieceType::Pawn, PieceColor::Black, "PAWN_BLACK2.obj", "B7"},
        {"PAWN_BLACK3", PieceType::Pawn, PieceColor::Black, "PAWN_BLACK3.obj", "C7"},
        {"PAWN_BLACK4", PieceType::Pawn, PieceColor::Black, "PAWN_BLACK4.obj", "D7"},
        {"PAWN_BLACK5", PieceType::Pawn, PieceColor::Black, "PAWN_BLACK5.obj", "E7"},
        {"PAWN_BLACK6", PieceType::Pawn, PieceColor::Black, "PAWN_BLACK6.obj", "F7"},
        {"PAWN_BLACK7", PieceType::Pawn, PieceColor::Black, "PAWN_BLACK7.obj", "G7"},
        {"PAWN_BLACK8", PieceType::Pawn, PieceColor::Black, "PAWN_BLACK8.obj", "H7"},
        {"ROCK_BLACK1", PieceType::Rook, PieceColor::Black, "ROCK_BLACK1.obj", "A8"},
        {"KNIGTH_BLACK1", PieceType::Knight, PieceColor::Black, "KNIGTH_BLACK1.obj", "B8"},
        {"BISPO_BLACK1", PieceType::Bishop, PieceColor::Black, "BISPO_BLACK1.obj", "C8"},
        {"KING_BLACK", PieceType::King, PieceColor::Black, "KING_BLACK.obj", "D8"},
        {"QUEEN_BLACK", PieceType::Queen, PieceColor::Black, "QUEEN_BLACK.obj", "E8"},
        {"BISPO_BLACK2", PieceType::Bishop, PieceColor::Black, "BISPO_BLACK2.obj", "F8"},
        {"KNIGTH_BLACK2", PieceType::Knight, PieceColor::Black, "KNIGTH_BLACK2.obj", "G8"},
        {"ROCK_BLACK2", PieceType::Rook, PieceColor::Black, "ROCK_BLACK2.obj", "H8"},
    };

    for (int file = 0; file < 8; ++file) {
        const char column = static_cast<char>('A' + file);
        layout.push_back({"PAWN_WHITE" + std::to_string(file + 1), PieceType::Pawn, PieceColor::White,
                          "PAWN_WHITE" + std::to_string(file + 1) + ".mtl.obj", std::string(1, column) + "2"});
    }
    return layout;
}

std::string PieceManager::JoinPath(const std::string& base, const std::string& file) {
    return (std::filesystem::path(base) / file).string();
}

} // namespace chessit
