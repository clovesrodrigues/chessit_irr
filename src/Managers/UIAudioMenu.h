#pragma once

#include "Game/GameEvents.h"

#include <irrlicht.h>
#include <string>

namespace chessit {
class AIManager;
class SoundManager;

class UIAudioMenu {
public:
    void Initialize(irr::gui::IGUIEnvironment* guiEnvironment, SoundManager* soundManager, AIManager* aiManager, const std::string& configPath);
    bool OnEvent(const irr::SEvent& event);
    void SaveConfig() const;
    void UpdateAIStatus();
    AIDifficulty GetDifficulty() const { return difficulty_; }

private:
    void BuildUI();
    void LoadConfig();
    void ApplyConfigToUI();
    void PopulateMusicCombo();
    static std::wstring ToWide(const std::string& text);
    static std::string FileNameOnly(const std::string& path);

    irr::gui::IGUIEnvironment* guiEnvironment_ = nullptr;
    SoundManager* soundManager_ = nullptr;
    AIManager* aiManager_ = nullptr;
    irr::gui::IGUIWindow* window_ = nullptr;
    irr::gui::IGUIComboBox* musicCombo_ = nullptr;
    irr::gui::IGUIScrollBar* musicVolumeBar_ = nullptr;
    irr::gui::IGUIScrollBar* sfxVolumeBar_ = nullptr;
    irr::gui::IGUIComboBox* difficultyCombo_ = nullptr;
    irr::gui::IGUIStaticText* aiEngineText_ = nullptr;
    irr::gui::IGUIStaticText* aiStatusText_ = nullptr;
    irr::gui::IGUIStaticText* aiLastMoveText_ = nullptr;
    std::string lastAIEngineText_;
    std::string lastAIStatusText_;
    std::string lastAILastMoveText_;
    std::string configPath_ = "config.ini";
    AIDifficulty difficulty_ = AIDifficulty::Medium;
};

} // namespace chessit
