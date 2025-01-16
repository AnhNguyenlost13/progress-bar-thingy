#pragma once

#include "utils.hpp" // Thanks TheSillyDoggo
#define updateProgressBar updateProgressbar // I blame RobTop.

#include <Geode/modify/PlayLayer.hpp>
class $modify(PlayLayer) {    
    void updateProgressBar() {
        PlayLayer::updateProgressBar();
        colorutil::update(getSpeed());
         
        if (!m_level->isPlatformer() && m_progressFill && m_progressBar->isVisible()) {
            m_progressFill->setColor(paint());
        }
    }

    // TODO: globed compat thanks
};