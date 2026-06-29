// ReSharper disable CppNonExplicitConversionOperator
// ReSharper disable CppNonExplicitConvertingConstructor
#pragma once

#include "color.hpp"
#include "settings/config.hpp"

template <size_t N>
struct TemplateStr
{
    char data[N]{};

    constexpr TemplateStr(const char (&s)[N])
    {
        for (size_t i = 0; i < N; ++i)
            data[i] = s[i];
    }
    constexpr operator std::string_view() const noexcept { return std::string_view(data, N - 1); }
};

template <size_t N>
TemplateStr(const char (&)[N]) -> TemplateStr<N>;

template <TemplateStr SettingName, typename T>
T fastGetSetting()
{
    static T s = (geode::listenForSettingChanges<T>(std::string(std::string_view(SettingName)), [](T v) { s = v; }),
                  Mod::get()->getSettingValue<T>(std::string(std::string_view(SettingName))));
    return s;
}

enum class Context
{
    Normal,
    Practice,
    NewBest,
    Unknown
};

class Catgirl
{
public:
    static Catgirl* getInstance() { return meow ? meow : meow = new Catgirl(); }

    void updateSettings()
    {
        normalConfig = loadConfigFor(Context::Normal);
        practiceConfig = loadConfigFor(Context::Practice);
        newBestConfig = loadConfigFor(Context::NewBest);
        updateId++;
    }

    ColorConfig normalConfig;
    ColorConfig practiceConfig;
    ColorConfig newBestConfig;

    Context context = Context::Normal;
    short updateId = 0;

private:
    inline static Catgirl* meow = nullptr;
    Catgirl() { updateSettings(); }

    static ColorConfig loadConfigFor(Context ctx)
    {
        const char* key;
        ccColor3B defaultColor;

        switch (ctx)
        {
        case Context::Normal:
            key = "normal-config";
            defaultColor = ccc3(0xFF, 0xB2, 0xFF);
            break;
        case Context::Practice:
            key = "practice-config";
            defaultColor = ccc3(0x83, 0xCB, 0xFF);
            break;
        case Context::NewBest:
            key = "newBest-config";
            defaultColor = ccc3(0xFF, 0xFF, 0xFF);
            break;
        default:
            return {};
        }

        ColorConfig def;
        def.type = (ctx == Context::NewBest) ? Pastel : (ctx == Context::Practice) ? Player2 : Player1;
        def.customColor = defaultColor;
        def.chromaSpeed = (ctx == Context::NewBest) ? 0.1f : 0.5f;

        def.fromJson(Mod::get()->getSavedValue<matjson::Value>(key, matjson::Value{}));
        return def;
    }

public:
    Catgirl(const Catgirl&) = delete;
    Catgirl& operator=(const Catgirl&) = delete;
};

/**
 * @brief Gets the level state-dependent color.
 */
inline ccColor3B paint(float levelProgress = -1.f)
{
    auto* d = Catgirl::getInstance();
    ColorConfig* cfg;
    switch (d->context)
    {
    case Context::Practice:
        cfg = &d->practiceConfig;
        break;
    case Context::NewBest:
        cfg = &d->newBestConfig;
        break;
    default:
        cfg = &d->normalConfig;
        break;
    }
    return cfg->colorForConfig(levelProgress);
}

/**
 * @brief Gets the chroma animation speed for the current context.
 */
inline float getSpeed()
{
    switch (const auto* d = Catgirl::getInstance(); d->context)
    {
    case Context::Practice:
        return d->practiceConfig.chromaSpeed;
    case Context::NewBest:
        return d->newBestConfig.chromaSpeed;
    default:
        return d->normalConfig.chromaSpeed;
    }
}

//
// Some other fill helpers
//

inline CCSprite* createProgressFillGradientSegment(CCSprite* fillSpr)
{
    auto* seg = CCSprite::createWithTexture(fillSpr->getTexture());
    seg->ignoreAnchorPointForPosition(false);
    seg->setAnchorPoint(ccp(0, 0));
    return seg;
}

inline CCSpriteBatchNode* createProgressFillGradientBatch(CCSprite* fillSpr, const int capacity)
{
    auto* batch = CCSpriteBatchNode::createWithTexture(fillSpr->getTexture(), std::max(1, capacity));
    batch->setBlendFunc(fillSpr->getBlendFunc());
    batch->setAnchorPoint(ccp(0, 0));
    batch->setPosition(ccp(0, 0));
    return batch;
}

inline std::vector<ColorConfig::GradientLocation> sortedGradientLocationsFor(const ColorConfig& cfg)
{
    auto sorted = cfg.gradientLocations;
    std::ranges::sort(sorted, [](const ColorConfig::GradientLocation& a, const ColorConfig::GradientLocation& b)
                      { return a.percentageLocation < b.percentageLocation; });
    return sorted;
}

inline ccColor3B colorForSortedGradient(const std::vector<ColorConfig::GradientLocation>& sorted, const float v)
{
    if (sorted.empty())
        return ccWHITE;

    if (v <= sorted.front().percentageLocation)
        return sorted.front().color;
    if (v >= sorted.back().percentageLocation)
        return sorted.back().color;

    for (size_t i = 1; i < sorted.size(); i++)
    {
        if (v <= sorted[i].percentageLocation)
        {
            const float t = (v - sorted[i - 1].percentageLocation) /
                (sorted[i].percentageLocation - sorted[i - 1].percentageLocation);
            const auto& [r1, g1, b1] = sorted[i - 1].color;
            const auto& [r2, g2, b2] = sorted[i].color;
            return ccc3(static_cast<uint8_t>(r1 + t * (r2 - r1)), static_cast<uint8_t>(g1 + t * (g2 - g1)),
                        static_cast<uint8_t>(b1 + t * (b2 - b1)));
        }
    }

    return sorted.back().color;
}

inline ccColor3B colorForSortedGradientLooped(const ColorConfig& cfg,
                                              const std::vector<ColorConfig::GradientLocation>& sorted, float v)
{
    v = fmodf(v, 1.0f);
    if (v < 0)
        v += 1.0f;

    if (cfg.gradientMirrorLoop)
    {
        v = v * 2.0f;
        if (v > 1.0f)
            v = 2.0f - v;
    }

    return colorForSortedGradient(sorted, v);
}

inline float calculateProgressFillGradientSegmentWidth(const float x, const float segmentWidth,
                                                       const float visibleWidth, const float fullWidth)
{
    const float overlapWidth = fminf(ceilf(segmentWidth) + 1.f, fullWidth - x);
    return fmaxf(0.f, fminf(overlapWidth, visibleWidth - x));
}

inline void updateProgressFillGradientSegment(CCSprite* seg, CCSprite* fillSpr, const float x, const float width)
{
    if (width <= 0.f)
        return seg->setVisible(false);

    const auto texRect = fillSpr->getTextureRect();
    if (!seg->getBatchNode() && seg->getTexture() != fillSpr->getTexture())
        seg->setTexture(fillSpr->getTexture());
    seg->setTextureRect(CCRectMake(texRect.origin.x + x, texRect.origin.y, width, texRect.size.height));
    seg->setPosition(ccp(x, 0));
    seg->setVisible(true);
}

// there's no operator== so
inline bool colorsEqual(const ccColor3B& a, const ccColor3B& b) { return a.r == b.r && a.g == b.g && a.b == b.b; }