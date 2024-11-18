#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#define updateProgressBar updateProgressbar // sorry RobTop

#include "util/color.hpp" // Never thought I would see this day.

using namespace geode::prelude;

ccColor3B paint() {
    auto gm = GameManager::sharedState(); // For convenience
    auto ms = Mod::get()->getSettingValue<int64_t>("mode"); // This is stupid.
    /**
    * Options:
    * - Default (vanilla behavior): P1 color
    * I know it's quite weird to make a mod that doesn't do its job at first
    * - Case 1: P2 color
    * - Case 2: Player glow color
    * - Case 3: Manual (user)
    * 
    * Special cases:
    * - Case 4: RGB
    * - Case 5: Pastel
    */
    switch(ms){
        case 1: 
            return gm->colorForIdx(gm->getPlayerColor2());
        case 2:
            return gm->colorForIdx(gm->getPlayerGlowColor());
        case 3:
            return Mod::get()->getSettingValue<ccColor3B>("color");
        case 4:
            return colorutil::getChromaColour();
        case 5:
            return colorutil::getPastelColour();
        case 6:
            // Placeholder: Custom gradient mode, this may come at one point(TM)
        default:
            return gm->colorForIdx(gm->getPlayerColor());
    }
}

class $modify(PlayLayer) {
    void startGame() {
        PlayLayer::startGame();
        if (Mod::get()->getSettingValue<int64_t>("mode") < 4 && !m_level->isPlatformer() && m_progressFill) {
            m_progressFill->setColor(paint());
        }
    }
    
    void updateProgressBar() {
        PlayLayer::updateProgressBar();
        // Please kill me.
        colorutil::update(Mod::get()->getSettingValue<double_t>("speed"));
        // colorutil::va += CCDirector::get()->getDeltaTime();
        // colorutil::update(CCDirector::get()->getDeltaTime());
        if (!m_level->isPlatformer() && m_progressFill) {
            m_progressFill->setColor(paint());
        }
        
        float uwu = static_cast<float>(PlayLayer::getCurrentPercentInt());
        if (uwu > (m_level->m_normalPercent)) {
            // later
        }
    }

    void togglePracticeMode(bool practiceMode) {
        PlayLayer::togglePracticeMode(practiceMode);
        if (practiceMode == true) {
        // hi
        } else {
        // meow
        }
    }
};
