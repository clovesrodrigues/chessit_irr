#include "Managers/PieceManager.h"

#include "Core/Logger.h"

#include <filesystem>

namespace chessit {

bool PieceManager::LoadInitialPieces(irr::scene::ISceneManager* sceneManager, const BoardManager& boardManager, const std::string& mediaDir) {
    if (!sceneManager) {
        Logger::Error("PieceManager received a null Irrlicht scene manager.");
        return false;
    }

    for (const auto& piece : pieces_) {
        if (piece && piece->node) piece->node->remove();
    }
    pieces_.clear();
    boardState_.clear();
    piecesByNodeId_.clear();

    irr::io::IFileSystem* fileSystem = sceneManager->getFileSystem();
    const irr::io::path previousWorkingDirectory = fileSystem ? fileSystem->getWorkingDirectory() : irr::io::path();
    if (fileSystem && !fileSystem->changeWorkingDirectoryTo(mediaDir.c_str())) {
        Logger::Warning("Could not switch Irrlicht working directory to media folder for piece loading: " + mediaDir);
    }

    bool allLoaded = true;
    irr::s32 nextNodeId = PieceNodeIdBase();
    for (const PieceSpawn& spawn : CreateInitialLayout()) {
        const irr::core::vector3df* squarePosition = boardManager.GetSquarePosition(spawn.square);
        if (!squarePosition) {
            Logger::Error("Missing board coordinate for square " + spawn.square + ".");
            allLoaded = false;
            continue;
        }

        // Load mesh using full path
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

        const irr::core::vector3df meshAnchor = ComputeMeshAnchor(mesh);

        node->setName(spawn.name.c_str());
        node->setID(nextNodeId++);
        node->setPosition(ComputeNodePosition(*squarePosition, meshAnchor));
        ApplyPieceMaterialStyle(node, spawn.color);

        auto piece = std::make_unique<ChessPiece>();
        piece->name = spawn.name;
        piece->type = spawn.type;
        piece->color = spawn.color;
        piece->position = *squarePosition;
        piece->meshAnchor = meshAnchor;
        piece->node = node;
        piece->currentSquare = spawn.square;
        boardState_[spawn.square] = piece.get();
        piecesByNodeId_[node->getID()] = piece.get();
        pieces_.push_back(std::move(piece));
    }

    if (fileSystem) fileSystem->changeWorkingDirectoryTo(previousWorkingDirectory);

    Logger::Info("Initial chess layout loaded with " + std::to_string(pieces_.size()) + " pieces.");
    return allLoaded;
}

ChessPiece* PieceManager::GetPieceAt(const std::string& square) const {
    const auto it = boardState_.find(square);
    return it == boardState_.end() ? nullptr : it->second;
}

ChessPiece* PieceManager::GetPieceByNode(const irr::scene::ISceneNode* node) const {
    if (!node) return nullptr;
    const auto it = piecesByNodeId_.find(node->getID());
    return it == piecesByNodeId_.end() ? nullptr : it->second;
}

bool PieceManager::HasAliveKing(PieceColor color) const {
    for (const auto& piece : pieces_) {
        if (piece && piece->alive && piece->type == PieceType::King && piece->color == color) return true;
    }
    return false;
}

bool PieceManager::MovePiece(ChessPiece* piece, const std::string& targetSquare, const BoardManager& boardManager, bool* capturedPiece) {
    if (capturedPiece) *capturedPiece = false;
    if (!piece || !piece->alive) return false;
    const irr::core::vector3df* targetPosition = boardManager.GetSquarePosition(targetSquare);
    if (!targetPosition) return false;

    auto targetIt = boardState_.find(targetSquare);
    if (targetIt != boardState_.end() && targetIt->second && targetIt->second != piece) {
        targetIt->second->alive = false;
        if (targetIt->second->node) targetIt->second->node->setVisible(false);
        if (capturedPiece) *capturedPiece = true;
        boardState_.erase(targetIt);
    }

    boardState_.erase(piece->currentSquare);
    piece->currentSquare = targetSquare;
    piece->position = *targetPosition;
    piece->firstMove = false;
    if (piece->node) piece->node->setPosition(ComputeNodePosition(*targetPosition, piece->meshAnchor));
    boardState_[targetSquare] = piece;
    return true;
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
        {"PAWN_WHITE1", PieceType::Pawn, PieceColor::White, "PAWN_WHITE1.mtl.obj", "A2"},
        {"PAWN_WHITE2", PieceType::Pawn, PieceColor::White, "PAWN_WHITE2.mtl.obj", "B2"},
        {"PAWN_WHITE3", PieceType::Pawn, PieceColor::White, "PAWN_WHITE3.mtl.obj", "C2"},
        {"PAWN_WHITE4", PieceType::Pawn, PieceColor::White, "PAWN_WHITE4.mtl.obj", "D2"},
        {"PAWN_WHITE5", PieceType::Pawn, PieceColor::White, "PAWN_WHITE5.mtl.obj", "E2"},
        {"PAWN_WHITE6", PieceType::Pawn, PieceColor::White, "PAWN_WHITE6.mtl.obj", "F2"},
        {"PAWN_WHITE7", PieceType::Pawn, PieceColor::White, "PAWN_WHITE7.mtl.obj", "G2"},
        {"PAWN_WHITE8", PieceType::Pawn, PieceColor::White, "PAWN_WHITE8.mtl.obj", "H2"},
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

    return layout;
}

void PieceManager::ApplyPieceMaterialStyle(irr::scene::ISceneNode* node, PieceColor color) {
    if (!node) return;
    node->setMaterialFlag(irr::video::EMF_LIGHTING, true);
    node->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true);
    node->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true);
    node->setMaterialFlag(irr::video::EMF_BACK_FACE_CULLING, false);
    node->setMaterialType(irr::video::EMT_SOLID);

    const bool whitePiece = color == PieceColor::White;
    for (irr::u32 i = 0; i < node->getMaterialCount(); ++i) {
        irr::video::SMaterial& material = node->getMaterial(i);
        material.MaterialType = irr::video::EMT_SOLID;
        material.AmbientColor = whitePiece ? irr::video::SColor(255, 190, 186, 174) : irr::video::SColor(255, 58, 62, 72);
        material.DiffuseColor = whitePiece ? irr::video::SColor(255, 238, 232, 214) : irr::video::SColor(255, 72, 78, 92);
        material.SpecularColor = whitePiece ? irr::video::SColor(255, 72, 68, 60) : irr::video::SColor(255, 48, 56, 76);
        material.EmissiveColor = whitePiece ? irr::video::SColor(255, 6, 6, 5) : irr::video::SColor(255, 1, 1, 2);
        material.Shininess = whitePiece ? 7.0f : 10.0f;
        material.MaterialTypeParam = 0.0f;
        material.ColorMaterial = irr::video::ECM_NONE;
        material.ZBuffer = irr::video::ECFN_LESSEQUAL;
        material.BackfaceCulling = true;
        material.GouraudShading = true;
    }
}

irr::core::vector3df PieceManager::ComputeMeshAnchor(const irr::scene::IAnimatedMesh* mesh) {
    if (!mesh) return irr::core::vector3df(0.0f, 0.0f, 0.0f);
    const irr::core::aabbox3df& box = mesh->getBoundingBox();
    return irr::core::vector3df((box.MinEdge.X + box.MaxEdge.X) * 0.5f,
                                box.MinEdge.Y,
                                (box.MinEdge.Z + box.MaxEdge.Z) * 0.5f);
}

irr::core::vector3df PieceManager::ComputeNodePosition(const irr::core::vector3df& squarePosition, const irr::core::vector3df& meshAnchor) {
    return squarePosition - meshAnchor;
}

std::string PieceManager::JoinPath(const std::string& base, const std::string& file) {
    return (std::filesystem::path(base) / file).string();
}

} // namespace chessit
