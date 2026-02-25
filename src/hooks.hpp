// ReSharper disable CppHidingFunction
#pragma once
#define as static_cast

#include "utils.hpp" // Thanks TheSillyDoggo

#include <Geode/modify/PlayLayer.hpp>
class $modify(canvas, PlayLayer) {
    void repaint() const {
        m_progressFill->setColor(paint());
        if (CCNode* globedProgressBarWrapper = m_progressBar->getChildByID("dankmeme.globed2/progress-bar-wrapper")) if (CCNode* globedSelfPlayerProgress = globedProgressBarWrapper->getChildByID("dankmeme.globed2/self-player-progress")) if (const auto target = globedSelfPlayerProgress->getChildByType<CCLayerColor>(0)) target->setColor(paint());
    }

    $override void startGame() {
        Catgirl::getInstance()->context = Context::Normal;
        PlayLayer::startGame();
    }

    $override void resetLevel() {
        Catgirl::getInstance()->updateSettings();
        PlayLayer::resetLevel();
    }

    $override void updateProgressbar() {
        PlayLayer::updateProgressbar();
        const auto levelPercentage = as<float>(get()->getCurrentPercentInt());
	    auto levelBest = get()->m_level->m_normalPercent;
        const auto delegate = Catgirl::getInstance();

        if (m_isPracticeMode) if (levelPercentage > levelBest) if (fastGetSetting<"practice-override", bool>()) delegate->context = Context::Practice; else delegate->context = Context::NewBest; else delegate->context = Context::Practice; else if (levelPercentage > levelBest) delegate->context = Context::NewBest; else delegate->context = Context::Normal;

        colorutil::update(getSpeed());
        if (!m_level->isPlatformer() && m_progressFill && m_progressBar->isVisible()) repaint();
    }
};
