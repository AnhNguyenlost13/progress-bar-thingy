#pragma once
#include <Geode/Geode.hpp>

// Thanks RaZooM!

// what the heck is this?
class NormalProgressBarSetting : public SettingV3
{
public:
    static Result<std::shared_ptr<SettingV3>> parse(std::string const& key, std::string const& modId, matjson::Value const& json)
    {
        const auto res = std::make_shared<NormalProgressBarSetting>();
        auto root = checkJson(json, "NormalProgressBarSetting");
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

class NormalProgressBarSettingNode : public SettingNodeV3
{
protected:
    ProgressBar* mProgressBar = nullptr;
    CCMenuItemSpriteExtra* mButton = nullptr;
    CCMenu* mMenu = nullptr;
    short mLastUpdateId = 0;

    bool init(const std::shared_ptr<NormalProgressBarSetting>& setting, const float width)
    {
        if (!SettingNodeV3::init(setting, width)) return false;
        mProgressBar = ProgressBar::create(ProgressBarStyle::Level);
        addChildAtPosition(mProgressBar, Anchor::Center);
        mProgressBar->updateAnchoredPosition(Anchor::Center, ccp(-45, -7));
        mProgressBar->updateProgress(static_cast<float>(rand() % 61 + 20));
        // apply DA STYLE
        mProgressBar->setFillColor(paint());
        const auto mButtonSprite = ButtonSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
        mButtonSprite->setScale(0.5f);
        mButton = CCMenuItemSpriteExtra::create(mButtonSprite, this, menu_selector(NormalProgressBarSettingNode::onRefreshButton));
        mButton->setPosition(width - 230.0f, 16.0f);
        mButton->setScale(0.8f);
        mMenu = CCMenu::create();
        mMenu->addChild(mButton);
        mMenu->setPosition(0.0f, 0.0f);
        addChild(mMenu);
        return true;
    }

    void onRefreshButton(CCObject* sender)
    {
        if (mLastUpdateId == Catgirl::getInstance()->updateId) return FLAlertLayer::create("Info", "Progress bar is already up to date!", "OK")->show();
        // if (mProgressBar) mProgressBar->removeFromParentAndCleanup(true);
        // mProgressBar = ProgressBar::create();
        /*mMenu->*/addChildAtPosition(mProgressBar, Anchor::Center);
        mProgressBar->updateAnchoredPosition(Anchor::Center, ccp(-45, -7));
        // reapply DA STYLE
        mProgressBar->setFillColor(paint());
        mProgressBar->updateProgress(static_cast<float>(rand() % 61 + 20));
        mLastUpdateId = Catgirl::getInstance()->updateId;
        Notification::create("Progress bar updated!", NotificationIcon::Success)->show();

    }

    void onCommit() override
    {
        log::debug("onCommit called");
    };
    void onResetToDefault() override
    {
        log::debug("onResetToDefault called");
    };

public:
    static NormalProgressBarSettingNode* create(const std::shared_ptr<NormalProgressBarSetting>& setting, const float width)
    {
        auto ret = new NormalProgressBarSettingNode();
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

    [[nodiscard]] std::shared_ptr<NormalProgressBarSetting> getNormalProgressBarSetting() const { return std::static_pointer_cast<NormalProgressBarSetting>(getSetting()); }
};


inline SettingNodeV3* NormalProgressBarSetting::createNode(const float width) { return NormalProgressBarSettingNode::create(std::static_pointer_cast<NormalProgressBarSetting>(shared_from_this()), width); }

$execute {
    if (Mod::get()->registerCustomSettingType("normal-preview", &NormalProgressBarSetting::parse).isOk()) log::info("Settings registered!");
}
