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

    auto meow = mg->getSettingValue<std::string>("normal-working-mode");
    auto uwu = mg->getSettingValue<std::string>("practice-working-mode");
    auto nyaa = mg->getSettingValue<std::string>("enby-working-mode");
    auto pt = mg->getSettingValue<bool>("practice-mode-toggle");
    auto po = mg->getSettingValue<bool>("practice-override");

    auto lp = as<float>(PlayLayer::get()->getCurrentPercentInt());
    auto lb = pl->m_level->m_normalPercent;

   // hello yanderedev
   if (lp > lb) {
    if (pm->m_isPracticeMode && po) {
        if (uwu == "Player Col 1") {
            return gm->colorForIdx(gm->getPlayerColor());
        } else if (uwu == "Player Col 2") {
            return gm->colorForIdx(gm->getPlayerColor2());
        } else if (uwu == "Player Glow") {
            return gm->colorForIdx(gm->getPlayerGlowColor());
        } else if (uwu == "Chroma") {
            return colorutil::getChromaColour();
        } else if (uwu == "Pastel") {
            return colorutil::getPastelColour();
        } else if (uwu == "Custom") {
            return Mod::get()->getSettingValue<ccColor3B>("practice-custom-color");
      //} else if (uwu == "Gradient") {
            // Placeholder
        }
    } else {
        if (nyaa == "Player Col 1") {
            return gm->colorForIdx(gm->getPlayerColor());
        } else if (nyaa == "Player Col 2") {
            return gm->colorForIdx(gm->getPlayerColor2());
        } else if (nyaa == "Player Glow") {
            return gm->colorForIdx(gm->getPlayerGlowColor());
        } else if (nyaa == "Chroma") {
            return colorutil::getChromaColour();
        } else if (nyaa == "Pastel") {
            return colorutil::getPastelColour();
        } else if (nyaa == "Custom") {
            return Mod::get()->getSettingValue<ccColor3B>("enby-custom-color");
      //} else if (nyaa == "Gradient") {
            // Placeholder
        }
    }
   } else {
    if (pm->m_isPracticeMode && pt) {    
        if (uwu == "Player Col 1") {
            return gm->colorForIdx(gm->getPlayerColor());
        } else if (uwu == "Player Col 2") {
            return gm->colorForIdx(gm->getPlayerColor2());
        } else if (uwu == "Player Glow") {
            return gm->colorForIdx(gm->getPlayerGlowColor());
        } else if (uwu == "Chroma") {
            return colorutil::getChromaColour();
        } else if (uwu == "Pastel") {
            return colorutil::getPastelColour();
        } else if (uwu == "Custom") {
            return Mod::get()->getSettingValue<ccColor3B>("practice-custom-color");
      //} else if (uwu == "Gradient") {
            // Placeholder
        }
    } else {
        if (meow == "Player Col 1") {
            return gm->colorForIdx(gm->getPlayerColor());
        } else if (meow == "Player Col 2") {
            return gm->colorForIdx(gm->getPlayerColor2());
        } else if (meow == "Player Glow") {
            return gm->colorForIdx(gm->getPlayerGlowColor());
        } else if (meow == "Chroma") {
            return colorutil::getChromaColour();
        } else if (meow == "Pastel") {
            return colorutil::getPastelColour();
        } else if (meow == "Custom") {
            return Mod::get()->getSettingValue<ccColor3B>("normal-custom-color");
      //} else if (meow == "Gradient") {
            // Placeholder
        }
    }
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
        colorutil::update(Mod::get()->getSettingValue<double_t>("speed"));
        // colorutil::va += CCDirector::get()->getDeltaTime();
        // colorutil::update(CCDirector::get()->getDeltaTime());
        if (!m_level->isPlatformer() && m_progressFill) {
            m_progressFill->setColor(paint());
        }
        
    }
};
