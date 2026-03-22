// ReSharper disable CppNonExplicitConversionOperator
// ReSharper disable CppNonExplicitConvertingConstructor
#pragma once

#include "color.hpp"
#include "settings/config.hpp"

template <size_t N>
struct TemplateStr
{
    char data[N];

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
    static Catgirl* meow;
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

Catgirl* Catgirl::meow = nullptr;

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
    return cfg->colorForConfig("", levelProgress);
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
