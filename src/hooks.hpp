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

    void updateGradientSegments(const ColorConfig* cfg)
    {
        const float fillHeight = m_progressFill->getContentSize().height;
        const float fullWidth = m_progressFill->getParent()->getContentSize().width - 4;
        const float visibleWidth = m_progressFill->getTextureRect().size.width;
        const float progressFrac = fullWidth > 0 ? visibleWidth / fullWidth : 1.f;

        m_fields->gradientOverlay->setContentSize(ccp(fullWidth, fillHeight));

        const int count = m_fields->currentSegmentCount;
        const float segWidth = fullWidth / count;
        int i = 0;
        for (const auto seg : CCArrayExt<CCLayerColor*>(m_fields->gradientOverlay->getChildren()))
        {
            const float x = segWidth * i;
            seg->setContentSize(ccp(fminf(ceilf(segWidth) + 1, fullWidth - x), fillHeight));
            seg->setPosition(ccp(x, 0));
            seg->setVisible(x < visibleWidth);

            float t = (static_cast<float>(i) + 0.5f) / count;
            if (!cfg->gradientFollowsProgress && progressFrac > 0.01f)
                t = t / progressFrac;
            if (cfg->gradientScrolling)
                seg->setColor(cfg->colorForGradientLooped(t + colorutil::getRGBStripOffset()));
            else
                seg->setColor(cfg->colorForGradient(fminf(t, 1.0f)));
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
            if (cfg->gradientLocations.size() <= 2 && !cfg->gradientScrolling && !cfg->gradientFollowsProgress)
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

        colorutil::update(getSpeed() * CCDirector::get()->getDeltaTime() * 60.f);
        if (!m_level->isPlatformer() && m_progressFill && m_progressBar->isVisible())
            repaint(levelPercentage);
    }
};
