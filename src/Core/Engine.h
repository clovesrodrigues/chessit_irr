#pragma once

#include "Managers/AIManager.h"
#include "Managers/BillboardManager.h"
#include "Managers/BoardManager.h"
#include "Managers/ChessSceneManager.h"
#include "Managers/InputManager.h"
#include "Managers/ONNXAIManager.h"
#include "Managers/PieceManager.h"
#include "Managers/SaveReplaySystem.h"
#include "Managers/SoundManager.h"
#include "Managers/UIManager.h"

#include <irrlicht.h>
#include <string>

namespace chessit {

class Engine {
public:
    Engine();
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    bool Initialize(const std::string& mediaDir);
    void Run();
    void Shutdown();

private:
    bool CreateDevice();
    void LoadLogoTexture();
    void DrawStartupSplash();
    void DrawScreenLogo();
    void StartNewGame();

    std::string mediaDir_;
    irr::IrrlichtDevice* device_ = nullptr;
    irr::video::IVideoDriver* driver_ = nullptr;
    irr::scene::ISceneManager* sceneManager_ = nullptr;
    irr::gui::IGUIEnvironment* guiEnvironment_ = nullptr;
    irr::u32 lastFrameTimeMs_ = 0;
    irr::video::ITexture* logoTexture_ = nullptr;

    BoardManager boardManager_;
    ChessSceneManager chessSceneManager_;
    BillboardManager billboardManager_;
    PieceManager pieceManager_;
    InputManager inputManager_;
    AIManager aiManager_;
    SoundManager soundManager_;
    UIManager uiManager_;
    SaveReplaySystem saveReplaySystem_;
    ONNXAIManager onnxAIManager_;
};

} // namespace chessit
