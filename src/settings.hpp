#pragma once
#include <Geode/Geode.hpp>

// Thanks RaZooM!

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
    bool isDefaultValue() const override { return true; };
    void reset() override {};

    SettingNodeV3* createNode(float width) override;
};

template <Context Ctx>
class ProgressBarPreviewNode : public SettingNodeV3
{
protected:
    ProgressBar* mProgressBar = nullptr;
    CCMenuItemSpriteExtra* mButton = nullptr;
    CCMenu* mMenu = nullptr;
    short mLastUpdateId = 0;

    ccColor3B paintForContext()
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
        if (!SettingNodeV3::init(setting, width)) return false;
        mProgressBar = ProgressBar::create(ProgressBarStyle::Level);
        addChildAtPosition(mProgressBar, Anchor::Center);
        mProgressBar->updateAnchoredPosition(Anchor::Center, ccp(-45, -7));
        mProgressBar->updateProgress(static_cast<float>(rand() % 61 + 20));
        // apply DA STYLE
        mProgressBar->setFillColor(paintForContext());
        const auto mButtonSprite = ButtonSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
        mButtonSprite->setScale(0.5f);
        mButton = CCMenuItemSpriteExtra::create(mButtonSprite, this, menu_selector(ProgressBarPreviewNode::onRefreshButton));
        mButton->setPosition(width - 230.0f, 16.0f);
        mButton->setScale(0.8f);
        mMenu = CCMenu::create();
        mMenu->addChild(mButton);
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
        const auto* meow = Catgirl::getInstance();
        WorkingMode mode;
        switch (Ctx)
        {
        case Context::Normal:
            mode = meow->normalWorkingMode;
            break;
        case Context::Practice:
            mode = meow->practiceWorkingMode;
            break;
        case Context::NewBest:
            mode = meow->newBestWorkingMode;
            break;
        default:
            mode = meow->normalWorkingMode;
            break;
        }

        if (mode != WorkingMode::Chroma && mode != WorkingMode::Pastel) return;

        const auto oldCtx = meow->context;
        const_cast<Catgirl*>(meow)->context = Ctx;
        colorutil::update(getSpeed());
        const_cast<Catgirl*>(meow)->context = oldCtx;

        mProgressBar->setFillColor(paintForContext());
    }

    void onRefreshButton(CCObject*)
    {
        if (mLastUpdateId == Catgirl::getInstance()->updateId)
            return FLAlertLayer::create("Info", "Progress bar is already up to date!", "OK")->show();
        addChildAtPosition(mProgressBar, Anchor::Center);
        mProgressBar->updateAnchoredPosition(Anchor::Center, ccp(-45, -7));
        // reapply DA STYLE
        mProgressBar->setFillColor(paintForContext());
        mProgressBar->updateProgress(static_cast<float>(rand() % 61 + 20));
        mLastUpdateId = Catgirl::getInstance()->updateId;
        Notification::create("Progress bar updated!", NotificationIcon::Success)->show();
    }

    void onCommit() override { log::debug("onCommit called"); };
    void onResetToDefault() override { log::debug("onResetToDefault called"); };

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

    [[nodiscard]] bool hasUncommittedChanges() const override { return false; }
    [[nodiscard]] bool hasNonDefaultValue() const override { return false; }
};

template <Context Ctx>
SettingNodeV3* ProgressBarPreviewSetting<Ctx>::createNode(const float width)
{
    return ProgressBarPreviewNode<Ctx>::create(
        std::static_pointer_cast<ProgressBarPreviewSetting<Ctx>>(shared_from_this()), width);
}

$execute
{
    if (auto* mod = Mod::get();
        mod->registerCustomSettingType("normal-preview", &ProgressBarPreviewSetting<Context::Normal>::parse).isOk() &&
        mod->registerCustomSettingType("practice-preview", &ProgressBarPreviewSetting<Context::Practice>::parse).isOk() &&
        mod->registerCustomSettingType("new-best-preview", &ProgressBarPreviewSetting<Context::NewBest>::parse).isOk())
        log::info("Settings registered!");
    Catgirl::getInstance()->updateSettings();
    listenForAllSettingChanges([](std::string_view, std::shared_ptr<SettingV3>){ Catgirl::getInstance()->updateSettings(); });
}
