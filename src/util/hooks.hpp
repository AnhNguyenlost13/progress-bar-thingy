// ReSharper disable CppHidingFunction
#pragma once

#include "utils.hpp" // Thanks TheSillyDoggo
#define updateProgressBar updateProgressbar // I keep messing up the casing so this will do.

#include <Geode/modify/PlayLayer.hpp>
class $modify(canvas, PlayLayer) {
    // Custom function
    void repaint() const {
        m_progressFill->setColor(paint());
        // beautiful. kill it with fire.
        if (CCNode* globedProgressBarWrapper = m_progressBar->getChildByID("dankmeme.globed2/progress-bar-wrapper")) { if (CCNode* globedSelfPlayerProgress = globedProgressBarWrapper->getChildByID("dankmeme.globed2/self-player-progress")) { if (auto target = globedSelfPlayerProgress->getChildByType<CCLayerColor>(0)) { target->setColor(paint()); }}}
    }

    $override void startGame() {
        Catgirl::getInstance()->context = Context::Normal;
        PlayLayer::startGame();
    }

    $override void resetLevel() {
        Catgirl::getInstance()->updateSettings();
        PlayLayer::resetLevel();
    }

    $override void updateProgressBar() {
        PlayLayer::updateProgressBar();
        const auto levelPercentage = as<float>(PlayLayer::get()->getCurrentPercentInt());
	    auto levelBest = PlayLayer::get()->m_level->m_normalPercent;
        const auto delegate = Catgirl::getInstance();
        // set context
        if (m_isPracticeMode) {
            if (levelPercentage > levelBest) {
                if (fastGetSetting<"practice-override", bool>()) { delegate->context = Context::Practice; } else { delegate->context = Context::NewBest; }
            } else {
                delegate->context = Context::Practice;
            }
        } else {
            if (levelPercentage > levelBest) { delegate->context = Context::NewBest; } else { delegate->context = Context::Normal; }
        }

        colorutil::update(getSpeed());
        if (!m_level->isPlatformer() && m_progressFill && m_progressBar->isVisible()) { canvas::repaint(); }
    }
};