// ReSharper disable CppHidingFunction
#pragma once

#include "utils.hpp"

#include <Geode/modify/PlayLayer.hpp>
class $modify(canvas, PlayLayer)
{
    enum class RenderKind
    {
        None,
        Solid,
        QuadGradient,
        SegmentedGradient,
    };

    struct Fields
    {
        CCSpriteBatchNode* gradientOverlay = nullptr;
        CCTexture2D* gradientTexture = nullptr;
        int currentSegmentCount = 0;
        RenderKind renderKind = RenderKind::None;
        const ColorConfig* renderedGradientConfig = nullptr;
        int renderedConfigUpdateId = -1;
        float renderedFullWidth = -1.f;
        float renderedVisibleWidth = -1.f;
        float renderedFillHeight = -1.f;
        float renderedColorProgressFrac = -1.f;
        float renderedScrollOffset = -1.f;
    };

    void invalidateGradientRenderState()
    {
        m_fields->renderedGradientConfig = nullptr;
        m_fields->renderedConfigUpdateId = -1;
        m_fields->renderedFullWidth = -1.f;
        m_fields->renderedVisibleWidth = -1.f;
        m_fields->renderedFillHeight = -1.f;
        m_fields->renderedColorProgressFrac = -1.f;
        m_fields->renderedScrollOffset = -1.f;
    }

    void setProgressFillColor(const ccColor3B& color, const bool force = false) const
    {
        if (force || !colorsEqual(m_progressFill->getColor(), color))
            m_progressFill->setColor(color);
    }

    void ensureGradientOverlay()
    {
        const int count = std::max(1, static_cast<int>(fastGetSetting<"gradient-segments", int64_t>()));
        auto* texture = m_progressFill->getTexture();

        if (m_fields->gradientOverlay && m_fields->currentSegmentCount == count && m_fields->gradientTexture == texture)
            return;

        if (m_fields->gradientOverlay)
            m_fields->gradientOverlay->removeFromParent();

        m_fields->gradientOverlay = createProgressFillGradientBatch(m_progressFill, count);
        m_fields->gradientOverlay->setID("gradient-overlay"_spr);

        for (int i = 0; i < count; i++)
        {
            const auto seg = createProgressFillGradientSegment(m_progressFill);
            m_fields->gradientOverlay->addChild(seg);
        }

        m_progressFill->addChild(m_fields->gradientOverlay, 1);
        m_fields->currentSegmentCount = count;
        m_fields->gradientTexture = texture;
        invalidateGradientRenderState();
    }

    void updateGradientSegments(const ColorConfig* cfg)
    {
        const auto fillRect = m_progressFill->getTextureRect();
        const float fillHeight = fillRect.size.height;
        const float fullWidth = m_progressFill->getParent()->getContentSize().width - 4;
        const float visibleWidth = fillRect.size.width;
        const float progressFrac = fullWidth > 0 ? visibleWidth / fullWidth : 1.f;
        const float colorProgressFrac = !cfg->gradientFollowsProgress && progressFrac > 0.01f ? progressFrac : -1.f;
        const float scrollOffset = cfg->gradientScrolling ? colorutil::getRGBStripOffset() : 0.f;
        const auto* delegate = Catgirl::getInstance();

        const bool stateChanged = m_fields->renderKind != RenderKind::SegmentedGradient;
        const bool layoutDirty = stateChanged || m_fields->renderedFullWidth != fullWidth ||
            m_fields->renderedVisibleWidth != visibleWidth || m_fields->renderedFillHeight != fillHeight;
        const bool colorDirty = stateChanged || m_fields->renderedGradientConfig != cfg ||
            m_fields->renderedConfigUpdateId != delegate->updateId ||
            m_fields->renderedColorProgressFrac != colorProgressFrac || m_fields->renderedScrollOffset != scrollOffset;

        if (!layoutDirty && !colorDirty)
            return;

        if (layoutDirty)
            m_fields->gradientOverlay->setContentSize(ccp(fullWidth, fillHeight));

        const auto sortedStops =
            colorDirty ? sortedGradientLocationsFor(*cfg) : std::vector<ColorConfig::GradientLocation>{};

        const int count = m_fields->currentSegmentCount;
        const float segWidth = fullWidth / count;
        int i = 0;
        for (const auto seg : CCArrayExt<CCSprite*>(m_fields->gradientOverlay->getChildren()))
        {
            if (layoutDirty)
            {
                const float x = segWidth * i;
                const float width = calculateProgressFillGradientSegmentWidth(x, segWidth, visibleWidth, fullWidth);
                updateProgressFillGradientSegment(seg, m_progressFill, x, width);
            }

            if (colorDirty)
            {
                float t = (static_cast<float>(i) + 0.5f) / count;
                if (!cfg->gradientFollowsProgress && progressFrac > 0.01f)
                    t = t / progressFrac;

                if (cfg->gradientScrolling)
                {
                    if (!colorsEqual(seg->getColor(),
                                     colorForSortedGradientLooped(*cfg, sortedStops, t + scrollOffset)))
                        seg->setColor(colorForSortedGradientLooped(*cfg, sortedStops, t + scrollOffset));
                }
                else
                {
                    if (!colorsEqual(seg->getColor(), colorForSortedGradient(sortedStops, fminf(t, 1.0f))))
                        seg->setColor(colorForSortedGradient(sortedStops, fminf(t, 1.0f)));
                }
            }
            i++;
        }

        m_fields->renderKind = RenderKind::SegmentedGradient;
        m_fields->renderedGradientConfig = cfg;
        m_fields->renderedConfigUpdateId = delegate->updateId;
        m_fields->renderedFullWidth = fullWidth;
        m_fields->renderedVisibleWidth = visibleWidth;
        m_fields->renderedFillHeight = fillHeight;
        m_fields->renderedColorProgressFrac = colorProgressFrac;
        m_fields->renderedScrollOffset = scrollOffset;
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

                setProgressFillColor(ccWHITE);

                const auto sortedStops = sortedGradientLocationsFor(*cfg);
                const auto [lr, lg, lb] = colorForSortedGradient(sortedStops, 0.f);
                const auto [rr, rg, rb] = colorForSortedGradient(sortedStops, 1.f);

                auto& [tl, bl, tr, br] = m_progressFill->m_sQuad;
                bl.colors = {lr, lg, lb, 0xFF};
                tl.colors = {lr, lg, lb, 0xFF};
                br.colors = {rr, rg, rb, 0xFF};
                tr.colors = {rr, rg, rb, 0xFF};

                m_fields->renderKind = RenderKind::QuadGradient;
                m_fields->renderedGradientConfig = cfg;
                m_fields->renderedConfigUpdateId = delegate->updateId;
            }
            else
            {
                if (m_fields->gradientOverlay)
                    m_fields->gradientOverlay->setVisible(true);

                setProgressFillColor(ccWHITE, m_fields->renderKind == RenderKind::QuadGradient);
                ensureGradientOverlay();
                updateGradientSegments(cfg);
            }
        }
        else
        {
            if (m_fields->gradientOverlay)
                m_fields->gradientOverlay->setVisible(false);
            setProgressFillColor(paint(levelProgress), m_fields->renderKind == RenderKind::QuadGradient);
            m_fields->renderKind = RenderKind::Solid;
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
