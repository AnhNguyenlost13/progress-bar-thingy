#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#define updateProgressBar updateProgressbar // sorry RobTop

#include "util/color.hpp" // Thanks TheSillyDoggo

using namespace geode::prelude;

ccColor3B paint() {
    auto gm = GameManager::sharedState(); // For convenience
    auto pm = GJBaseGameLayer::get();
    auto pl = PlayLayer::get();
    auto mg = Mod::get();

    auto normalWorkingMode = mg->getSettingValue<std::string>("normal-working-mode");
    auto practiceWorkingMode = mg->getSettingValue<std::string>("practice-working-mode");
    auto newBestWorkingMode = mg->getSettingValue<std::string>("enby-working-mode");
    auto pt = mg->getSettingValue<bool>("practice-mode-toggle");
    auto po = mg->getSettingValue<bool>("practice-override");

    auto lp = as<float>(PlayLayer::get()->getCurrentPercentInt());
    auto lb = pl->m_level->m_normalPercent;

   // hello yanderedev
   if (lp > lb) {
    if (pm->m_isPracticeMode && po) {
        if (practiceWorkingMode == "Player Col 1") {
            return gm->colorForIdx(gm->getPlayerColor());
        } else if (practiceWorkingMode == "Player Col 2") {
            return gm->colorForIdx(gm->getPlayerColor2());
        } else if (practiceWorkingMode == "Player Glow") {
            return gm->colorForIdx(gm->getPlayerGlowColor());
        } else if (practiceWorkingMode == "Chroma") {
            return colorutil::getChromaColour();
        } else if (practiceWorkingMode == "Pastel") {
            return colorutil::getPastelColour();
        } else if (practiceWorkingMode == "Custom") {
            return Mod::get()->getSettingValue<ccColor3B>("practice-custom-color");
      //} else if (practiceWorkingMode == "Gradient") {
            // Placeholder
        }
    } else {
        if (newBestWorkingMode == "Player Col 1") {
            return gm->colorForIdx(gm->getPlayerColor());
        } else if (newBestWorkingMode == "Player Col 2") {
            return gm->colorForIdx(gm->getPlayerColor2());
        } else if (newBestWorkingMode == "Player Glow") {
            return gm->colorForIdx(gm->getPlayerGlowColor());
        } else if (newBestWorkingMode == "Chroma") {
            return colorutil::getChromaColour();
        } else if (newBestWorkingMode == "Pastel") {
            return colorutil::getPastelColour();
        } else if (newBestWorkingMode == "Custom") {
            return Mod::get()->getSettingValue<ccColor3B>("enby-custom-color");
      //} else if (newBestWorkingMode == "Gradient") {
            // Placeholder
        }
    }
   } else {
    if (pm->m_isPracticeMode && pt) {    
        if (practiceWorkingMode == "Player Col 1") {
            return gm->colorForIdx(gm->getPlayerColor());
        } else if (practiceWorkingMode == "Player Col 2") {
            return gm->colorForIdx(gm->getPlayerColor2());
        } else if (practiceWorkingMode == "Player Glow") {
            return gm->colorForIdx(gm->getPlayerGlowColor());
        } else if (practiceWorkingMode == "Chroma") {
            return colorutil::getChromaColour();
        } else if (practiceWorkingMode == "Pastel") {
            return colorutil::getPastelColour();
        } else if (practiceWorkingMode == "Custom") {
            return Mod::get()->getSettingValue<ccColor3B>("practice-custom-color");
      //} else if (practiceWorkingMode == "Gradient") {
            // Placeholder
        }
    } else {
        if (normalWorkingMode == "Player Col 1") {
            return gm->colorForIdx(gm->getPlayerColor());
        } else if (normalWorkingMode == "Player Col 2") {
            return gm->colorForIdx(gm->getPlayerColor2());
        } else if (normalWorkingMode == "Player Glow") {
            return gm->colorForIdx(gm->getPlayerGlowColor());
        } else if (normalWorkingMode == "Chroma") {
            return colorutil::getChromaColour();
        } else if (normalWorkingMode == "Pastel") {
            return colorutil::getPastelColour();
        } else if (normalWorkingMode == "Custom") {
            return Mod::get()->getSettingValue<ccColor3B>("normal-custom-color");
      //} else if (normalWorkingMode == "Gradient") {
            // Placeholder
        }
    }
   }
   return {255, 255, 255}; // Congrats!
}

class $modify(PlayLayer) {
    void startGame() {
        PlayLayer::startGame();
        if (!m_level->isPlatformer() && m_progressFill) {
            m_progressFill->setColor(paint());
        }
    }
    
    void updateProgressBar() {
        PlayLayer::updateProgressBar();
        colorutil::update(Mod::get()->getSettingValue<double_t>("speed"));
        // colorutil::va += CCDirector::get()->getDeltaTime();
        // colorutil::update(CCDirector::get()->getDeltaTime());
        if (!m_level->isPlatformer() && m_progressFill) {
            m_progressFill->setColor(paint());
        }
        
    }
};
