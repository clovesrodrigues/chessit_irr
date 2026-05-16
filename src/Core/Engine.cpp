#include "Core/Engine.h"

#include "Core/Logger.h"

#include <algorithm>
#include <filesystem>

namespace chessit {
namespace {
constexpr irr::u32 SplashDurationMs = 7000;
constexpr float SplashStartScreenRatio = 0.55f;
constexpr float SplashEndScreenRatio = 0.85f;
constexpr float SplashZoomMultiplier = 1.35f;
constexpr irr::s32 ScreenLogoSizePx = 175;
constexpr irr::s32 ScreenLogoMarginPx = 20;
} // namespace

Engine::Engine() = default;
Engine::~Engine() { Shutdown(); }

bool Engine::Initialize(const std::string& mediaDir) {
    mediaDir_ = mediaDir;
    if (!CreateDevice()) return false;

    LoadLogoTexture();
    DrawStartupSplash();

    const std::string positionsPath = (std::filesystem::path(mediaDir_) / "CHESSIT_POSITIONS.txt").string();
    if (!boardManager_.LoadPositions(positionsPath)) return false;

    if (!chessSceneManager_.LoadScene(sceneManager_, driver_, boardManager_, mediaDir_)) {
        Logger::Error("Scene loading failed.");
        return false;
    }

    if (!billboardManager_.Initialize(sceneManager_, driver_, mediaDir_)) {
        Logger::Error("Billboard manager initialization failed.");
        return false;
    }

    if (!pieceManager_.LoadInitialPieces(sceneManager_, boardManager_, mediaDir_)) {
        Logger::Error("One or more pieces failed to load.");
        return false;
    }

    soundManager_.Initialize(mediaDir_);
    saveReplaySystem_.Initialize();
    onnxAIManager_.Initialize();

    const std::filesystem::path modelPath = std::filesystem::path("bin") / "chessit_ai.onnx";
    const std::filesystem::path localModelPath = "chessit_ai.onnx";
    if (std::filesystem::exists(modelPath)) {
        onnxAIManager_.LoadModel(modelPath.string());
    } else if (std::filesystem::exists(localModelPath)) {
        onnxAIManager_.LoadModel(localModelPath.string());
    } else {
        Logger::Info("ONNX model not found. Computer will use fallback capture AI.");
    }

    aiManager_.Initialize(&boardManager_, &pieceManager_, &soundManager_, &onnxAIManager_, &saveReplaySystem_);
    uiManager_.Initialize(guiEnvironment_, driver_, mediaDir_, &soundManager_, &aiManager_);
    inputManager_.Initialize(device_,
                             sceneManager_,
                             &pieceManager_,
                             &boardManager_,
                             &billboardManager_,
                             &soundManager_,
                             &uiManager_,
                             &aiManager_,
                             &saveReplaySystem_);

    Logger::Info("ChessIt 3D engine initialized.");
    return true;
}

void Engine::Run() {
    if (!device_ || !driver_ || !sceneManager_ || !guiEnvironment_) return;

    lastFrameTimeMs_ = device_->getTimer()->getTime();
    while (device_->run()) {
        if (device_->isWindowActive()) {
            const irr::u32 now = device_->getTimer()->getTime();
            const float deltaSeconds = static_cast<float>(now - lastFrameTimeMs_) / 1000.0f;
            lastFrameTimeMs_ = now;

            inputManager_.Update(deltaSeconds);
            aiManager_.Update();
            billboardManager_.Update(deltaSeconds);
            uiManager_.Update();
            if (uiManager_.ConsumeNewGameRequested()) StartNewGame();

            driver_->beginScene(true, true, irr::video::SColor(255, 24, 24, 30));
            sceneManager_->drawAll();
            uiManager_.DrawOverlay();
            guiEnvironment_->drawAll();
            DrawScreenLogo();
            driver_->endScene();
        } else {
            device_->yield();
        }
    }
}

void Engine::LoadLogoTexture() {
    if (!driver_) return;

    const std::string logoPath = (std::filesystem::path(mediaDir_) / "LOGO.png").string();
    logoTexture_ = driver_->getTexture(logoPath.c_str());
    if (!logoTexture_) {
        Logger::Error("Failed to load logo texture: " + logoPath);
    }
}

void Engine::DrawStartupSplash() {
    if (!device_ || !driver_) return;

    const irr::u32 startTimeMs = device_->getTimer()->getTime();
    while (device_->run()) {
        const irr::u32 now = device_->getTimer()->getTime();
        const irr::u32 elapsedMs = now - startTimeMs;
        if (elapsedMs >= SplashDurationMs) {
            break;
        }

        if (!device_->isWindowActive()) {
            device_->yield();
            continue;
        }

        driver_->beginScene(true, true, irr::video::SColor(255, 0, 0, 0));
        if (logoTexture_) {
            const irr::core::dimension2du screenSize = driver_->getScreenSize();
            const irr::core::dimension2du logoSize = logoTexture_->getOriginalSize();
            const float progress = static_cast<float>(elapsedMs) / static_cast<float>(SplashDurationMs);
            const float startScaleLimit = std::min(
                (static_cast<float>(screenSize.Width) * SplashStartScreenRatio) / static_cast<float>(logoSize.Width),
                (static_cast<float>(screenSize.Height) * SplashStartScreenRatio) / static_cast<float>(logoSize.Height));
            const float startScale = std::min(startScaleLimit, 1.0f);
            const float endScaleLimit = std::min(
                (static_cast<float>(screenSize.Width) * SplashEndScreenRatio) / static_cast<float>(logoSize.Width),
                (static_cast<float>(screenSize.Height) * SplashEndScreenRatio) / static_cast<float>(logoSize.Height));
            const float endScale = std::clamp(startScale * SplashZoomMultiplier, startScale, endScaleLimit);
            const float scale = startScale + ((endScale - startScale) * progress);
            const irr::s32 scaledWidth = static_cast<irr::s32>(static_cast<float>(logoSize.Width) * scale);
            const irr::s32 scaledHeight = static_cast<irr::s32>(static_cast<float>(logoSize.Height) * scale);
            const irr::s32 left = (static_cast<irr::s32>(screenSize.Width) - scaledWidth) / 2;
            const irr::s32 top = (static_cast<irr::s32>(screenSize.Height) - scaledHeight) / 2;

            driver_->draw2DImage(
                logoTexture_,
                irr::core::rect<irr::s32>(left, top, left + scaledWidth, top + scaledHeight),
                irr::core::rect<irr::s32>(0, 0, static_cast<irr::s32>(logoSize.Width), static_cast<irr::s32>(logoSize.Height)),
                nullptr,
                nullptr,
                true);
        }
        driver_->endScene();
    }
}

void Engine::DrawScreenLogo() {
    if (!driver_ || !logoTexture_) return;

    const irr::core::dimension2du screenSize = driver_->getScreenSize();
    const irr::core::dimension2du textureSize = logoTexture_->getOriginalSize();
    const irr::s32 left = static_cast<irr::s32>(screenSize.Width) - ScreenLogoSizePx - ScreenLogoMarginPx;
    const irr::s32 top = static_cast<irr::s32>(screenSize.Height) - ScreenLogoSizePx - ScreenLogoMarginPx;

    driver_->draw2DImage(
        logoTexture_,
        irr::core::rect<irr::s32>(left, top, left + ScreenLogoSizePx, top + ScreenLogoSizePx),
        irr::core::rect<irr::s32>(0, 0, static_cast<irr::s32>(textureSize.Width), static_cast<irr::s32>(textureSize.Height)),
        nullptr,
        nullptr,
        true);
}

void Engine::StartNewGame() {
    inputManager_.ResetInteractionState();
    billboardManager_.HideAll();
    pieceManager_.LoadInitialPieces(sceneManager_, boardManager_, mediaDir_);
    aiManager_.Initialize(&boardManager_, &pieceManager_, &soundManager_, &onnxAIManager_, &saveReplaySystem_);
    saveReplaySystem_.StartNewGame();
    uiManager_.HideGameOver();
    Logger::Info("New game started.");
}

void Engine::Shutdown() {
    soundManager_.Shutdown();
    if (device_) {
        Logger::Info("Dropping Irrlicht device.");
        device_->drop();
        device_ = nullptr;
        driver_ = nullptr;
        sceneManager_ = nullptr;
        guiEnvironment_ = nullptr;
    }
}

bool Engine::CreateDevice() {
    irr::SIrrlichtCreationParameters params;
    params.DriverType = irr::video::EDT_OPENGL;
    params.WindowSize = irr::core::dimension2du(1280, 720);
    params.Bits = 32;
    params.Fullscreen = false;
    params.Stencilbuffer = true;
    params.Vsync = true;
    params.AntiAlias = 4;
    params.EventReceiver = &inputManager_;

    device_ = irr::createDeviceEx(params);
    if (!device_) {
        Logger::Error("Failed to create Irrlicht device.");
        return false;
    }

    device_->setWindowCaption(L"___   ChessIt 3D - by Cloves Rodrigues   ___");
    driver_ = device_->getVideoDriver();
    sceneManager_ = device_->getSceneManager();
    guiEnvironment_ = device_->getGUIEnvironment();

    if (!driver_ || !sceneManager_ || !guiEnvironment_) {
        Logger::Error("Irrlicht subsystems are not available.");
        return false;
    }
    return true;
}

} // namespace chessit
