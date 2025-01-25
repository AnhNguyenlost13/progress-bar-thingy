#pragma once

#include "utils.hpp" // Thanks TheSillyDoggo
#define updateProgressBar updateProgressbar // I blame RobTop.

#include <Geode/modify/PlayLayer.hpp>
class $modify(canvas, PlayLayer) {
    void repaint() {
        m_progressFill->setColor(paint());
        // beautiful. kill it with fire.
        if (auto globedProgressBarWrapper = m_progressBar->getChildByID("dankmeme.globed2/progress-bar-wrapper")) { if (auto globedSelfPlayerProgress = globedProgressBarWrapper->getChildByID("dankmeme.globed2/self-player-progress")) { if (CCLayerColor* target = globedSelfPlayerProgress->getChildByType<CCLayerColor>(0)) { target->setColor(paint()); }}}
    }

    void resetLevel() {
        Catgirl::getInstance()->context = Context::Normal;
        Catgirl::getInstance()->updateSettings();
        PlayLayer::resetLevel();
    }

    // TODO: find out a way to disable this hook when not dynamic
    void updateProgressBar() {
        PlayLayer::updateProgressBar();
        auto levelPercentage = as<float>(PlayLayer::get()->getCurrentPercentInt());
	    auto levelBest = PlayLayer::get()->m_level->m_normalPercent;

        // set context
        if (m_isPracticeMode) {
            if (levelPercentage > levelBest) {
                if (fastGetSetting<"practice-override", bool>()) { Catgirl::getInstance()->context = Context::Practice; } else { Catgirl::getInstance()->context = Context::NewBest; }
            } else {
                Catgirl::getInstance()->context = Context::Practice;
            }
        } else {
            if (levelPercentage > levelBest) { Catgirl::getInstance()->context = Context::NewBest; }
        }

        // update rgb speed
        colorutil::update(getSpeed());
        if (!m_level->isPlatformer() && m_progressFill && m_progressBar->isVisible()) { canvas::repaint(); }

    }
};