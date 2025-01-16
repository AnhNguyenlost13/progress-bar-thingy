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
            // beautiful. kill it with fire.
            if (auto globedProgressBarWrapper = m_progressBar->getChildByID("dankmeme.globed2/progress-bar-wrapper")) { if (auto globedSelfPlayerProgress = globedProgressBarWrapper->getChildByID("dankmeme.globed2/self-player-progress")) { if (CCLayerColor* target = globedSelfPlayerProgress->getChildByType<CCLayerColor>(0)) { target->setColor(paint()); }}}
        }
    }
};