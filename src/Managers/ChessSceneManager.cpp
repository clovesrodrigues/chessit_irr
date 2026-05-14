#include "Managers/ChessSceneManager.h"

#include "Core/Logger.h"

#include <filesystem>

namespace chessit {

bool ChessSceneManager::LoadScene(irr::scene::ISceneManager* sceneManager, irr::video::IVideoDriver* driver, const BoardManager& boardManager, const std::string& mediaDir) {
    if (!sceneManager || !driver) {
        Logger::Error("ChessSceneManager requires a valid scene manager and video driver.");
        return false;
    }

    bool loaded = true;
    const std::string envPath = JoinPath(mediaDir, "env.irr");
    if (!sceneManager->loadScene(envPath.c_str())) {
        Logger::Warning("Could not load optional environment scene: " + envPath);
        loaded = false;
    } else {
        Logger::Info("Environment scene loaded: " + envPath);
    }

    const std::string boardPath = JoinPath(mediaDir, "BOARDER.obj");
    irr::scene::IAnimatedMesh* boardMesh = sceneManager->getMesh(boardPath.c_str());
    if (!boardMesh) {
        Logger::Error("Could not load board mesh: " + boardPath);
        return false;
    }

    irr::scene::ISceneNode* boardNode = sceneManager->addMeshSceneNode(boardMesh);
    if (!boardNode) {
        Logger::Error("Could not create board scene node.");
        return false;
    }

    boardNode->setName("BOARDER");
    boardNode->setPosition(boardManager.GetPositions().boardPosition);
    boardNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
    boardNode->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true);
    Logger::Info("Board mesh loaded: " + boardPath);

    irr::scene::ICameraSceneNode* camera = sceneManager->addCameraSceneNode(nullptr, boardManager.GetPositions().camera.position, ComputeCameraTarget(boardManager));
    if (!camera) {
        Logger::Error("Could not create camera scene node.");
        return false;
    }
    camera->setFOV(boardManager.GetPositions().camera.fovY);
    camera->setAspectRatio(boardManager.GetPositions().camera.aspect);
    sceneManager->setActiveCamera(camera);

    sceneManager->addLightSceneNode(nullptr, irr::core::vector3df(0.0f, -10.0f, 0.0f), irr::video::SColorf(1.0f, 1.0f, 1.0f), 120.0f);
    return loaded;
}

std::string ChessSceneManager::JoinPath(const std::string& base, const std::string& file) {
    return (std::filesystem::path(base) / file).string();
}

irr::core::vector3df ChessSceneManager::ComputeCameraTarget(const BoardManager& boardManager) {
    if (const irr::core::vector3df* d4 = boardManager.GetSquarePosition("D4")) {
        if (const irr::core::vector3df* e5 = boardManager.GetSquarePosition("E5")) {
            return (*d4 + *e5) * 0.5f;
        }
        return *d4;
    }
    return boardManager.GetPositions().boardPosition;
}

} // namespace chessit
