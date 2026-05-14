#pragma once

#include "Managers/BoardManager.h"

#include <irrlicht.h>
#include <string>

namespace chessit {

class ChessSceneManager {
public:
    bool LoadScene(irr::scene::ISceneManager* sceneManager, irr::video::IVideoDriver* driver, const BoardManager& boardManager, const std::string& mediaDir);

private:
    static std::string JoinPath(const std::string& base, const std::string& file);
    static irr::core::vector3df ComputeCameraTarget(const BoardManager& boardManager);
};

} // namespace chessit
