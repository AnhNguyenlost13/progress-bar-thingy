#pragma once

#include <Geode/Geode.hpp>
#include "config.hpp"

using namespace geode::prelude;

// thanks to TheSillyDoggo

class PopupBase : public CCLayerColor
{
protected:
    CCSize m_size;
    bool m_noElasticity = false;
    CCScale9Sprite* m_bgSprite = nullptr;
    CCMenu* m_buttonMenu = nullptr;

    void keyDown(const enumKeyCodes code, const double timestamp) override { CCLayerColor::keyDown(code, timestamp); };

    void keyBackClicked() override { onClose(nullptr); };

    void registerWithTouchDispatcher() override { CCTouchDispatcher::get()->addTargetedDelegate(this, -500, true); };

    ~PopupBase() override { CCTouchDispatcher::get()->unregisterForcePrio(this); };

    static float calcPopupScale() { return 1.f; };

public:
    bool addToScene = true;
    bool shouldVisit = true;
    CCLayer* m_mainLayer = nullptr;

    virtual bool init(const float width, const float height)
    {
        if (!CCLayerColor::init())
            return false;

        const float uiScale = calcPopupScale();
        this->setScale(uiScale);
        this->setContentSize(getContentSize() / uiScale);
        this->setPosition((CCDirector::get()->getWinSize() - getContentSize()) / 2);

        CCTouchDispatcher::get()->registerForcePrio(this, 2);
        this->setKeypadEnabled(true);
        this->setTouchEnabled(true);
        this->setKeyboardEnabled(true);

        m_size = ccp(width, height);
        m_bgSprite = CCScale9Sprite::create("GJ_square01.png");
        m_bgSprite->setID("popup-bg"_spr);
        m_bgSprite->setVisible(false);
        m_buttonMenu = CCMenu::create();
        m_buttonMenu->setID("popup-button-menu"_spr);

        m_mainLayer = CCLayer::create();
        m_mainLayer->setID("popup-main-layer"_spr);
        m_mainLayer->ignoreAnchorPointForPosition(false);
        m_mainLayer->setContentSize(m_size);
        m_mainLayer->setAnchorPoint(ccp(0.5f, 0.5f));
        m_mainLayer->setPosition(getContentSize() / 2);

        setup();
        this->addChild(m_mainLayer);
        this->addChild(m_buttonMenu);
        return true;
    };

    virtual bool initAnchored(const float width, const float height) { return init(width, height); };
    virtual bool setup() { return true; };

    virtual void show()
    {
        const auto scene = CCScene::get();
        if (addToScene)
            scene->addChild(this, scene->getHighestChildZ() + 1);

        if (!m_noElasticity)
        {
            m_mainLayer->setScale(0);
            m_mainLayer->runAction(CCEaseElasticOut::create(CCScaleTo::create(0.5f, 1.0f), 0.6f));
            this->runAction(CCFadeTo::create(0.14f, 150));
        }
    };

    void visit() override
    {
        if (shouldVisit)
            CCLayer::visit();
    };

    virtual void onClose(CCObject* sender)
    {
        const auto scene = CCScene::get();
        if (addToScene)
            scene->removeChild(this);
    };
};

class SetupColorConfigUI : public PopupBase, public ColorPickerDelegate, public TextInputDelegate
{
    bool allowEffects = false;
    ColorConfig defaultConfig;
    ColorConfig startConfig;
    ColorConfig currentConfig;
    std::function<void(ColorConfig)> onFinishFunc = nullptr;
    CCLayerColor* startColor = nullptr;
    CCLayerColor* endColor = nullptr;
    CCControlColourPicker* picker = nullptr;
    CCMenu* typeMenu = nullptr;
    CCMenu* bottomLeft = nullptr;
    std::map<CCMenuItemToggler*, ColorConfigType> configTypes = {};
    CCNode* gradientPreviewContainer = nullptr;
    std::unordered_map<CCLayerColor*, float> gradientPreviewSprites = {};
    CCNode* gradientTimePreview = nullptr;
    std::vector<CCNode*> gradientLines = {};
    int selectedGradientLine = 0;
    CCMenu* gradientLineConfigNode = nullptr;
    CCSprite* gradientLineColor = nullptr;
    Slider* gradientLineLocation = nullptr;
    CCMenuItemSpriteExtra* gradientAddStepBtn = nullptr;
    CCMenuItemSpriteExtra* gradientDelStepBtn = nullptr;
    CCMenu* gradientStepRow = nullptr;
    Slider* speedSlider = nullptr;
    TextInput* speedInput = nullptr;
    CCNode* colArea = nullptr;
    TextInput* rInput = nullptr;
    TextInput* gInput = nullptr;
    TextInput* bInput = nullptr;
    TextInput* hexInput = nullptr;
    CCNode* shader = nullptr;
    CCMenuItemToggler* gradientMappedToggle = nullptr;
    CCMenuItemToggler* gradientProgressToggle = nullptr;
    CCMenuItemToggler* gradientScrollToggle = nullptr;
    CCLabelBMFont* gradientProgressLabel = nullptr;
    CCLabelBMFont* gradientScrollLabel = nullptr;
    CCMenuItemSpriteExtra* spreadInfoBtn = nullptr;
    CCMenuItemSpriteExtra* progressInfoBtn = nullptr;
    CCMenuItemSpriteExtra* scrollInfoBtn = nullptr;
    ProgressBar* gradientPreviewBar = nullptr;
    std::vector<CCLayerColor*> barOverlaySegments;
    CCMenu* gradientOptsMenu = nullptr;
    bool m_subPopupOpen = false;
    bool m_epilepsyWarned = false;
    float m_previewTime = 0.f;

    void setMenusEnabled(bool enabled) const;
    void fixTouchPriorities();

    void textChanged(CCTextInputNode* node) override;

public:
    static SetupColorConfigUI* create(const std::function<void(ColorConfig)>& onFinishFunc, bool allowEffects = true);

    void show() override;
    void onClose(CCObject* sender) override;
    void onChangeType(CCObject* sender);
    void onSetDefault(CCObject* sender);
    void onUndoChanged(CCObject* sender);
    void onSelectGradientLine(CCObject* sender);
    void onChangeGradientLineColor(CCObject* sender);
    void onGradientLocationSlider(CCObject* sender);
    void onAddGradientStep(CCObject* sender);
    void onDeleteGradientStep(CCObject* sender);
    void onSpeedSliderChanged(CCObject* sender);
    void onToggleGradientMapped(CCObject* sender);
    void onToggleGradientProgress(CCObject* sender);
    void onToggleGradientScroll(CCObject* sender);
    void onSpreadInfo(CCObject*);
    void onProgressInfo(CCObject*);
    void onScrollInfo(CCObject*);

    void setStartConfig(const ColorConfig& config);
    void setDefaultConfig(const ColorConfig& config);
    void updateUI();
    void updateGradientBarMode();
    void updateInputs(const TextInput* except) const;

    void addTypeButtons(CCMenu* menu);
    void updateTypeButtons(const CCMenuItemToggler* excluding) const;

    void createGradientPreview();
    void updateGradientPreview();
    CCNode* createGradientLine(bool timePreview, ccColor3B col = ccWHITE, bool selected = false, int tag = 0);
    void updateGradientLines();

    void colorValueChanged(ccColor3B color) override;
    void update(float dt) override;
    bool setup() override;
};
