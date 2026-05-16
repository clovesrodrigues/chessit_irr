#pragma once

#include <irrlicht.h>
#include <string>

namespace chessit {

class AIManager;
class BillboardManager;
class BoardManager;
class ChessSceneManager;
class InputManager;
class ONNXAIManager;
class PieceManager;
class SaveReplaySystem;
class SoundManager;
class UIManager;

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

    std::unique_ptr<BoardManager> boardManager_;
    std::unique_ptr<ChessSceneManager> chessSceneManager_;
    std::unique_ptr<BillboardManager> billboardManager_;
    std::unique_ptr<PieceManager> pieceManager_;
    std::unique_ptr<InputManager> inputManager_;
    std::unique_ptr<AIManager> aiManager_;
    std::unique_ptr<SoundManager> soundManager_;
    std::unique_ptr<UIManager> uiManager_;
    std::unique_ptr<SaveReplaySystem> saveReplaySystem_;
    std::unique_ptr<ONNXAIManager> onnxAIManager_;
};

} // namespace chessit
