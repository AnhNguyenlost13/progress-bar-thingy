#pragma once

#include <Geode/Geode.hpp>

enum ColorConfigType
{
    CustomColor = 0,
    Player1 = 1,
    Player2 = 2,
    PlayerGlow = 3,
    Chroma = 4,
    Pastel = 5,
    Gradient = 6,
};

struct ColorConfig
{
    struct GradientLocation
    {
        cocos2d::ccColor3B color = cocos2d::ccc3(255, 255, 255);
        float percentageLocation = 0.0f;

        bool operator==(const GradientLocation& o) const
        {
            return color.r == o.color.r && color.g == o.color.g && color.b == o.color.b &&
                percentageLocation == o.percentageLocation;
        }
    };

    cocos2d::ccColor3B customColor = cocos2d::ccc3(255, 255, 255);
    float opacity = 1.0f;
    float chromaSpeed = 1.0f;
    ColorConfigType type = CustomColor;
    std::vector<GradientLocation> gradientLocations = {{cocos2d::ccc3(255, 0, 0), 0}, {cocos2d::ccc3(0, 255, 0), 1}};
    bool smoothGradient = true;
    bool gradientFollowsProgress = false;
    bool gradientScrolling = false;

    cocos2d::ccColor3B colorForConfig(float levelProgress = -1.f) const;
    cocos2d::ccColor3B colorForGradient(float v) const;

    bool operator==(const ColorConfig& o) const
    {
        return customColor.r == o.customColor.r && customColor.g == o.customColor.g &&
            customColor.b == o.customColor.b && opacity == o.opacity && chromaSpeed == o.chromaSpeed &&
            type == o.type && gradientLocations == o.gradientLocations && smoothGradient == o.smoothGradient &&
            gradientFollowsProgress == o.gradientFollowsProgress && gradientScrolling == o.gradientScrolling;
    }
    bool operator!=(const ColorConfig& o) const { return !(*this == o); }

    matjson::Value toJson();
    void fromJson(matjson::Value value);
};
