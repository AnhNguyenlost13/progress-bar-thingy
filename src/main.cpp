#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

ccColor3B paint() {
    auto gm = GameManager::sharedState(); // For convenience
    /**
    * Options:
    * - Default (vanilla behavior): P1 color
    * - Case 1: P2 color
    * - Case 2: Player glow color
    * - Case 3: Manual (configure in settings)
    */
    switch(Mod::get()->getSettingValue<int64_t>("mode")){
        case 1: 
            return gm->colorForIdx(gm->getPlayerColor2());
        case 2:
            return gm->colorForIdx(gm->getPlayerGlowColor());
        case 3:
            return Mod::get()->getSettingValue<ccColor3B>("color");
        default:
            return gm->colorForIdx(gm->getPlayerColor());
    }
}

class $modify(PlayLayer) {
    void startGame() {
        PlayLayer::startGame();
        if (!m_level->isPlatformer() && m_progressFill) {
            m_progressFill->setColor(paint());
        }
    }
};
