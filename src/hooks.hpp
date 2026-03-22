// ReSharper disable CppHidingFunction
#pragma once

#include "utils.hpp" // Thanks TheSillyDoggo

#include <Geode/modify/PlayLayer.hpp>
class $modify(canvas, PlayLayer)
{
    struct Fields
    {
        CCNode* gradientOverlay = nullptr;
        int currentSegmentCount = 0;
    };

    void ensureGradientOverlay()
    {
        const int count = fastGetSetting<"gradient-segments", int64_t>();

        if (m_fields->gradientOverlay && m_fields->currentSegmentCount == count)
            return;

        if (m_fields->gradientOverlay)
            m_fields->gradientOverlay->removeFromParent();

        m_fields->gradientOverlay = CCNode::create();
        m_fields->gradientOverlay->setID("gradient-overlay"_spr);
        m_fields->gradientOverlay->setAnchorPoint(ccp(0, 0));
        m_fields->gradientOverlay->setPosition(ccp(0, 0));

        for (int i = 0; i < count; i++)
        {
            const auto seg = CCLayerColor::create(ccc4(255, 255, 255, 255));
            seg->ignoreAnchorPointForPosition(false);
            seg->setAnchorPoint(ccp(0, 0));
            m_fields->gradientOverlay->addChild(seg);
        }

        m_progressFill->addChild(m_fields->gradientOverlay, 1);
        m_fields->currentSegmentCount = count;
    }

    void updateGradientSegments(ColorConfig* cfg)
    {
        const auto fillSize = m_progressFill->getContentSize();
        m_fields->gradientOverlay->setContentSize(fillSize);

        const int count = m_fields->currentSegmentCount;
        const float segWidth = fillSize.width / count;
        int i = 0;
        for (const auto seg : CCArrayExt<CCLayerColor*>(m_fields->gradientOverlay->getChildren()))
        {
            const float t = (static_cast<float>(i) + 0.5f) / count;
            seg->setContentSize(ccp(ceilf(segWidth) + 1, fillSize.height));
            seg->setPosition(ccp(segWidth * i, 0));
            seg->setColor(cfg->colorForGradient(t));
            i++;
        }
    }

    void repaint(const float levelProgress)
    {
        auto* delegate = Catgirl::getInstance();
        ColorConfig* cfg;
        switch (delegate->context)
        {
        case Context::Practice:
            cfg = &delegate->practiceConfig;
            break;
        case Context::NewBest:
            cfg = &delegate->newBestConfig;
            break;
        default:
            cfg = &delegate->normalConfig;
            break;
        }

        if (cfg->type == Gradient && cfg->smoothGradient)
        {
            if (cfg->gradientLocations.size() <= 2)
            {
                if (m_fields->gradientOverlay)
                    m_fields->gradientOverlay->setVisible(false);

                m_progressFill->setColor(ccWHITE);

                const auto [lr, lg, lb] = cfg->colorForGradient(0.f);
                const auto [rr, rg, rb] = cfg->colorForGradient(1.f);

                auto& [tl, bl, tr, br] = m_progressFill->m_sQuad;
                bl.colors = {lr, lg, lb, 0xFF};
                tl.colors = {lr, lg, lb, 0xFF};
                br.colors = {rr, rg, rb, 0xFF};
                tr.colors = {rr, rg, rb, 0xFF};
            }
            else
            {
                if (m_fields->gradientOverlay)
                    m_fields->gradientOverlay->setVisible(true);

                m_progressFill->setColor(ccWHITE);
                ensureGradientOverlay();
                updateGradientSegments(cfg);
            }
        }
        else
        {
            if (m_fields->gradientOverlay)
                m_fields->gradientOverlay->setVisible(false);
            m_progressFill->setColor(paint(levelProgress));
        }

        // Globed compatibility
        if (auto* w = m_progressBar->getChildByID("dankmeme.globed2/progress-bar-wrapper"))
            if (auto* s = w->getChildByID("dankmeme.globed2/self-player-progress"))
                if (const auto t = s->getChildByType<CCLayerColor>(0))
                    t->setColor(paint(levelProgress));
    }

    $override void startGame()
    {
        Catgirl::getInstance()->context = Context::Normal;
        PlayLayer::startGame();
    }

    $override void resetLevel() override
    {
        Catgirl::getInstance()->updateSettings();
        PlayLayer::resetLevel();
    }

    $override void updateProgressbar()
    {
        PlayLayer::updateProgressbar();
        const auto levelPercentage = static_cast<float>(get()->getCurrentPercentInt());
        auto levelBest = get()->m_level->m_normalPercent;
        const auto delegate = Catgirl::getInstance();

        if (m_isPracticeMode)
            if (levelPercentage > levelBest)
                if (fastGetSetting<"practice-override", bool>())
                    delegate->context = Context::Practice;
                else
                    delegate->context = Context::NewBest;
            else
                delegate->context = Context::Practice;
        else if (levelPercentage > levelBest)
            delegate->context = Context::NewBest;
        else
            delegate->context = Context::Normal;

        colorutil::update(getSpeed());
        if (!m_level->isPlatformer() && m_progressFill && m_progressBar->isVisible())
            repaint(levelPercentage);
    }
};
