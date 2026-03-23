#include "config.hpp"
#include "../color.hpp"

using namespace cocos2d;

ccColor3B ColorConfig::colorForConfig(const float levelProgress) const
{
    const auto gm = GameManager::sharedState();
    switch (type)
    {
    case Player1:
        return gm->colorForIdx(gm->getPlayerColor());
    case Player2:
        return gm->colorForIdx(gm->getPlayerColor2());
    case PlayerGlow:
        return gm->colorForIdx(gm->getPlayerGlowColor());
    case Chroma:
        return colorutil::getChromaColor();
    case Pastel:
        return colorutil::getPastelColor();
    case Gradient:
        {
            float v;
            if (gradientFollowsProgress && levelProgress >= 0.f)
                v = levelProgress / 100.f; // it's the %
            else
                v = (sinf(colorutil::va * chromaSpeed * 0.1f) + 1.0f) / 2.0f;
            return colorForGradient(v);
        }
    case CustomColor:
    default:
        return customColor;
    }
}

ccColor3B ColorConfig::colorForGradient(const float v) const
{
    if (gradientLocations.empty())
        return ccWHITE;

    auto sorted = gradientLocations;
    std::ranges::sort(sorted, [](const GradientLocation& a, const GradientLocation& b)
                      { return a.percentageLocation < b.percentageLocation; });

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

ccColor3B ColorConfig::colorForGradientLooped(float v) const
{
    v = fmodf(v, 1.0f);
    if (v < 0)
        v += 1.0f;

    if (gradientMirrorLoop)
    {
        v = v * 2.0f;
        if (v > 1.0f)
            v = 2.0f - v;
    }

    return colorForGradient(v);
}

matjson::Value ColorConfig::toJson()
{
    auto stops = matjson::Value::array();
    for (const auto& [color, percentageLocation] : gradientLocations)
    {
        stops.push(matjson::makeObject({
            {"r", static_cast<int>(color.r)},
            {"g", static_cast<int>(color.g)},
            {"b", static_cast<int>(color.b)},
            {"pos", percentageLocation},
        }));
    }

    return matjson::makeObject({
        {"r", static_cast<int>(customColor.r)},
        {"g", static_cast<int>(customColor.g)},
        {"b", static_cast<int>(customColor.b)},
        {"opacity", opacity},
        {"chromaSpeed", chromaSpeed},
        {"type", static_cast<int>(type)},
        {"smoothGradient", smoothGradient},
        {"gradientFollowsProgress", gradientFollowsProgress},
        {"gradientScrolling", gradientScrolling},
        {"gradientMirrorLoop", gradientMirrorLoop},
        {"gradient", stops},
    });
}

void ColorConfig::fromJson(matjson::Value value)
{
    if (!value.isObject())
        return;
    if (value.contains("r"))
        customColor.r = static_cast<uint8_t>(value["r"].asInt().unwrapOr(255));
    if (value.contains("g"))
        customColor.g = static_cast<uint8_t>(value["g"].asInt().unwrapOr(255));
    if (value.contains("b"))
        customColor.b = static_cast<uint8_t>(value["b"].asInt().unwrapOr(255));
    if (value.contains("opacity"))
        opacity = static_cast<float>(value["opacity"].asDouble().unwrapOr(1.0));
    if (value.contains("chromaSpeed"))
        chromaSpeed = static_cast<float>(value["chromaSpeed"].asDouble().unwrapOr(1.0));
    if (value.contains("type"))
        type = static_cast<ColorConfigType>(value["type"].asInt().unwrapOr(0));
    if (value.contains("smoothGradient"))
        smoothGradient = value["smoothGradient"].asBool().unwrapOr(true);
    if (value.contains("gradientFollowsProgress"))
        gradientFollowsProgress = value["gradientFollowsProgress"].asBool().unwrapOr(false);
    if (value.contains("gradientScrolling"))
        gradientScrolling = value["gradientScrolling"].asBool().unwrapOr(false);
    if (value.contains("gradientMirrorLoop"))
        gradientMirrorLoop = value["gradientMirrorLoop"].asBool().unwrapOr(false);

    if (value.contains("gradient") && value["gradient"].isArray())
    {
        gradientLocations.clear();
        for (auto& stop : value["gradient"])
        {
            GradientLocation loc;
            loc.color.r = static_cast<uint8_t>(stop["r"].asInt().unwrapOr(255));
            loc.color.g = static_cast<uint8_t>(stop["g"].asInt().unwrapOr(255));
            loc.color.b = static_cast<uint8_t>(stop["b"].asInt().unwrapOr(255));
            loc.percentageLocation = static_cast<float>(stop["pos"].asDouble().unwrapOr(0.0));
            gradientLocations.push_back(loc);
        }
    }
}
