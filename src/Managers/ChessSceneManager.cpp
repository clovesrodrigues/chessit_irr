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
    ApplyBoardMaterialStyle(boardNode);
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

    ConfigureSceneLighting(sceneManager, boardManager);

    if (fileSystem) fileSystem->changeWorkingDirectoryTo(previousWorkingDirectory);
    return loaded;
}

void ChessSceneManager::ApplyBoardMaterialStyle(irr::scene::ISceneNode* node) {
    if (!node) return;
    node->setMaterialFlag(irr::video::EMF_LIGHTING, true);
    node->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true);
    node->setMaterialType(irr::video::EMT_SOLID);
    for (irr::u32 i = 0; i < node->getMaterialCount(); ++i) {
        irr::video::SMaterial& material = node->getMaterial(i);
        material.AmbientColor = irr::video::SColor(255, 170, 170, 170);
        material.DiffuseColor = irr::video::SColor(255, 230, 225, 210);
        material.SpecularColor = irr::video::SColor(255, 160, 150, 130);
        material.EmissiveColor = irr::video::SColor(255, 8, 8, 8);
        material.Shininess = 18.0f;
        material.GouraudShading = true;
    }
}

void ChessSceneManager::ConfigureSceneLighting(irr::scene::ISceneManager* sceneManager, const BoardManager& boardManager) {
    if (!sceneManager) return;

    sceneManager->setAmbientLight(irr::video::SColorf(0.22f, 0.22f, 0.24f));
    const irr::core::vector3df board = boardManager.GetPositions().boardPosition;

    irr::scene::ILightSceneNode* keyLight = sceneManager->addLightSceneNode(
        nullptr, board + irr::core::vector3df(-4.0f, 12.0f, -7.0f), irr::video::SColorf(1.0f, 0.92f, 0.78f), 28.0f);
    if (keyLight) {
        irr::video::SLight& light = keyLight->getLightData();
        light.SpecularColor = irr::video::SColorf(1.0f, 0.95f, 0.85f);
        light.AmbientColor = irr::video::SColorf(0.08f, 0.07f, 0.06f);
    }

    irr::scene::ILightSceneNode* fillLight = sceneManager->addLightSceneNode(
        nullptr, board + irr::core::vector3df(5.5f, 7.0f, 6.0f), irr::video::SColorf(0.38f, 0.48f, 0.70f), 22.0f);
    if (fillLight) {
        irr::video::SLight& light = fillLight->getLightData();
        light.SpecularColor = irr::video::SColorf(0.35f, 0.45f, 0.75f);
        light.AmbientColor = irr::video::SColorf(0.03f, 0.04f, 0.06f);
    }

    irr::scene::ILightSceneNode* rimLight = sceneManager->addLightSceneNode(
        nullptr, board + irr::core::vector3df(0.0f, 6.0f, 9.0f), irr::video::SColorf(0.75f, 0.78f, 0.92f), 18.0f);
    if (rimLight) {
        irr::video::SLight& light = rimLight->getLightData();
        light.SpecularColor = irr::video::SColorf(0.65f, 0.70f, 1.0f);
    }
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
