#include "config.hpp"
#include "../color.hpp"

using namespace cocos2d;

ccColor3B ColorConfig::colorForConfig(std::string channel, float levelProgress)
{
    auto gm = GameManager::sharedState();
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
                v = levelProgress / 100.f; // levelProgress is 0-100
            else
                v = (sinf(colorutil::va * chromaSpeed) + 1.0f) / 2.0f;
            return colorForGradient(v);
        }
    case CustomColor:
    default:
        return customColor;
    }
}

ccColor3B ColorConfig::colorForGradient(float v)
{
    if (gradientLocations.empty())
        return ccc3(255, 255, 255);

    auto sorted = gradientLocations;
    std::sort(sorted.begin(), sorted.end(), [](const GradientLocation& a, const GradientLocation& b)
              { return a.percentageLocation < b.percentageLocation; });

    if (v <= sorted.front().percentageLocation)
        return sorted.front().color;
    if (v >= sorted.back().percentageLocation)
        return sorted.back().color;

    for (size_t i = 1; i < sorted.size(); i++)
    {
        if (v <= sorted[i].percentageLocation)
        {
            float t = (v - sorted[i - 1].percentageLocation) /
                (sorted[i].percentageLocation - sorted[i - 1].percentageLocation);
            const auto& c1 = sorted[i - 1].color;
            const auto& c2 = sorted[i].color;
            return ccc3(static_cast<uint8_t>(c1.r + t * (c2.r - c1.r)), static_cast<uint8_t>(c1.g + t * (c2.g - c1.g)),
                        static_cast<uint8_t>(c1.b + t * (c2.b - c1.b)));
        }
    }

    return sorted.back().color;
}

matjson::Value ColorConfig::toJson()
{
    auto stops = matjson::Value::array();
    for (const auto& loc : gradientLocations)
    {
        stops.push(matjson::makeObject({
            {"r", (int)loc.color.r},
            {"g", (int)loc.color.g},
            {"b", (int)loc.color.b},
            {"pos", loc.percentageLocation},
        }));
    }

    return matjson::makeObject({
        {"r", (int)customColor.r},
        {"g", (int)customColor.g},
        {"b", (int)customColor.b},
        {"opacity", opacity},
        {"chromaSpeed", chromaSpeed},
        {"type", (int)type},
        {"smoothGradient", smoothGradient},
        {"gradientFollowsProgress", gradientFollowsProgress},
        {"gradient", stops},
    });
}

void ColorConfig::fromJson(matjson::Value value)
{
    if (!value.isObject())
        return;
    if (value.contains("r"))
        customColor.r = (uint8_t)value["r"].asInt().unwrapOr(255);
    if (value.contains("g"))
        customColor.g = (uint8_t)value["g"].asInt().unwrapOr(255);
    if (value.contains("b"))
        customColor.b = (uint8_t)value["b"].asInt().unwrapOr(255);
    if (value.contains("opacity"))
        opacity = (float)value["opacity"].asDouble().unwrapOr(1.0);
    if (value.contains("chromaSpeed"))
        chromaSpeed = (float)value["chromaSpeed"].asDouble().unwrapOr(1.0);
    if (value.contains("type"))
        type = (ColorConfigType)value["type"].asInt().unwrapOr(0);
    if (value.contains("smoothGradient"))
        smoothGradient = value["smoothGradient"].asBool().unwrapOr(true);
    if (value.contains("gradientFollowsProgress"))
        gradientFollowsProgress = value["gradientFollowsProgress"].asBool().unwrapOr(false);

    if (value.contains("gradient") && value["gradient"].isArray())
    {
        gradientLocations.clear();
        for (auto& stop : value["gradient"])
        {
            GradientLocation loc;
            loc.color.r = (uint8_t)stop["r"].asInt().unwrapOr(255);
            loc.color.g = (uint8_t)stop["g"].asInt().unwrapOr(255);
            loc.color.b = (uint8_t)stop["b"].asInt().unwrapOr(255);
            loc.percentageLocation = (float)stop["pos"].asDouble().unwrapOr(0.0);
            gradientLocations.push_back(loc);
        }
    }
}
