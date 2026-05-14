#include "Managers/BillboardManager.h"

#include "Core/Logger.h"

#include <algorithm>
#include <cmath>
#include <filesystem>

namespace chessit {
namespace {
constexpr float kFadeInSpeed = 6.0f;
constexpr float kFadeOutSpeed = 4.5f;
constexpr irr::s32 kBillboardNodeId = 0x40000000;
}

bool BillboardManager::Initialize(irr::scene::ISceneManager* sceneManager, irr::video::IVideoDriver* driver, const std::string& mediaDir) {
    sceneManager_ = sceneManager;
    driver_ = driver;
    selectionTexture_ = LoadSelectionTexture(mediaDir);
    if (!sceneManager_ || !driver_ || !selectionTexture_) {
        Logger::Error("BillboardManager could not initialize selection texture.");
        return false;
    }
    pool_.reserve(80);
    Logger::Info("BillboardManager initialized with transparent particle texture.");
    return true;
}

void BillboardManager::Update(float deltaSeconds) {
    for (std::size_t i = 0; i < pool_.size(); ++i) {
        BillboardEffect& effect = pool_[i];
        if (!effect.node || (!effect.active && effect.alpha <= 0.0f)) continue;

        effect.pulseTime += deltaSeconds;
        effect.alpha += (effect.fadingOut ? -kFadeOutSpeed : kFadeInSpeed) * deltaSeconds;
        effect.alpha = std::max(0.0f, std::min(1.0f, effect.alpha));

        if (effect.fadingOut && effect.alpha <= 0.0f) {
            effect.active = false;
            effect.fadingOut = false;
            effect.node->setVisible(false);
            activeByKey_.erase(effect.key);
            continue;
        }

        const float pulse = 1.0f + 0.12f * std::sin(effect.pulseTime * 5.5f);
        const float size = effect.baseSize * pulse;
        const irr::u32 alpha = static_cast<irr::u32>(220.0f * effect.alpha);
        effect.node->setPosition(effect.targetPosition);
        effect.node->setSize(irr::core::dimension2df(size, size));
        effect.node->setColor(irr::video::SColor(alpha, 255, 255, 255));
        effect.node->setVisible(true);
    }
}

void BillboardManager::ShowBillboard(const std::string& key, const irr::core::vector3df& position, float size) {
    BillboardEffect* effect = Acquire(key);
    if (!effect) return;
    effect->targetPosition = position;
    effect->baseSize = size;
    effect->active = true;
    effect->fadingOut = false;
    ConfigureNode(*effect);
}

void BillboardManager::HideBillboard(const std::string& key) {
    const auto it = activeByKey_.find(key);
    if (it == activeByKey_.end() || it->second >= pool_.size()) return;
    pool_[it->second].fadingOut = true;
}

void BillboardManager::HideGroupWithPrefix(const std::string& prefix) {
    std::vector<std::string> keys;
    for (const auto& pair : activeByKey_) {
        if (pair.first.rfind(prefix, 0) == 0) keys.push_back(pair.first);
    }
    for (const std::string& key : keys) HideBillboard(key);
}

void BillboardManager::HideAll() {
    for (BillboardEffect& effect : pool_) {
        if (effect.node) {
            effect.active = false;
            effect.fadingOut = false;
            effect.alpha = 0.0f;
            effect.node->setVisible(false);
        }
    }
    activeByKey_.clear();
}

BillboardManager::BillboardEffect* BillboardManager::Acquire(const std::string& key) {
    const auto found = activeByKey_.find(key);
    if (found != activeByKey_.end() && found->second < pool_.size()) return &pool_[found->second];

    for (std::size_t i = 0; i < pool_.size(); ++i) {
        if (!pool_[i].active && pool_[i].alpha <= 0.0f) {
            pool_[i].key = key;
            activeByKey_[key] = i;
            return &pool_[i];
        }
    }

    BillboardEffect effect;
    effect.node = sceneManager_->addBillboardSceneNode(nullptr, irr::core::dimension2df(1.0f, 1.0f));
    if (!effect.node) return nullptr;
    effect.node->setID(kBillboardNodeId);
    effect.node->setVisible(false);
    effect.node->setAutomaticCulling(irr::scene::EAC_OFF);
    pool_.push_back(effect);
    const std::size_t index = pool_.size() - 1;
    pool_[index].key = key;
    activeByKey_[key] = index;
    return &pool_[index];
}

void BillboardManager::ConfigureNode(BillboardEffect& effect) {
    if (!effect.node) return;
    effect.node->setMaterialTexture(0, selectionTexture_);
    effect.node->setMaterialFlag(irr::video::EMF_LIGHTING, false);
    effect.node->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, false);
    effect.node->setMaterialFlag(irr::video::EMF_BACK_FACE_CULLING, false);
    effect.node->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR);
    effect.node->setPosition(effect.targetPosition);
}

irr::video::ITexture* BillboardManager::LoadSelectionTexture(const std::string& mediaDir) {
    if (!driver_) return nullptr;
    const std::string texturePath = (std::filesystem::path(mediaDir) / "particlewhite.bmp").string();
    const bool oldMipMap = driver_->getTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS);
    driver_->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);
    irr::video::ITexture* texture = driver_->getTexture(texturePath.c_str());
    driver_->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, oldMipMap);
    if (!texture) {
        Logger::Error("Could not load billboard texture: " + texturePath);
        return nullptr;
    }
    driver_->makeColorKeyTexture(texture, irr::core::position2di(0, 0), true);
    driver_->makeColorKeyTexture(texture, irr::video::SColor(255, 0, 0, 0), true);
    return texture;
}

} // namespace chessit
