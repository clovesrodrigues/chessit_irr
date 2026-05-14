#include "Managers/ChessSceneManager.h"

#include "Core/Logger.h"

#include <filesystem>

namespace chessit {

bool ChessSceneManager::LoadScene(irr::scene::ISceneManager* sceneManager, irr::video::IVideoDriver* driver, const BoardManager& boardManager, const std::string& mediaDir) {
    if (!sceneManager || !driver) {
        Logger::Error("ChessSceneManager requires a valid scene manager and video driver.");
        return false;
    }

    irr::io::IFileSystem* fileSystem = sceneManager->getFileSystem();
    const irr::io::path previousWorkingDirectory = fileSystem ? fileSystem->getWorkingDirectory() : irr::io::path();
    
    // Try to change to media directory if possible, but don't fail if we can't
    if (fileSystem && !fileSystem->changeWorkingDirectoryTo(mediaDir.c_str())) {
        Logger::Warning("Could not switch Irrlicht working directory to media folder: " + mediaDir);
    }

    bool loaded = true;
    
    // Load environment scene with full path
    const std::string envScenePath = JoinPath(mediaDir, "env.irr");
    if (!sceneManager->loadScene(envScenePath.c_str())) {
        Logger::Warning("Could not load optional environment scene: " + envScenePath);
        loaded = false;
    } else {
        Logger::Info("Environment scene loaded: " + envScenePath);
    }

    // Load board mesh with full path
    const std::string boardMeshPath = JoinPath(mediaDir, "BOARDER.obj");
    irr::scene::IAnimatedMesh* boardMesh = sceneManager->getMesh(boardMeshPath.c_str());
    if (!boardMesh) {
        Logger::Error("Could not load board mesh: " + boardMeshPath);
        if (fileSystem) fileSystem->changeWorkingDirectoryTo(previousWorkingDirectory);
        return false;
    }

    irr::scene::ISceneNode* boardNode = sceneManager->addMeshSceneNode(boardMesh);
    if (!boardNode) {
        Logger::Error("Could not create board scene node.");
        if (fileSystem) fileSystem->changeWorkingDirectoryTo(previousWorkingDirectory);
        return false;
    }

    boardNode->setName("BOARDER");
    boardNode->setPosition(boardManager.GetPositions().boardPosition);
    boardNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
    boardNode->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true);
    Logger::Info("Board mesh loaded: " + boardMeshPath);

    const CameraConfig& cameraConfig = boardManager.GetPositions().camera;
    const irr::core::vector3df cameraTarget = cameraConfig.hasTarget ? cameraConfig.target : ComputeCameraTarget(boardManager);
    irr::scene::ICameraSceneNode* camera = sceneManager->addCameraSceneNode(nullptr, cameraConfig.position, cameraTarget);
    if (!camera) {
        Logger::Error("Could not create camera scene node.");
        if (fileSystem) fileSystem->changeWorkingDirectoryTo(previousWorkingDirectory);
        return false;
    }
    camera->setFOV(cameraConfig.fovY);
    camera->setAspectRatio(cameraConfig.aspect);
    sceneManager->setActiveCamera(camera);
    Logger::Info("Camera controls: right mouse rotates, middle mouse pans, mouse wheel moves, P logs the current CAMERA line.");

    sceneManager->addLightSceneNode(nullptr, irr::core::vector3df(0.0f, -10.0f, 0.0f), irr::video::SColorf(1.0f, 1.0f, 1.0f), 120.0f);

    if (fileSystem) fileSystem->changeWorkingDirectoryTo(previousWorkingDirectory);
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
