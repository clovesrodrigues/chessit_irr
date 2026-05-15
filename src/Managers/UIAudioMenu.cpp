#include "Managers/UIAudioMenu.h"

#include "Core/Logger.h"
#include "Managers/AIManager.h"
#include "Managers/SoundManager.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace chessit {
namespace {
constexpr irr::s32 kWindowId = 5000;
constexpr irr::s32 kMusicComboId = 5001;
constexpr irr::s32 kMusicVolumeId = 5002;
constexpr irr::s32 kSFXVolumeId = 5003;
constexpr irr::s32 kDifficultyComboId = 5004;

AIDifficulty DifficultyFromInt(int value) {
    switch (value) {
        case 0: return AIDifficulty::Easy;
        case 2: return AIDifficulty::Hard;
        case 3: return AIDifficulty::Expert;
        default: return AIDifficulty::Medium;
    }
}

int DifficultyToInt(AIDifficulty difficulty) {
    switch (difficulty) {
        case AIDifficulty::Easy: return 0;
        case AIDifficulty::Hard: return 2;
        case AIDifficulty::Expert: return 3;
        default: return 1;
    }
}
}

void UIAudioMenu::Initialize(irr::gui::IGUIEnvironment* guiEnvironment, SoundManager* soundManager, AIManager* aiManager, const std::string& configPath) {
    guiEnvironment_ = guiEnvironment;
    soundManager_ = soundManager;
    aiManager_ = aiManager;
    configPath_ = configPath;
    LoadConfig();
    if (aiManager_) aiManager_->SetDifficulty(difficulty_);
    BuildUI();
    ApplyConfigToUI();
}

bool UIAudioMenu::OnEvent(const irr::SEvent& event) {
    if (event.EventType != irr::EET_GUI_EVENT || !event.GUIEvent.Caller) return false;
    const irr::s32 id = event.GUIEvent.Caller->getID();

    if (event.GUIEvent.EventType == irr::gui::EGET_SCROLL_BAR_CHANGED) {
        if (id == kMusicVolumeId && musicVolumeBar_ && soundManager_) {
            soundManager_->SetMusicVolume(static_cast<float>(musicVolumeBar_->getPos()) / 100.0f);
            SaveConfig();
            return true;
        }
        if (id == kSFXVolumeId && sfxVolumeBar_ && soundManager_) {
            soundManager_->SetSFXVolume(static_cast<float>(sfxVolumeBar_->getPos()) / 100.0f);
            SaveConfig();
            return true;
        }
    }

    if (event.GUIEvent.EventType == irr::gui::EGET_COMBO_BOX_CHANGED) {
        if (id == kMusicComboId && musicCombo_ && soundManager_) {
            const irr::s32 selected = musicCombo_->getSelected();
            if (selected >= 0) {
                const irr::u32 index = musicCombo_->getItemData(static_cast<irr::u32>(selected));
                const auto& tracks = soundManager_->GetAvailableMusic();
                if (index < tracks.size()) soundManager_->PlayMusic(tracks[index]);
            }
            SaveConfig();
            return true;
        }
        if (id == kDifficultyComboId && difficultyCombo_) {
            difficulty_ = DifficultyFromInt(difficultyCombo_->getSelected());
            if (aiManager_) aiManager_->SetDifficulty(difficulty_);
            UpdateAIStatus();
            SaveConfig();
            return true;
        }
    }
    return false;
}

void UIAudioMenu::BuildUI() {
    if (!guiEnvironment_) return;
    window_ = guiEnvironment_->addWindow(irr::core::rect<irr::s32>(20, 70, 390, 330), false, L"Audio / Config / IA", nullptr, kWindowId);
    if (!window_) return;

    guiEnvironment_->addStaticText(L"Music", irr::core::rect<irr::s32>(12, 28, 95, 48), false, false, window_);
    musicCombo_ = guiEnvironment_->addComboBox(irr::core::rect<irr::s32>(100, 26, 290, 50), window_, kMusicComboId);
    PopulateMusicCombo();

    guiEnvironment_->addStaticText(L"Music Vol", irr::core::rect<irr::s32>(12, 64, 95, 84), false, false, window_);
    musicVolumeBar_ = guiEnvironment_->addScrollBar(true, irr::core::rect<irr::s32>(100, 64, 290, 84), window_, kMusicVolumeId);
    if (musicVolumeBar_) { musicVolumeBar_->setMin(0); musicVolumeBar_->setMax(100); musicVolumeBar_->setSmallStep(5); }

    guiEnvironment_->addStaticText(L"SFX Vol", irr::core::rect<irr::s32>(12, 98, 95, 118), false, false, window_);
    sfxVolumeBar_ = guiEnvironment_->addScrollBar(true, irr::core::rect<irr::s32>(100, 98, 290, 118), window_, kSFXVolumeId);
    if (sfxVolumeBar_) { sfxVolumeBar_->setMin(0); sfxVolumeBar_->setMax(100); sfxVolumeBar_->setSmallStep(5); }

    guiEnvironment_->addStaticText(L"AI Level", irr::core::rect<irr::s32>(12, 134, 95, 154), false, false, window_);
    difficultyCombo_ = guiEnvironment_->addComboBox(irr::core::rect<irr::s32>(100, 132, 290, 156), window_, kDifficultyComboId);
    if (difficultyCombo_) {
        difficultyCombo_->addItem(L"Easy", 0);
        difficultyCombo_->addItem(L"Medium", 1);
        difficultyCombo_->addItem(L"Hard", 2);
        difficultyCombo_->addItem(L"Expert", 3);
    }

    guiEnvironment_->addStaticText(L"AI Engine", irr::core::rect<irr::s32>(12, 172, 95, 192), false, false, window_);
    aiEngineText_ = guiEnvironment_->addStaticText(L"checking...", irr::core::rect<irr::s32>(100, 172, 350, 194), false, true, window_);

    guiEnvironment_->addStaticText(L"AI Status", irr::core::rect<irr::s32>(12, 202, 95, 222), false, false, window_);
    aiStatusText_ = guiEnvironment_->addStaticText(L"checking...", irr::core::rect<irr::s32>(100, 202, 350, 244), false, true, window_);

    guiEnvironment_->addStaticText(L"Last Move", irr::core::rect<irr::s32>(12, 252, 95, 272), false, false, window_);
    aiLastMoveText_ = guiEnvironment_->addStaticText(L"none", irr::core::rect<irr::s32>(100, 252, 350, 286), false, true, window_);
    UpdateAIStatus();
}

void UIAudioMenu::UpdateAIStatus() {
    if (!aiManager_) return;

    const std::string engine = aiManager_->GetAIModeText();
    const std::string status = aiManager_->GetAIStatusText();
    const std::string lastMove = aiManager_->WasLastMoveNeural() ? "ONNX neural move" : "Fallback / none yet";

    if (aiEngineText_ && engine != lastAIEngineText_) {
        aiEngineText_->setText(ToWide(engine).c_str());
        lastAIEngineText_ = engine;
    }
    if (aiStatusText_ && status != lastAIStatusText_) {
        aiStatusText_->setText(ToWide(status).c_str());
        lastAIStatusText_ = status;
    }
    if (aiLastMoveText_ && lastMove != lastAILastMoveText_) {
        aiLastMoveText_->setText(ToWide(lastMove).c_str());
        lastAILastMoveText_ = lastMove;
    }
}

void UIAudioMenu::LoadConfig() {
    std::ifstream file(configPath_);
    if (!file.is_open() || !soundManager_) return;
    std::string line;
    while (std::getline(file, line)) {
        const auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        const std::string key = line.substr(0, eq);
        const std::string value = line.substr(eq + 1);
        if (key == "music_volume") {
            std::istringstream stream(value);
            float volume = 0.45f;
            if (stream >> volume) soundManager_->SetMusicVolume(volume);
        } else if (key == "sfx_volume") {
            std::istringstream stream(value);
            float volume = 0.80f;
            if (stream >> volume) soundManager_->SetSFXVolume(volume);
        } else if (key == "difficulty") {
            std::istringstream stream(value);
            int difficulty = 1;
            if (stream >> difficulty) difficulty_ = DifficultyFromInt(difficulty);
        } else if (key == "music" && !value.empty()) {
            soundManager_->PlayMusic(value);
        }
    }
}

void UIAudioMenu::ApplyConfigToUI() {
    if (!soundManager_) return;
    if (musicVolumeBar_) musicVolumeBar_->setPos(static_cast<irr::s32>(soundManager_->GetMusicVolume() * 100.0f));
    if (sfxVolumeBar_) sfxVolumeBar_->setPos(static_cast<irr::s32>(soundManager_->GetSFXVolume() * 100.0f));
    if (difficultyCombo_) difficultyCombo_->setSelected(DifficultyToInt(difficulty_));
    UpdateAIStatus();

    if (musicCombo_) {
        const auto& tracks = soundManager_->GetAvailableMusic();
        for (irr::u32 i = 0; i < tracks.size(); ++i) {
            if (tracks[i] == soundManager_->GetCurrentMusic()) {
                musicCombo_->setSelected(static_cast<irr::s32>(i));
                break;
            }
        }
    }
}

void UIAudioMenu::PopulateMusicCombo() {
    if (!musicCombo_ || !soundManager_) return;
    const auto& tracks = soundManager_->GetAvailableMusic();
    if (tracks.empty()) {
        musicCombo_->addItem(L"No music found", 0);
        return;
    }
    for (irr::u32 i = 0; i < tracks.size(); ++i) {
        musicCombo_->addItem(ToWide(FileNameOnly(tracks[i])).c_str(), i);
    }
}

void UIAudioMenu::SaveConfig() const {
    if (!soundManager_) return;
    std::ofstream file(configPath_, std::ios::trunc);
    if (!file.is_open()) return;
    file << "music_volume=" << soundManager_->GetMusicVolume() << '\n';
    file << "sfx_volume=" << soundManager_->GetSFXVolume() << '\n';
    file << "music=" << soundManager_->GetCurrentMusic() << '\n';
    file << "difficulty=" << DifficultyToInt(difficulty_) << '\n';
}

std::wstring UIAudioMenu::ToWide(const std::string& text) {
    return std::wstring(text.begin(), text.end());
}

std::string UIAudioMenu::FileNameOnly(const std::string& path) {
    return std::filesystem::path(path).filename().string();
}

} // namespace chessit
