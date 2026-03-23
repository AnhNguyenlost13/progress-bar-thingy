#pragma once
#include <Geode/Geode.hpp>
#include "dialog.hpp"

// Thanks RaZooM!

static void migrateOldSettings()
{
    if (Mod::get()->getSavedValue<bool>("v2-migrated", false))
        return;

    log::info("Migrating old settings...");

    struct OldCtx
    {
        const char *modeKey, *colorKey, *speedKey, *newKey;
        ccColor3B defaultColor;
    };

    static const OldCtx ctxs[] = {
        {"normal-working-mode", "normal-custom-color", "normal-rgb-speed", "normal-config", ccc3(0xFF, 0xB2, 0xFF)},
        {"practice-working-mode", "practice-custom-color", "practice-rgb-speed", "practice-config",
         ccc3(0x83, 0xCB, 0xFF)},
        {"enby-working-mode", "enby-custom-color", "enby-rgb-speed", "newBest-config", ccc3(0xFF, 0xFF, 0xFF)},
    };

    static const std::unordered_map<std::string, ColorConfigType> modeMap = {
        {"Player Col 1", Player1}, {"Player Col 2", Player2}, {"Player Glow", PlayerGlow},
        {"Chroma", Chroma},        {"Pastel", Pastel},        {"Custom", CustomColor},
    };

    for (const auto& [modeKey, colorKey, speedKey, newKey, defaultColor] : ctxs)
    {
        if (Mod::get()->getSavedValue<matjson::Value>(newKey, {}).isObject())
            continue;

        ColorConfig cfg;
        cfg.type = Player1;
        cfg.customColor = defaultColor;
        cfg.chromaSpeed = 0.5f;

        if (Mod::get()->getSetting(modeKey) && Mod::get()->getSetting(colorKey) && Mod::get()->getSetting(speedKey))
        {
            auto mode = Mod::get()->getSettingValue<std::string>(modeKey);
            if (auto it = modeMap.find(mode); it != modeMap.end())
                cfg.type = it->second;

            cfg.customColor = Mod::get()->getSettingValue<ccColor3B>(colorKey);

            const float old = Mod::get()->getSettingValue<float>(speedKey);
            cfg.chromaSpeed = 0.1f + old * 4.9f;
        }

        Mod::get()->setSavedValue(newKey, cfg.toJson());
    }

    log::info("Settings migration complete!");
    Mod::get()->setSavedValue("v2-migrated", true);
}

template <Context>
class ProgressBarPreviewSetting : public SettingV3
{
public:
    static Result<std::shared_ptr<SettingV3>> parse(std::string const& key, std::string const& modId,
                                                    matjson::Value const& json)
    {
        const auto res = std::make_shared<ProgressBarPreviewSetting>();
        auto root = checkJson(json, "ProgressBarPreviewSetting");
        res->init(key, modId, root);
        res->parseNameAndDescription(root);
        res->parseEnableIf(root);
        root.checkUnknownKeys();
        return root.ok(std::static_pointer_cast<SettingV3>(res));
    }

    bool load(matjson::Value const& json) override { return true; };
    bool save(matjson::Value& json) const override { return true; };
    bool isDefaultValue() const override { return false; };
    void reset() override {};

    SettingNodeV3* createNode(float width) override;
};

template <Context Ctx>
class ProgressBarPreviewNode : public SettingNodeV3
{
protected:
    ProgressBar* mProgressBar = nullptr;
    CCMenuItemSpriteExtra* mConfigBtn = nullptr;
    CCMenu* mMenu = nullptr;
    ColorConfig m_committedConfig;
    ColorConfig m_pendingConfig;
    std::vector<CCLayerColor*> m_barOverlaySegments;
    float m_previewTime = 0.f;

    static const char* configKey()
    {
        switch (Ctx)
        {
        case Context::Normal:
            return "normal-config";
        case Context::Practice:
            return "practice-config";
        case Context::NewBest:
            return "newBest-config";
        default:
            return "normal-config";
        }
    }

    static ColorConfig defaultColorConfig()
    {
        ColorConfig c;
        c.chromaSpeed = 0.5f;
        switch (Ctx)
        {
        case Context::Normal:
            c.type = Player1;
            c.customColor = ccc3(0xFF, 0xB2, 0xFF);
            break;
        case Context::Practice:
            c.type = Player2;
            c.customColor = ccc3(0x83, 0xCB, 0xFF);
            break;
        case Context::NewBest:
            c.type = Pastel;
            c.chromaSpeed = 0.1f;
            c.customColor = ccc3(0xFF, 0xFF, 0xFF);
            break;
        case Context::Unknown:
        default:
            break;
        }

        return c;
    }

    static ColorConfig loadConfig()
    {
        ColorConfig c = defaultColorConfig();
        c.fromJson(Mod::get()->getSavedValue<matjson::Value>(configKey(), matjson::Value{}));
        return c;
    }

    static void saveConfig(ColorConfig& config)
    {
        Mod::get()->setSavedValue(configKey(), config.toJson());
        (void)Mod::get()->saveData();
        Catgirl::getInstance()->updateSettings();
    }

    void updatePreviewFromPending()
    {
        if (m_pendingConfig.type == Gradient && m_pendingConfig.smoothGradient)
        {
            mProgressBar->setFillColor(ccWHITE);
            for (int i = 0; i < static_cast<int>(m_barOverlaySegments.size()); i++)
            {
                const float pos = (static_cast<float>(i) + 0.5f) / m_barOverlaySegments.size();
                if (m_pendingConfig.gradientScrolling)
                    m_barOverlaySegments[i]->setColor(
                        m_pendingConfig.colorForGradient(fmodf(pos + colorutil::getRGBStripOffset(), 1.0f)));
                else
                    m_barOverlaySegments[i]->setColor(m_pendingConfig.colorForGradient(pos));
                m_barOverlaySegments[i]->setVisible(true);
            }
        }
        else
        {
            mProgressBar->setFillColor(m_pendingConfig.colorForConfig());
            for (const auto seg : m_barOverlaySegments)
                seg->setVisible(false);
        }
    }

    static ccColor3B paintForContext()
    {
        auto* meow = Catgirl::getInstance();
        const auto oldCtx = meow->context;
        meow->context = Ctx;
        const auto color = paint();
        meow->context = oldCtx;
        return color;
    }

    bool init(const std::shared_ptr<ProgressBarPreviewSetting<Ctx>>& setting, const float width)
    {
        if (!SettingNodeV3::init(setting, width))
            return false;

        m_committedConfig = loadConfig();
        m_pendingConfig = m_committedConfig;

        mProgressBar = ProgressBar::create(ProgressBarStyle::Level);
        mProgressBar->setID("preview-bar"_spr);
        addChildAtPosition(mProgressBar, Anchor::Center);
        mProgressBar->updateAnchoredPosition(Anchor::Center, ccp(-45, -7));
        mProgressBar->updateProgress(static_cast<float>(rand() % 61 + 20));
        mProgressBar->setFillColor(m_pendingConfig.colorForConfig());

        if (const auto barSpr = mProgressBar->getChildByID("progress-bar"))
        {
            if (const auto fillSpr = barSpr->getChildByID("progress-bar-fill"))
            {
                const float visibleWidth = barSpr->getContentSize().width - 4;
                const int segs = static_cast<int>(Mod::get()->getSettingValue<int64_t>("gradient-segments"));
                const float segWidth = visibleWidth / segs;
                for (int i = 0; i < segs; i++)
                {
                    auto seg = CCLayerColor::create(ccc4(255, 255, 255, 255));
                    seg->ignoreAnchorPointForPosition(false);
                    seg->setAnchorPoint(ccp(0, 0));
                    const float x = segWidth * i;
                    seg->setContentSize(
                        ccp(fminf(ceilf(segWidth) + 1, visibleWidth - x), fillSpr->getContentSize().height));
                    seg->setPosition(ccp(x, 0));
                    seg->setVisible(false);
                    m_barOverlaySegments.push_back(seg);
                    fillSpr->addChild(seg, 1);
                }
            }
        }

        const auto s = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
        s->setScale(0.45f);
        mConfigBtn = CCMenuItemSpriteExtra::create(s, this, menu_selector(ProgressBarPreviewNode::onConfigure));
        mConfigBtn->setID("configure-btn"_spr);
        mConfigBtn->setPosition(width - 230.0f, 16.0f);

        mMenu = CCMenu::create();
        mMenu->setID("preview-menu"_spr);
        mMenu->addChild(mConfigBtn);
        mMenu->setPosition(0.0f, 0.0f);
        addChild(mMenu);

        return true;
    }

    void onEnter() override
    {
        SettingNodeV3::onEnter();
        scheduleUpdate();
    }

    void update(float dt) override
    {
        if (m_pendingConfig.type != Chroma && m_pendingConfig.type != Pastel && m_pendingConfig.type != Gradient)
            return;
        m_previewTime += dt * m_pendingConfig.chromaSpeed * 60.f;
        const float savedVa = colorutil::va;
        colorutil::va = m_previewTime;
        updatePreviewFromPending();
        colorutil::va = savedVa;
    }

    void onConfigure(CCObject*)
    {
        auto* ui = SetupColorConfigUI::create(
            [this](const ColorConfig& conf)
            {
                m_pendingConfig = conf;
                this->markChanged(this);
                updatePreviewFromPending();
            },
            true);

        ui->setStartConfig(m_pendingConfig);
        ui->setDefaultConfig(defaultColorConfig());
        ui->show();
    }

    void onCommit() override
    {
        m_committedConfig = m_pendingConfig;
        saveConfig(m_committedConfig);
    }

    void onResetToDefault() override
    {
        m_pendingConfig = defaultColorConfig();
        this->markChanged(this);
        updatePreviewFromPending();
    }

public:
    static ProgressBarPreviewNode* create(const std::shared_ptr<ProgressBarPreviewSetting<Ctx>>& setting,
                                          const float width)
    {
        auto ret = new ProgressBarPreviewNode();
        if (ret->init(setting, width))
        {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    [[nodiscard]] bool hasUncommittedChanges() const override { return m_pendingConfig != m_committedConfig; }
    [[nodiscard]] bool hasNonDefaultValue() const override { return m_pendingConfig != defaultColorConfig(); }
};

template <Context Ctx>
SettingNodeV3* ProgressBarPreviewSetting<Ctx>::createNode(const float width)
{
    return ProgressBarPreviewNode<Ctx>::create(
        std::static_pointer_cast<ProgressBarPreviewSetting<Ctx>>(shared_from_this()), width);
}

$execute
{
    migrateOldSettings();

    if (auto* mod = Mod::get();
        mod->registerCustomSettingType("normal-preview", &ProgressBarPreviewSetting<Context::Normal>::parse).isOk() &&
        mod->registerCustomSettingType("practice-preview", &ProgressBarPreviewSetting<Context::Practice>::parse)
            .isOk() &&
        mod->registerCustomSettingType("new-best-preview", &ProgressBarPreviewSetting<Context::NewBest>::parse).isOk())
        log::info("Settings registered!");

    Catgirl::getInstance()->updateSettings();
}
