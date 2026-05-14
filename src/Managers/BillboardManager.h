#pragma once

#include <irrlicht.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace chessit {

class BillboardManager {
public:
    bool Initialize(irr::scene::ISceneManager* sceneManager, irr::video::IVideoDriver* driver, const std::string& mediaDir);
    void Update(float deltaSeconds);
    void ShowBillboard(const std::string& key, const irr::core::vector3df& position, float size = 1.15f);
    void HideBillboard(const std::string& key);
    void HideGroupWithPrefix(const std::string& prefix);
    void HideAll();

private:
    struct BillboardEffect {
        irr::scene::IBillboardSceneNode* node = nullptr;
        std::string key;
        irr::core::vector3df targetPosition;
        float baseSize = 1.15f;
        float alpha = 0.0f;
        float pulseTime = 0.0f;
        bool active = false;
        bool fadingOut = false;
    };

    BillboardEffect* Acquire(const std::string& key);
    void ConfigureNode(BillboardEffect& effect);
    irr::video::ITexture* LoadSelectionTexture(const std::string& mediaDir);

    irr::scene::ISceneManager* sceneManager_ = nullptr;
    irr::video::IVideoDriver* driver_ = nullptr;
    irr::video::ITexture* selectionTexture_ = nullptr;
    std::vector<BillboardEffect> pool_;
    std::unordered_map<std::string, std::size_t> activeByKey_;
};

} // namespace chessit
