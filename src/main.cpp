#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

#define updateProgressBar updateProgressbar // I blame RobTop.
#include "util/color.hpp" // Thanks TheSillyDoggo

using namespace geode::prelude;

// TODO: change the variable names cuz WHAT IS THIS

enum class WorkingMode {
    PlayerCol1,
    PlayerCol2,
    PlayerGlow,
    Chroma,
    Pastel,
    Custom,
    // Gradient, // who knows
    Unknown     // This should never happen either lmao
};


WorkingMode getWorkingMode(const std::string& mode) {
    static const std::unordered_map<std::string, WorkingMode> modeMap = {
        {"Player Col 1", WorkingMode::PlayerCol1},
        {"Player Col 2", WorkingMode::PlayerCol2},
        {"Player Glow", WorkingMode::PlayerGlow},
        {"Chroma", WorkingMode::Chroma},
        {"Pastel", WorkingMode::Pastel},
        {"Custom", WorkingMode::Custom}
        // {"Gradient", WorkingMode::Gradient} /* Placeholder */
    };

    auto idkWhatToNameThis = modeMap.find(mode);
    return idkWhatToNameThis != modeMap.end() ? idkWhatToNameThis->second : WorkingMode::Unknown;
}

// Get the color for a given working mode
ccColor3B getColor(WorkingMode mode, const std::string& customColorKey) {
    auto gm = GameManager::sharedState();
    switch (mode) {
        case WorkingMode::PlayerCol1:
            return gm->colorForIdx(gm->getPlayerColor());
        case WorkingMode::PlayerCol2:
            return gm->colorForIdx(gm->getPlayerColor2());
        case WorkingMode::PlayerGlow:
            return gm->colorForIdx(gm->getPlayerGlowColor());
        case WorkingMode::Chroma:
            return colorutil::getChromaColour();
        case WorkingMode::Pastel:
            return colorutil::getPastelColour();
        case WorkingMode::Custom:
            return Mod::get()->getSettingValue<ccColor3B>(customColorKey);
        default:
            return {255, 255, 255}; // Default to white, though this should never happen
    }
}

ccColor3B paint() {
    auto gm = GameManager::sharedState(); // For convenience
    auto gjbgl = GJBaseGameLayer::get();
    auto pl = PlayLayer::get();
    auto mg = Mod::get();

    auto meow = getWorkingMode(mg->getSettingValue<std::string>("normal-working-mode"));
    auto uwu = getWorkingMode(mg->getSettingValue<std::string>("practice-working-mode"));
    auto nyaa = getWorkingMode(mg->getSettingValue<std::string>("enby-working-mode"));
    auto pt = mg->getSettingValue<bool>("practice-mode-toggle");
    auto po = mg->getSettingValue<bool>("practice-override");

    auto lp = as<float>(PlayLayer::get()->getCurrentPercentInt()); // Level progress
    auto lb = pl->m_level->m_normalPercent; // Level best

    if (lp > lb) {
        if (gjbgl->m_isPracticeMode && po) {
            return getColor(uwu, "practice-custom-color");
        } else {
            return getColor(nyaa, "enby-custom-color");
        }
    } else {
        if (gjbgl->m_isPracticeMode && pt) {
            return getColor(uwu, "practice-custom-color");
        } else {
            return getColor(meow, "normal-custom-color");
        }
    }
}

float getSpeed() {
    auto mg = Mod::get();
    // auto gm = GameManager::sharedState();
    auto pl = PlayLayer::get();
    auto pm = GJBaseGameLayer::get();

    // auto meow = getWorkingMode(mg->getSettingValue<std::string>("normal-working-mode"));
    // auto uwu = getWorkingMode(mg->getSettingValue<std::string>("practice-working-mode"));
    // auto nyaa = getWorkingMode(mg->getSettingValue<std::string>("enby-working-mode"));
    auto pt = mg->getSettingValue<bool>("practice-mode-toggle");
    auto po = mg->getSettingValue<bool>("practice-override");

    auto lp = as<float>(PlayLayer::get()->getCurrentPercentInt());
    auto lb = pl->m_level->m_normalPercent;

    if (lp > lb) {
        if (pm->m_isPracticeMode && po) {
            return mg->getSettingValue<double_t>("practice-rgb-speed");
        } else {
            return mg->getSettingValue<double_t>("enby-rgb-speed");
        }
    } else {
        if (pm->m_isPracticeMode && pt) {
            return mg->getSettingValue<double_t>("practice-rgb-speed");
        } else {
            return mg->getSettingValue<double_t>("normal-rgb-speed");
        }
    }
}

class $modify(PlayLayer) {
    void startGame() {
        PlayLayer::startGame();
        if (Mod::get()->getSettingValue<int64_t>("mode") < 4 && !m_level->isPlatformer() && m_progressFill) {
            m_progressFill->setColor(paint()); // Whatever.
        }
    }
    
    // Really there's no better way to do this as far as I know
    // After all we have to check the player's percentage every frame for the change on new best feature
    // And recoloring for the RGB modes
    void updateProgressBar() {
        PlayLayer::updateProgressBar();
        colorutil::update(getSpeed());
        
        if (!m_level->isPlatformer() && m_progressFill) {
            m_progressFill->setColor(paint());
        }
    }
};
