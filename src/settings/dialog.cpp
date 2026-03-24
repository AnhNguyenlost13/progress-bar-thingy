// ReSharper disable CppParameterMayBeConstPtrOrRef
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
#include "dialog.hpp"
#include "../color.hpp"

static constexpr float SPEED_MIN = 0.1f;
static constexpr float SPEED_MAX = 5.0f;
static float speedTo01(const float v) { return (v - SPEED_MIN) / (SPEED_MAX - SPEED_MIN); }
static float speedFrom01(const float v) { return v * (SPEED_MAX - SPEED_MIN) + SPEED_MIN; }

SetupColorConfigUI* SetupColorConfigUI::create(const std::function<void(ColorConfig)>& onFinishFunc,
                                               const bool allowEffects)
{
    auto pRet = new SetupColorConfigUI();

    constexpr CCSize size = ccp(380, 250);
    pRet->onFinishFunc = onFinishFunc;
    pRet->allowEffects = allowEffects;

    if (pRet && pRet->initAnchored(size.width, size.height))
    {
        pRet->autorelease();
        return pRet;
    }

    CC_SAFE_DELETE(pRet);
    return nullptr;
}

bool SetupColorConfigUI::setup()
{
    this->scheduleUpdate();

    const auto bg = CCScale9Sprite::create("GJ_square01.png");
    bg->setID("background"_spr);
    bg->setContentSize(m_size);
    m_mainLayer->addChildAtPosition(bg, Anchor::Center);

    m_buttonMenu->setVisible(false);

    const auto title = CCLabelBMFont::create("Color Setup", "goldFont.fnt");
    title->setID("title-label"_spr);
    title->setScale(0.7f);

    const auto menu = CCMenu::create();
    menu->setID("ok-menu"_spr);
    const auto okSpr = ButtonSprite::create("OK", 0, false, "goldFont.fnt", "GJ_button_01.png", 30, 0.7f);
    const auto okBtn = CCMenuItemSpriteExtra::create(okSpr, this, menu_selector(SetupColorConfigUI::onClose));
    okBtn->setID("ok-btn"_spr);
    menu->addChild(okBtn);

    m_mainLayer->addChildAtPosition(title, Anchor::Top, ccp(0, -18));
    m_mainLayer->addChildAtPosition(menu, Anchor::Bottom, ccp(0, 24.5f));

    startColor = CCLayerColor::create(ccc4(0, 0, 0, 255), 30, 15);
    startColor->setID("start-color"_spr);
    startColor->ignoreAnchorPointForPosition(false);
    endColor = CCLayerColor::create(ccc4(0, 0, 0, 255), 30, 30);
    endColor->setID("end-color"_spr);
    endColor->ignoreAnchorPointForPosition(false);

    shader = CCNode::create();
    shader->setID("picker-wrapper"_spr);
    picker = CCControlColourPicker::colourPicker();
    picker->setID("color-picker"_spr);
    picker->setDelegate(this);
    picker->setAnchorPoint(ccp(0, 0));
    picker->setScale(0.8f);
    shader->addChild(picker);

    typeMenu = CCMenu::create();
    typeMenu->setID("type-menu"_spr);
    typeMenu->setContentSize(ccp(0, 0));
    typeMenu->setScale(0.8f);
    typeMenu->setVisible(allowEffects);

    const auto topRightMenu = CCMenu::create();
    topRightMenu->setID("top-right-menu"_spr);
    topRightMenu->setContentSize(ccp(0, 300));
    topRightMenu->setAnchorPoint(ccp(0.5f, 1));
    topRightMenu->setLayout(AxisLayout::create(Axis::Column)
                                ->setAutoScale(false)
                                ->setAxisReverse(true)
                                ->setAxisAlignment(AxisAlignment::End)
                                ->setGap(10));
    topRightMenu->setScale(0.75f);
    const auto defaultBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Default", 0, false, "goldFont.fnt", "GJ_button_04.png", 30, 0.6f), this,
        menu_selector(SetupColorConfigUI::onSetDefault));
    defaultBtn->setID("default-btn"_spr);
    topRightMenu->addChild(defaultBtn);
    if (allowEffects)
    {
        const auto presetBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Presets", 0, false, "goldFont.fnt", "GJ_button_04.png", 30, 0.6f), this,
            menu_selector(SetupColorConfigUI::onPresets));
        presetBtn->setID("preset-btn"_spr);
        topRightMenu->addChild(presetBtn);
    }

    const auto undoDistRow = CCMenu::create();
    undoDistRow->setContentSize(ccp(66, 30));
    undoDistRow->setLayout(AxisLayout::create(Axis::Row)->setAutoScale(false)->setGap(6));

    const auto undoBg = CCScale9Sprite::create("GJ_button_04.png", {0, 0, 40, 40});
    undoBg->setContentSize({30, 30});
    const auto undoIcon = CCSprite::createWithSpriteFrameName("geode.loader/reset-gold.png");
    undoIcon->setPosition(undoBg->getContentSize() / 2);
    undoIcon->setScale(0.7f);
    undoBg->addChild(undoIcon);
    const auto undoBtn = CCMenuItemSpriteExtra::create(undoBg, this, menu_selector(SetupColorConfigUI::onUndoChanged));
    undoBtn->setID("undo-btn"_spr);
    undoDistRow->addChild(undoBtn);

    const auto distBg = CCScale9Sprite::create("GJ_button_04.png", {0, 0, 40, 40});
    distBg->setContentSize({30, 30});
    const auto distIcon = CCLabelBMFont::create("=", "bigFont.fnt");
    distIcon->setPosition(distBg->getContentSize() / 2);
    distIcon->setScale(0.6f);
    distBg->addChild(distIcon);
    const auto distBtn =
        CCMenuItemSpriteExtra::create(distBg, this, menu_selector(SetupColorConfigUI::onDistributeStops));
    distBtn->setID("gradient-dist-btn"_spr);
    undoDistRow->addChild(distBtn);

    undoDistRow->updateLayout();
    topRightMenu->addChild(undoDistRow);

    gradientStepRow = CCMenu::create();
    const auto stepRow = gradientStepRow;
    stepRow->setID("gradient-step-row"_spr);
    stepRow->setContentSize(ccp(80, 30));
    stepRow->setLayout(AxisLayout::create(Axis::Row)->setAutoScale(false)->setGap(6));

    const auto addBg = CCScale9Sprite::create("GJ_button_01.png", {0, 0, 40, 40});
    addBg->setContentSize({30, 30});
    const auto addIcon = CCSprite::createWithSpriteFrameName("edit_addCBtn_001.png");
    addIcon->setPosition(addBg->getContentSize() / 2);
    addIcon->setScale(0.7f);
    addBg->addChild(addIcon);
    gradientAddStepBtn =
        CCMenuItemSpriteExtra::create(addBg, this, menu_selector(SetupColorConfigUI::onAddGradientStep));
    gradientAddStepBtn->setID("gradient-add-step-btn"_spr);

    const auto delBg = CCScale9Sprite::create("GJ_button_06.png", {0, 0, 40, 40});
    delBg->setContentSize({30, 30});
    const auto delIcon = CCSprite::createWithSpriteFrameName("edit_delCBtn_001.png");
    delIcon->setPosition(delBg->getContentSize() / 2);
    delIcon->setScale(0.7f);
    delBg->addChild(delIcon);
    gradientDelStepBtn =
        CCMenuItemSpriteExtra::create(delBg, this, menu_selector(SetupColorConfigUI::onDeleteGradientStep));
    gradientDelStepBtn->setID("gradient-del-step-btn"_spr);

    stepRow->setContentSize(ccp(80, 30));
    stepRow->addChild(gradientAddStepBtn);
    stepRow->addChild(gradientDelStepBtn);
    stepRow->updateLayout();
    topRightMenu->addChild(stepRow);

    const auto loopBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Loop", 60, false, "bigFont.fnt", "GJ_button_04.png", 25, 0.5f), this,
        menu_selector(SetupColorConfigUI::onCloseLoop));
    loopBtn->setID("gradient-loop-btn"_spr);
    topRightMenu->addChild(loopBtn);

    topRightMenu->updateLayout();

    bottomLeft = CCMenu::create();
    bottomLeft->setID("bottom-left-menu"_spr);
    bottomLeft->setVisible(this->allowEffects);
    bottomLeft->setScale(0.65f);
    bottomLeft->setContentSize(ccp(0, 0));

    speedSlider = Slider::create(this, menu_selector(SetupColorConfigUI::onSpeedSliderChanged), 0.5f);
    speedSlider->setID("speed-slider"_spr);
    speedSlider->setPosition(ccp(0, 0));
    speedSlider->setValue(speedTo01(1.0f));
    bottomLeft->addChild(speedSlider);

    speedInput = TextInput::create(45, "Speed");
    speedInput->setID("speed-input"_spr);
    speedInput->setTextAlign(TextInputAlign::Center);
    speedInput->setAnchorPoint(ccp(0, 0.5f));
    speedInput->setPosition(ccp(60.0f, 0));
    speedInput->setDelegate(this);
    bottomLeft->addChild(speedInput);

    const auto speedTitle = CCLabelBMFont::create("Speed", "goldFont.fnt");
    speedTitle->setID("speed-title"_spr);
    speedTitle->setScale(0.65f);
    speedTitle->setPosition(ccp(25.0f, 32));
    bottomLeft->addChild(speedTitle);

    colArea = CCNode::create();
    colArea->setID("color-area"_spr);

    rInput = TextInput::create(50, "R");
    rInput->setID("r-input"_spr);
    rInput->setTextAlign(TextInputAlign::Center);
    rInput->setScale(0.6f);
    rInput->setPosition(ccp(-38, 20));
    rInput->setDelegate(this, 1);

    gInput = TextInput::create(50, "G");
    gInput->setID("g-input"_spr);
    gInput->setTextAlign(TextInputAlign::Center);
    gInput->setScale(0.6f);
    gInput->setPosition(ccp(0, 20));
    gInput->setDelegate(this, 2);

    bInput = TextInput::create(50, "B");
    bInput->setID("b-input"_spr);
    bInput->setTextAlign(TextInputAlign::Center);
    bInput->setScale(0.6f);
    bInput->setPosition(ccp(38, 20));
    bInput->setDelegate(this, 3);

    hexInput = TextInput::create(110, "HEX");
    hexInput->setID("hex-input"_spr);
    hexInput->setTextAlign(TextInputAlign::Center);
    hexInput->setScale(0.6f);
    hexInput->setPosition(ccp(0, -20));
    hexInput->setDelegate(this, 4);

    const auto rTitle = CCLabelBMFont::create("R", "goldFont.fnt");
    rTitle->setID("r-title"_spr);
    rTitle->setScale(0.4f);
    rTitle->setPosition(ccp(-38, 39));

    const auto gTitle = CCLabelBMFont::create("G", "goldFont.fnt");
    gTitle->setID("g-title"_spr);
    gTitle->setScale(0.4f);
    gTitle->setPosition(ccp(0, 39));

    const auto bTitle = CCLabelBMFont::create("B", "goldFont.fnt");
    bTitle->setID("b-title"_spr);
    bTitle->setScale(0.4f);
    bTitle->setPosition(ccp(38, 39));

    const auto hexTitle = CCLabelBMFont::create("HEX", "goldFont.fnt");
    hexTitle->setID("hex-title"_spr);
    hexTitle->setScale(0.4f);
    hexTitle->setPosition(ccp(0, -1));

    colArea->addChild(rInput);
    colArea->addChild(gInput);
    colArea->addChild(bInput);
    colArea->addChild(hexInput);
    colArea->addChild(rTitle);
    colArea->addChild(gTitle);
    colArea->addChild(bTitle);
    colArea->addChild(hexTitle);

    createGradientPreview();
    addTypeButtons(typeMenu);
    updateUI();

    m_mainLayer->addChildAtPosition(shader, Anchor::Center, ccp(0, 0));
    m_mainLayer->addChildAtPosition(endColor, Anchor::TopLeft, ccp(30, -30));
    m_mainLayer->addChildAtPosition(startColor, Anchor::TopLeft, ccp(30, -30 - (15 * 1.5f)));
    m_mainLayer->addChildAtPosition(typeMenu, Anchor::BottomRight, ccp(-80, 20));
    m_mainLayer->addChildAtPosition(topRightMenu, Anchor::TopRight, ccp(-38, -12));
    m_mainLayer->addChildAtPosition(bottomLeft, Anchor::BottomLeft, ccp(65, 25));
    m_mainLayer->addChildAtPosition(colArea, Anchor::Left, ccp(65, 0));

    return true;
}

void SetupColorConfigUI::createGradientPreview()
{
    gradientPreviewContainer = CCNode::create();
    gradientPreviewContainer->setID("gradient-preview"_spr);
    gradientPreviewContainer->setContentSize(ccp(180, 50));
    gradientPreviewContainer->setAnchorPoint(ccp(0.5f, 0.5f));
    gradientPreviewContainer->setZOrder(6);

    constexpr int quality = 400;
    for (size_t i = 0; i < quality; i++)
    {
        float v = static_cast<float>(i) / static_cast<float>(quality);
        const float xPos = v * gradientPreviewContainer->getContentWidth();

        auto spr = CCLayerColor::create(ccc4(0, 0, 0, 255), gradientPreviewContainer->getContentWidth() - xPos,
                                        gradientPreviewContainer->getContentHeight());
        spr->setAnchorPoint(ccp(0, 0));
        spr->ignoreAnchorPointForPosition(false);
        spr->setPosition(ccp(xPos, 0));

        gradientPreviewSprites.emplace(spr, v);
        gradientPreviewContainer->addChild(spr);
    }

    gradientTimePreview = createGradientLine(true);
    gradientTimePreview->setID("gradient-time-preview"_spr);
    gradientPreviewContainer->addChild(gradientTimePreview, 69);

    updateGradientLines();

    gradientLineConfigNode = CCMenu::create();
    gradientLineConfigNode->setID("gradient-config"_spr);

    const auto lblColor = CCLabelBMFont::create("Color:", "bigFont.fnt");
    lblColor->setID("gradient-color-label"_spr);
    lblColor->setAnchorPoint(ccp(0, 0.5f));
    lblColor->setScale(0.35f);
    lblColor->setPosition(ccp(-75, -17));
    gradientLineConfigNode->addChild(lblColor);

    gradientLineColor = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
    gradientLineColor->setID("gradient-line-color"_spr);
    gradientLineColor->setScale(0.65f);

    const auto colBtn = CCMenuItemSpriteExtra::create(gradientLineColor, this,
                                                      menu_selector(SetupColorConfigUI::onChangeGradientLineColor));
    colBtn->setID("gradient-color-btn"_spr);
    colBtn->setPosition(ccp(-22, -17));
    gradientLineConfigNode->addChild(colBtn);

    const auto lblPos = CCLabelBMFont::create("Pos:", "bigFont.fnt");
    lblPos->setID("gradient-pos-label"_spr);
    lblPos->setAnchorPoint(ccp(0, 0.5f));
    lblPos->setScale(0.35f);
    lblPos->setPosition(ccp(0, -17));
    gradientLineConfigNode->addChild(lblPos);

    gradientLinePositionInput = TextInput::create(45, "%");
    gradientLinePositionInput->setID("gradient-pos-input"_spr);
    gradientLinePositionInput->setScale(0.7f);
    gradientLinePositionInput->setPosition(ccp(45, -17));
    gradientLinePositionInput->setDelegate(this, 5);
    gradientLineConfigNode->addChild(gradientLinePositionInput);

    gradientLineLocation = Slider::create(this, menu_selector(SetupColorConfigUI::onGradientLocationSlider), 0.8f);
    gradientLineLocation->setID("gradient-location-slider"_spr);
    gradientLineLocation->setAnchorPoint(ccp(0, 0));
    gradientLineLocation->setPosition(ccp(0, 7));
    gradientLineLocation->m_sliderBar->setOpacity(0);
    gradientLineConfigNode->addChild(gradientLineLocation);

    gradientOptsMenu = CCMenu::create();
    gradientOptsMenu->setID("gradient-options-menu"_spr);
    gradientOptsMenu->setContentSize(ccp(0, 0));

    const auto optsTitle = CCLabelBMFont::create("Options", "goldFont.fnt");
    optsTitle->setID("gradient-options-title"_spr);
    optsTitle->setScale(0.45f);
    optsTitle->setPosition(ccp(0, 20));
    gradientOptsMenu->addChild(optsTitle);

    const auto optsRow = CCMenu::create();
    optsRow->setID("gradient-options-row"_spr);
    optsRow->setContentSize(ccp(300, 20));
    optsRow->setLayout(RowLayout::create()->setAutoScale(false)->setAxisAlignment(AxisAlignment::Center)->setGap(2));

    auto makeToggle = [&](auto callback, float sprScale)
    {
        auto t = CCMenuItemToggler::createWithStandardSprites(this, callback, 1.0f);
        t->m_onButton->getNormalImage()->setScale(sprScale);
        t->m_offButton->getNormalImage()->setScale(sprScale);
        auto sz = t->m_onButton->getNormalImage()->getScaledContentSize();
        t->m_onButton->setContentSize(sz);
        t->m_offButton->setContentSize(sz);
        t->m_onButton->getNormalImage()->setPosition(sz / 2);
        t->m_offButton->getNormalImage()->setPosition(sz / 2);
        t->setContentSize(sz);
        return t;
    };

    auto makeInfoBtn = [&](auto callback)
    {
        auto spr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        spr->setScale(0.35f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, callback);
        btn->setLayoutOptions(AxisLayoutOptions::create()->setNextGap(12));
        return btn;
    };

    gradientMappedToggle = makeToggle(menu_selector(SetupColorConfigUI::onToggleGradientMapped), 0.5f);
    gradientMappedToggle->setID("gradient-mapped-toggle"_spr);
    optsRow->addChild(gradientMappedToggle);

    const auto mappedLabel = CCLabelBMFont::create("Spread", "bigFont.fnt");
    mappedLabel->setID("gradient-mapped-label"_spr);
    mappedLabel->setScale(0.28f);
    optsRow->addChild(mappedLabel);

    spreadInfoBtn = makeInfoBtn(menu_selector(SetupColorConfigUI::onSpreadInfo));
    optsRow->addChild(spreadInfoBtn);

    gradientProgressToggle = makeToggle(menu_selector(SetupColorConfigUI::onToggleGradientProgress), 0.5f);
    gradientProgressToggle->setID("gradient-progress-toggle"_spr);
    optsRow->addChild(gradientProgressToggle);

    gradientProgressLabel = CCLabelBMFont::create("Follow Level", "bigFont.fnt");
    gradientProgressLabel->setID("gradient-progress-label"_spr);
    gradientProgressLabel->setScale(0.28f);
    optsRow->addChild(gradientProgressLabel);

    progressInfoBtn = makeInfoBtn(menu_selector(SetupColorConfigUI::onProgressInfo));
    optsRow->addChild(progressInfoBtn);

    gradientScrollToggle = makeToggle(menu_selector(SetupColorConfigUI::onToggleGradientScroll), 0.5f);
    gradientScrollToggle->setID("gradient-scroll-toggle"_spr);
    optsRow->addChild(gradientScrollToggle);

    gradientScrollLabel = CCLabelBMFont::create("Scroll", "bigFont.fnt");
    gradientScrollLabel->setID("gradient-scroll-label"_spr);
    gradientScrollLabel->setScale(0.28f);
    optsRow->addChild(gradientScrollLabel);

    scrollInfoBtn = makeInfoBtn(menu_selector(SetupColorConfigUI::onScrollInfo));
    optsRow->addChild(scrollInfoBtn);

    gradientMirrorToggle = makeToggle(menu_selector(SetupColorConfigUI::onToggleGradientMirror), 0.5f);
    gradientMirrorToggle->setID("gradient-mirror-toggle"_spr);
    optsRow->addChild(gradientMirrorToggle);

    gradientMirrorLabel = CCLabelBMFont::create("Mirror", "bigFont.fnt");
    gradientMirrorLabel->setID("gradient-mirror-label"_spr);
    gradientMirrorLabel->setScale(0.28f);
    optsRow->addChild(gradientMirrorLabel);

    optsRow->updateLayout();
    gradientOptsMenu->addChild(optsRow);
    optsRow->setPosition(ccp(0, 0));

    gradientPreviewBar = ProgressBar::create(ProgressBarStyle::Level);
    gradientPreviewBar->setID("gradient-preview-bar"_spr);
    gradientPreviewBar->setAnchorPoint(ccp(0.5f, 0.5f));
    gradientPreviewBar->updateProgress(100.f);
    gradientPreviewBar->setFillColor(ccWHITE);
    gradientPreviewBar->setVisible(false);
    gradientPreviewBar->setScale(180.f / gradientPreviewBar->getContentSize().width);

    if (const auto barSpr = gradientPreviewBar->getChildByID("progress-bar"))
    {
        if (const auto fillSpr = barSpr->getChildByID("progress-bar-fill"))
        {
            const float visibleWidth = barSpr->getContentSize().width - 4;
            const float fillHeight = fillSpr->getContentSize().height;
            const int barSegments = static_cast<int>(Mod::get()->getSettingValue<int64_t>("gradient-segments"));
            const float segWidth = visibleWidth / barSegments;
            for (int i = 0; i < barSegments; i++)
            {
                auto seg = CCLayerColor::create(ccc4(255, 255, 255, 255));
                seg->ignoreAnchorPointForPosition(false);
                seg->setAnchorPoint(ccp(0, 0));
                const float x = segWidth * i;
                const float w = fminf(ceilf(segWidth) + 1, visibleWidth - x);
                seg->setContentSize(ccp(w, fillHeight));
                seg->setPosition(ccp(x, 0));
                barOverlaySegments.push_back(seg);
                fillSpr->addChild(seg, 1);
            }
        }
    }

    m_mainLayer->addChildAtPosition(gradientPreviewBar, Anchor::Center, ccp(0, 55));
    m_mainLayer->addChildAtPosition(gradientPreviewContainer, Anchor::Center, ccp(0, 55));
    gradientPreviewContainer->setZOrder(10);
    m_mainLayer->addChildAtPosition(gradientLineConfigNode, Anchor::Center, ccp(0, -5));
    m_mainLayer->addChildAtPosition(gradientOptsMenu, Anchor::Center, ccp(0, -65));
}

CCNode* SetupColorConfigUI::createGradientLine(const bool timePreview, const ccColor3B col, const bool selected,
                                               const int tag)
{
    const auto n = CCNode::create();
    n->setContentSize(ccp(1, 50));
    n->setAnchorPoint(ccp(0.5f, 0));

    const auto layer = CCLayerColor::create(ccc4(0, 0, 0, 200), n->getContentWidth(), n->getContentHeight());
    n->addChild(layer);

    if (timePreview)
    {
        layer->setColor(ccc3(0, 119, 255));

        const auto spr = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
        spr->setAnchorPoint(ccp(0, 0.5f));
        spr->setRotation(-90);
        spr->setScale(0.3f);
        spr->setPosition(ccp(n->getContentWidth() / 2, n->getContentHeight()));
        n->addChild(spr);
    }
    else
    {
        layer->setOpacity(100);

        const auto lineMenu = CCMenu::create();
        lineMenu->setPosition(ccp(n->getContentWidth() / 2, -12));

        const auto spr = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
        spr->setColor(col);
        spr->setScale(0.4f);
        spr->setOpacity(selected ? 255 : 175);

        const auto lineBtn =
            CCMenuItemSpriteExtra::create(spr, this, menu_selector(SetupColorConfigUI::onSelectGradientLine));
        lineBtn->setTag(tag);

        lineMenu->addChild(lineBtn);
        n->addChild(lineMenu);
    }

    return n;
}

void SetupColorConfigUI::updateGradientPreview()
{
    if (!(currentConfig.smoothGradient && gradientPreviewBar->isVisible()))
        for (const auto& [layer, v] : gradientPreviewSprites)
            layer->setColor(currentConfig.colorForGradient(v));

    else
        for (int i = 0; i < static_cast<int>(barOverlaySegments.size()); i++)
            barOverlaySegments[i]->setColor(
                currentConfig.colorForGradient((static_cast<float>(i) + 0.5f) / barOverlaySegments.size()));

    for (const auto line : gradientLines)
        line->setPositionX(gradientPreviewContainer->getContentWidth() *
                           currentConfig.gradientLocations[line->getTag()].percentageLocation);

    gradientLineColor->setColor(currentConfig.gradientLocations[selectedGradientLine].color);
    gradientTimePreview->setPositionX(gradientPreviewContainer->getContentWidth() *
                                      (sinf(colorutil::va * currentConfig.chromaSpeed) + 1.0f) / 2.0f);
}

void SetupColorConfigUI::updateGradientLines()
{
    for (const auto line : gradientLines)
        line->removeFromParent();
    gradientLines.clear();

    int i = 0;
    for (const auto& [color, percentageLocation] : currentConfig.gradientLocations)
    {
        auto l = createGradientLine(false, color, i == selectedGradientLine, i);
        l->setTag(i);
        gradientPreviewContainer->addChild(l, 4);
        gradientLines.push_back(l);
        i++;
    }

    fixTouchPriorities();
}

void SetupColorConfigUI::onSelectGradientLine(CCObject* sender)
{
    selectedGradientLine = sender->getTag();
    updateGradientLines();
    updateGradientBarMode();
}

void SetupColorConfigUI::onChangeGradientLineColor(CCObject*)
{
    if (m_subPopupOpen)
        return;
    m_subPopupOpen = true;
    setMenusEnabled(false);

    const auto ui = create(
        [this](const ColorConfig& conf)
        {
            currentConfig.gradientLocations[selectedGradientLine].color = conf.customColor;
            updateGradientLines();
            updateGradientBarMode();

            m_subPopupOpen = false;
            setMenusEnabled(true);

            // hacky stuff because touch prio sucks
            queueInMainThread(
                [self = Ref(this)]()
                {
                    CCTouchDispatcher::get()->registerForcePrio(self, 2);
                    self->fixTouchPriorities();
                });
        },
        false);

    ui->setStartConfig({currentConfig.gradientLocations[selectedGradientLine].color});
    ui->setDefaultConfig({ccc3(255, 0, 0)});
    ui->show();
}

void SetupColorConfigUI::setMenusEnabled(const bool enabled) const
{
    gradientLineConfigNode->setEnabled(enabled);
    gradientOptsMenu->setEnabled(enabled);
    gradientStepRow->setEnabled(enabled);
    bottomLeft->setEnabled(enabled);
    typeMenu->setEnabled(enabled);

    for (const auto line : gradientLines)
        if (const auto children = line->getChildren())
            for (int i = 0; i < children->count(); i++)
                if (const auto menu = dynamic_cast<CCMenu*>(children->objectAtIndex(i)))
                    menu->setEnabled(enabled);
}

// wtf
void SetupColorConfigUI::fixTouchPriorities()
{
    auto* dispatcher = CCTouchDispatcher::get();

    int popupPrio = -500;
    if (auto* d = typeinfo_cast<CCTouchDelegate*>(this))
        if (const auto* h = dispatcher->findHandler(d))
            popupPrio = h->m_nPriority;

    const int prio = popupPrio - 3;

    auto fixTree = [&](CCNode* node)
    {
        if (!node)
            return;
        if (auto* d = typeinfo_cast<CCTouchDelegate*>(node))
            if (dispatcher->findHandler(d))
                dispatcher->setPriority(prio, d);
        handleTouchPriorityWith(node, prio, true);
    };

    fixTree(gradientLineConfigNode);
    fixTree(gradientOptsMenu);
    fixTree(gradientPreviewContainer);
    fixTree(bottomLeft);
    fixTree(typeMenu);
    fixTree(gradientStepRow);
}

void SetupColorConfigUI::onAddGradientStep(CCObject*)
{
    constexpr float x = 0.5f;
    currentConfig.gradientLocations.push_back({currentConfig.colorForGradient(x), x});
    selectedGradientLine = static_cast<int>(currentConfig.gradientLocations.size()) - 1;
    updateGradientLines();
    updateGradientBarMode();
}

void SetupColorConfigUI::onDeleteGradientStep(CCObject*)
{
    if (currentConfig.gradientLocations.size() == 1)
        return FLAlertLayer::create("Gradient Steps", "There must be at least <cc>one</c> gradient step!", "OK")
            ->show();

    currentConfig.gradientLocations.erase(currentConfig.gradientLocations.begin() + selectedGradientLine);
    selectedGradientLine = 0;
    updateGradientLines();
    updateGradientBarMode();
}

void SetupColorConfigUI::onGradientLocationSlider(CCObject*)
{
    currentConfig.gradientLocations[selectedGradientLine].percentageLocation = gradientLineLocation->getValue();
}

void SetupColorConfigUI::addTypeButtons(CCMenu* menu)
{
    struct TypeEntry
    {
        ColorConfigType type;
        const char* label;
        float x, y;
    };

    static constexpr float row = 30.f;
    static const TypeEntry types[] = {
        {Player1, "P1", 0, row * 5},    {Player2, "P2", 55, row * 5},   {PlayerGlow, "Glow", 0, row * 4},
        {Chroma, "Chroma", 0, row * 3}, {Pastel, "Pastel", 0, row * 2}, {Gradient, "Gradient", 0, row},
    };

    for (const auto& [type, typeLabel, x, y] : types)
    {
        auto toggler =
            CCMenuItemToggler::createWithStandardSprites(this, menu_selector(SetupColorConfigUI::onChangeType), 0.75f);
        toggler->setPosition(ccp(x, y));
        std::string s = typeLabel;
        std::ranges::replace(s, ' ', '-');
        std::ranges::transform(s, s.begin(), tolower);
        toggler->setID((GEODE_MOD_ID + std::string("/type-btn-") + s));

        const auto label = CCLabelBMFont::create(typeLabel, "bigFont.fnt");
        label->setAnchorPoint(ccp(0, 0.5f));
        label->setPosition(ccp(x + 18, y));
        label->setScale(0.35f);
        label->setID((GEODE_MOD_ID + std::string("/type-label-") + s));

        const int idTag = static_cast<int>(std::hash<std::string>{}(s));
        toggler->setTag(idTag);
        label->setTag(idTag);

        menu->addChild(toggler);
        menu->addChild(label);
        configTypes.emplace(toggler, type);
    }

    updateTypeButtons(nullptr);
}

static const int kGradientTag = static_cast<int>(std::hash<std::string>{}("gradient"));

void SetupColorConfigUI::updateTypeButtons(const CCMenuItemToggler* excluding) const
{
    for (const auto& [toggler, type] : configTypes)
    {
        if (toggler == excluding)
            continue;
        toggler->toggle(type == currentConfig.type);
    }

    for (const auto child : CCArrayExt<CCNode*>(typeMenu->getChildren()))
        if (currentConfig.type == Gradient)
            child->setVisible(child->getTag() == kGradientTag);
        else
            child->setVisible(true);
}

void SetupColorConfigUI::onChangeType(CCObject* sender)
{
    const auto btn = typeinfo_cast<CCMenuItemToggler*>(sender);

    if (currentConfig.type == configTypes[btn])
        currentConfig.type = CustomColor;
    else
        currentConfig.type = configTypes[btn];

    updateTypeButtons(btn);
    updateUI();
}

void SetupColorConfigUI::update(const float dt)
{
    m_previewTime += dt * currentConfig.chromaSpeed * 60.f;

    const float savedVa = colorutil::va;
    colorutil::va = m_previewTime;
    startColor->setColor(startConfig.colorForConfig());
    endColor->setColor(currentConfig.colorForConfig());
    colorutil::va = savedVa;

    gradientLineLocation->setValue(currentConfig.gradientLocations[selectedGradientLine].percentageLocation);

    // Update position % input (only when not actively editing)
    if (!gradientLinePositionInput->getInputNode()->m_selected)
    {
        const int pct =
            static_cast<int>(currentConfig.gradientLocations[selectedGradientLine].percentageLocation * 100.f + 0.5f);
        gradientLinePositionInput->setString(std::to_string(pct));
    }

    updateGradientPreview();

    if (currentConfig.gradientScrolling && currentConfig.smoothGradient && gradientPreviewBar->isVisible())
    {
        const float offset = m_previewTime * 0.05f;
        for (int i = 0; i < static_cast<int>(barOverlaySegments.size()); i++)
            barOverlaySegments[i]->setColor(currentConfig.colorForGradientLooped(
                (static_cast<float>(i) + 0.5f) / barOverlaySegments.size() + offset));
    }
}

void SetupColorConfigUI::show()
{
    PopupBase::show();
    // we love touch priority
    queueInMainThread(
        [self = Ref(this)]()
        {
            CCTouchDispatcher::get()->registerForcePrio(self, 2);
            self->fixTouchPriorities();
        });
}

void SetupColorConfigUI::onClose(CCObject* sender)
{
    if (onFinishFunc)
        onFinishFunc(currentConfig);

    PopupBase::onClose(sender);
}

void SetupColorConfigUI::onSetDefault(CCObject*)
{
    currentConfig = defaultConfig;
    selectedGradientLine = 0;
    updateUI();
    updateTypeButtons(nullptr);
}

void SetupColorConfigUI::onUndoChanged(CCObject*)
{
    currentConfig = startConfig;
    selectedGradientLine = 0;
    updateUI();
    updateTypeButtons(nullptr);
}

void SetupColorConfigUI::setStartConfig(const ColorConfig& config)
{
    startConfig = config;
    currentConfig = config;
    updateUI();
    updateTypeButtons(nullptr);
}

void SetupColorConfigUI::setDefaultConfig(const ColorConfig& config) { defaultConfig = config; }

void SetupColorConfigUI::updateGradientBarMode()
{
    const bool fullBar = currentConfig.type == Gradient && currentConfig.smoothGradient;
    gradientPreviewBar->setVisible(fullBar);

    for (const auto& [spr, _] : gradientPreviewSprites)
        spr->setVisible(!fullBar);
    gradientTimePreview->setVisible(!fullBar);

    for (const auto line : gradientLines)
    {
        if (auto* vLine = line->getChildren()->objectAtIndex(0))
            typeinfo_cast<CCNode*>(vLine)->setVisible(!fullBar);

        line->setContentSize(ccp(1, fullBar ? 0 : 50));
        if (line->getChildrenCount() > 1)
            if (const auto menu = dynamic_cast<CCMenu*>(line->getChildren()->objectAtIndex(1)))
                menu->setPositionY(fullBar ? -2 : -12);
    }

    gradientProgressLabel->setString(fullBar ? "Cut" : "Follow Level");
    const bool scrollOn = fullBar && currentConfig.gradientScrolling;
    gradientScrollToggle->setVisible(fullBar);
    gradientScrollLabel->setVisible(fullBar);
    scrollInfoBtn->setVisible(fullBar);
    gradientMirrorToggle->setVisible(scrollOn);
    gradientMirrorLabel->setVisible(scrollOn);

    if (auto* row = gradientOptsMenu->getChildByID("gradient-options-row"_spr))
        row->updateLayout();
}

void SetupColorConfigUI::updateUI()
{
    const bool isGradient = currentConfig.type == Gradient;

    picker->setColorValue(currentConfig.customColor);
    picker->setTouchEnabled(currentConfig.type == CustomColor);
    picker->setVisible(!isGradient);
    gradientPreviewContainer->setVisible(isGradient);
    gradientLineConfigNode->setVisible(isGradient);
    gradientStepRow->setVisible(isGradient);
    if (auto* loopBtn = m_mainLayer->getChildByIDRecursive("gradient-loop-btn"_spr))
        loopBtn->setVisible(isGradient);
    if (auto* distBtn = m_mainLayer->getChildByIDRecursive("gradient-dist-btn"_spr))
        distBtn->setVisible(isGradient);
    gradientOptsMenu->setVisible(isGradient);
    gradientMappedToggle->toggle(currentConfig.smoothGradient);
    gradientProgressToggle->toggle(currentConfig.gradientFollowsProgress);
    gradientScrollToggle->toggle(currentConfig.gradientScrolling);
    gradientMirrorToggle->toggle(currentConfig.gradientMirrorLoop);
    updateGradientLines();
    updateGradientBarMode();

    speedSlider->setValue(speedTo01(currentConfig.chromaSpeed));
    speedInput->setString(utils::numToString<float>(currentConfig.chromaSpeed, 2));
    bottomLeft->setVisible(allowEffects && currentConfig.type >= Chroma);
    colArea->setVisible(!isGradient);
    updateInputs(nullptr);
}

void SetupColorConfigUI::onSpeedSliderChanged(CCObject*)
{
    currentConfig.chromaSpeed = speedFrom01(speedSlider->getValue());
    speedInput->setString(utils::numToString<float>(currentConfig.chromaSpeed, 2));

    if (!m_epilepsyWarned && currentConfig.chromaSpeed >= 2.5f)
    {
        m_epilepsyWarned = true;
        FLAlertLayer::create(
            "Warning",
            "<cr>High speed values</c> may cause <cl>rapid flashing</c>.\n\n"
            "This could be harmful for people with <cy>photosensitive epilepsy</c>. Proceed with caution.",
            "OK")
            ->show();
    }
}

void SetupColorConfigUI::onToggleGradientMapped(CCObject* sender)
{
    currentConfig.smoothGradient = !typeinfo_cast<CCMenuItemToggler*>(sender)->isToggled();
    updateGradientBarMode();
}

void SetupColorConfigUI::onToggleGradientProgress(CCObject* sender)
{
    currentConfig.gradientFollowsProgress = !typeinfo_cast<CCMenuItemToggler*>(sender)->isToggled();
}

void SetupColorConfigUI::onSpreadInfo(CCObject*) // NOLINT(*-convert-member-functions-to-static)
{
    FLAlertLayer::create("Spread Gradient",
                         "Maps the <cl>full gradient</c> across the bar's width.\n\n"
                         "<cg>ON</c>: All gradient colors are visible at once, spread across the bar.\n"
                         "<cr>OFF</c>: The bar shows a single color that cycles through the gradient over time.",
                         "OK")
        ->show();
}

void SetupColorConfigUI::onToggleGradientScroll(CCObject* sender)
{
    currentConfig.gradientScrolling = !typeinfo_cast<CCMenuItemToggler*>(sender)->isToggled();
    updateGradientBarMode();
}

void SetupColorConfigUI::onToggleGradientMirror(CCObject* sender)
{
    currentConfig.gradientMirrorLoop = !typeinfo_cast<CCMenuItemToggler*>(sender)->isToggled();
}

void SetupColorConfigUI::onProgressInfo(CCObject*)
{
    if (currentConfig.smoothGradient)
    {
        FLAlertLayer::create("Cut Gradient",
                             "<cg>ON</c>: Gradient ends at the <cl>progress fill edge</c>.\n"
                             "<cr>OFF</c>: Gradient spans the <cl>full bar</c>, visible past the fill.",
                             "OK")
            ->show();
    }
    else
    {
        FLAlertLayer::create("Follow Level",
                             "<cg>ON</c>: Color is based on your <cl>level progress %</c>.\n"
                             "<cr>OFF</c>: Color cycles through the gradient over <cl>time</c> (use Speed to control).",
                             "OK")
            ->show();
    }
}

void SetupColorConfigUI::onScrollInfo(CCObject*)
{
    FLAlertLayer::create("Scroll",
                         "Scrolls the gradient across the bar like <cl>RGB light strips</c>.\n\n"
                         "The gradient loops seamlessly. Use <cy>Speed</c> to control scroll speed.",
                         "OK")
        ->show();
}

void SetupColorConfigUI::onDistributeStops(CCObject*)
{
    const auto count = currentConfig.gradientLocations.size();
    if (count < 2)
        return;
    std::ranges::sort(currentConfig.gradientLocations,
                      [](const ColorConfig::GradientLocation& a, const ColorConfig::GradientLocation& b)
                      { return a.percentageLocation < b.percentageLocation; });
    for (size_t i = 0; i < count; i++)
        currentConfig.gradientLocations[i].percentageLocation = static_cast<float>(i) / (static_cast<float>(count) - 1);
    selectedGradientLine = 0;
    updateGradientLines();
    updateGradientBarMode();
}

void SetupColorConfigUI::onCloseLoop(CCObject*)
{
    if (currentConfig.gradientLocations.size() < 2)
        return;

    const auto& [firstCol, firstPos] = currentConfig.gradientLocations.front();
    const auto& [lastCol, lastPos] = currentConfig.gradientLocations.back();
    if (firstCol.r == lastCol.r && firstCol.g == lastCol.g && firstCol.b == lastCol.b)
        return Notification::create("The first and last colors are already the same!", NotificationIcon::Warning)
            ->show();

    currentConfig.gradientLocations.push_back({firstCol, 0.f});
    const auto count = currentConfig.gradientLocations.size();
    for (size_t i = 0; i < count; i++)
        currentConfig.gradientLocations[i].percentageLocation = static_cast<float>(i) / (static_cast<float>(count) - 1);
    selectedGradientLine = static_cast<int>(count) - 1;
    updateGradientLines();
    updateGradientBarMode();
}

void SetupColorConfigUI::onPresets(CCObject*)
{
    auto* popup = PresetPopup::create(currentConfig,
                                      [this](const ColorConfig& cfg)
                                      {
                                          currentConfig = cfg;
                                          selectedGradientLine = 0;
                                          updateUI();
                                          updateTypeButtons(nullptr);
                                      });
    popup->show();
}

void SetupColorConfigUI::colorValueChanged(const ccColor3B color)
{
    static bool ig = false;
    if (ig)
        return;
    ig = true;
    currentConfig.customColor = color;
    updateInputs(nullptr);
    ig = false;
}

void SetupColorConfigUI::textChanged(CCTextInputNode* node)
{
    if (node->getTag() > 0)
    {
        switch (node->getTag())
        {
        case 1:
            currentConfig.customColor.r =
                utils::numFromString<int>(rInput->getString()).unwrapOr(currentConfig.customColor.r);
            updateInputs(rInput);
            break;
        case 2:
            currentConfig.customColor.g =
                utils::numFromString<int>(gInput->getString()).unwrapOr(currentConfig.customColor.g);
            updateInputs(gInput);
            break;
        case 3:
            currentConfig.customColor.b =
                utils::numFromString<int>(bInput->getString()).unwrapOr(currentConfig.customColor.b);
            updateInputs(bInput);
            break;
        case 4:
            currentConfig.customColor =
                cc3bFromHexString(hexInput->getString(), true).unwrapOr(currentConfig.customColor);
            updateInputs(hexInput);
            break;
        case 5:
            {
                if (const auto pct = utils::numFromString<int>(gradientLinePositionInput->getString()); pct.isOk())
                    currentConfig.gradientLocations[selectedGradientLine].percentageLocation =
                        std::clamp(pct.unwrap(), 0, 100) / 100.f;
                break;
            }
        default:
            break;
        }
        return;
    }

    currentConfig.chromaSpeed =
        utils::numFromString<float>(speedInput->getString()).unwrapOr(currentConfig.chromaSpeed);
    speedSlider->setValue(speedTo01(currentConfig.chromaSpeed));
}

void SetupColorConfigUI::updateInputs(const TextInput* except) const
{
    if (except != rInput)
        rInput->setString(utils::numToString<int>(currentConfig.customColor.r));
    if (except != gInput)
        gInput->setString(utils::numToString<int>(currentConfig.customColor.g));
    if (except != bInput)
        bInput->setString(utils::numToString<int>(currentConfig.customColor.b));
    if (except != hexInput)
        hexInput->setString(cc3bToHexString(currentConfig.customColor));
    picker->setColorValue(currentConfig.customColor);
}

static ColorConfig makeGradientPreset(std::initializer_list<std::pair<ccColor3B, float>> stops)
{
    ColorConfig c;
    c.type = Gradient;
    c.smoothGradient = true;
    c.gradientLocations.clear();
    for (const auto& [col, pos] : stops)
        c.gradientLocations.push_back({col, pos});
    return c;
}

static Preset makeSimplePreset(const char* name, const ColorConfigType type, const ccColor3B color = ccWHITE,
                               const float speed = 0.5f)
{
    ColorConfig c;
    c.type = type;
    c.customColor = color;
    c.chromaSpeed = speed;
    return {name, c};
}

static std::vector<Preset> getDefaultPresets()
{
    return {makeSimplePreset("Player Col 1", Player1),
            makeSimplePreset("Player Col 2", Player2),
            makeSimplePreset("Chroma", Chroma),
            makeSimplePreset("Pastel", Pastel, ccWHITE, 0.1f),
            {"Rainbow",
             makeGradientPreset({
                 {ccc3(255, 0, 0), 0.f},
                 {ccc3(255, 165, 0), 0.17f},
                 {ccc3(255, 255, 0), 0.33f},
                 {ccc3(0, 255, 0), 0.5f},
                 {ccc3(0, 0, 255), 0.67f},
                 {ccc3(148, 0, 211), 0.83f},
                 {ccc3(255, 0, 0), 1.f},
             })},
            {"Sunset",
             makeGradientPreset({
                 {ccc3(255, 94, 77), 0.f},
                 {ccc3(255, 154, 0), 0.33f},
                 {ccc3(180, 50, 160), 0.67f},
                 {ccc3(80, 20, 120), 1.f},
             })},
            {"Ocean",
             makeGradientPreset({
                 {ccc3(0, 50, 80), 0.f},
                 {ccc3(0, 150, 200), 0.33f},
                 {ccc3(0, 200, 180), 0.67f},
                 {ccc3(100, 255, 220), 1.f},
             })},
            {"Fire",
             makeGradientPreset({
                 {ccc3(255, 0, 0), 0.f},
                 {ccc3(255, 100, 0), 0.33f},
                 {ccc3(255, 200, 0), 0.67f},
                 {ccc3(255, 255, 100), 1.f},
             })},
            {"Pastel Rainbow",
             makeGradientPreset({
                 {ccc3(255, 179, 186), 0.f},
                 {ccc3(255, 223, 186), 0.2f},
                 {ccc3(255, 255, 186), 0.4f},
                 {ccc3(186, 255, 201), 0.6f},
                 {ccc3(186, 225, 255), 0.8f},
                 {ccc3(219, 186, 255), 1.f},
             })},
            // ragebait
            {"Transgender",
             makeGradientPreset({
                 {ccc3(91, 206, 250), 0.f},
                 {ccc3(245, 169, 184), 0.25f},
                 {ccc3(255, 255, 255), 0.5f},
                 {ccc3(245, 169, 184), 0.75f},
                 {ccc3(91, 206, 250), 1.f},
             })}};
}

PresetPopup* PresetPopup::create(const ColorConfig& current, std::function<void(ColorConfig)> onSelect)
{
    auto* ret = new PresetPopup();
    ret->m_onSelect = std::move(onSelect);
    ret->m_currentConfig = current;
    if (ret->initAnchored(300, 230))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool PresetPopup::setup()
{
    const auto bg = CCScale9Sprite::create("GJ_square01.png");
    bg->setContentSize(m_size);
    m_mainLayer->addChildAtPosition(bg, Anchor::Center);
    m_buttonMenu->setVisible(false);

    const auto title = CCLabelBMFont::create("Presets", "goldFont.fnt");
    title->setScale(0.7f);
    m_mainLayer->addChildAtPosition(title, Anchor::Top, ccp(0, -18));

    const auto closeSpr = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
    closeSpr->setScale(0.7f);
    const auto closeBtn = CCMenuItemSpriteExtra::create(closeSpr, this, menu_selector(PresetPopup::onClose));
    const auto closeMenu = CCMenu::create();
    closeMenu->addChild(closeBtn);
    m_mainLayer->addChildAtPosition(closeMenu, Anchor::TopLeft, ccp(0, 0));

    const auto saveSpr =
        ButtonSprite::create("Save Current Config as Preset", 0, false, "goldFont.fnt", "GJ_button_01.png", 30, 0.5f);
    const auto saveBtn = CCMenuItemSpriteExtra::create(saveSpr, this, menu_selector(PresetPopup::onSavePreset));
    const auto saveMenu = CCMenu::create();
    saveMenu->addChild(saveBtn);
    m_mainLayer->addChildAtPosition(saveMenu, Anchor::Bottom, ccp(0, 22));

    const float scrollW = m_size.width - 6;
    const float scrollH = m_size.height - 80;
    m_scroll = ScrollLayer::create(ccp(scrollW, scrollH));
    m_scroll->setPosition(ccp(3, 45));
    m_mainLayer->addChild(m_scroll);

    loadPresets();
    rebuildList();
    this->scheduleUpdate();

    return true;
}

void PresetPopup::show()
{
    PopupBase::show();
    // we love touch prio again
    queueInMainThread(
        [self = Ref(this)]()
        {
            CCTouchDispatcher::get()->registerForcePrio(self, 2);
            handleTouchPriority(self);
        });
}

void PresetPopup::update(const float dt)
{
    m_previewTime += dt * 60.f;

    const float savedVa = colorutil::va;
    colorutil::va = m_previewTime;

    for (auto& [bar, segments, config] : m_cellRefs)
    {
        if (config.type == Gradient && config.smoothGradient && config.gradientScrolling)
        {
            const float offset = m_previewTime * config.chromaSpeed * 0.05f;
            for (int i = 0; i < static_cast<int>(segments.size()); i++)
            {
                const float t = (static_cast<float>(i) + 0.5f) / segments.size();
                segments[i]->setColor(config.colorForGradientLooped(t + offset));
            }
        }
        else if (config.type == Chroma || config.type == Pastel)
            bar->setFillColor(config.colorForConfig());
    }

    colorutil::va = savedVa;
}

void PresetPopup::loadPresets()
{
    m_presets.clear();

    if (auto saved = Mod::get()->getSavedValue<matjson::Value>("user-presets", matjson::Value{});
        saved.isArray() && !saved.asArray().unwrap().empty())
    {
        for (auto& entry : saved)
        {
            if (!entry.isObject() || !entry.contains("name") || !entry.contains("config"))
                continue;
            Preset p;
            p.name = entry["name"].asString().unwrapOr("Unnamed");
            p.config.fromJson(entry["config"]);
            p.config.type = Gradient;
            p.config.smoothGradient = true;
            m_presets.push_back(p);
        }
    }
    else
    {
        m_presets = getDefaultPresets();
        savePresets();
    }
}

void PresetPopup::savePresets()
{
    auto arr = matjson::Value::array();
    for (auto& [name, config] : m_presets)
        arr.push(matjson::makeObject({{"name", name}, {"config", config.toJson()}}));
    Mod::get()->setSavedValue("user-presets", arr);
    (void)Mod::get()->saveData();
}

CCNode* PresetPopup::createCell(const Preset& preset, const int index, const float width)
{
    constexpr float cellH = 40.f;
    auto* cell = CCNode::create();
    cell->setContentSize(ccp(width, cellH));
    cell->setAnchorPoint(ccp(0, 1));

    auto* cellBg = CCLayerColor::create(index % 2 == 0 ? ccc4(255, 255, 255, 20) : ccc4(0, 0, 0, 20));
    cellBg->setContentSize(ccp(width, cellH));
    cell->addChild(cellBg);

    auto* nameLabel = CCLabelBMFont::create(preset.name.c_str(), "bigFont.fnt");
    nameLabel->setAnchorPoint(ccp(0, 0.5f));
    nameLabel->setPosition(ccp(8, cellH / 2));
    nameLabel->limitLabelWidth(65, 0.3f, 0.15f);
    cell->addChild(nameLabel);

    const float barVisualWidth = width - 140;
    auto* bar = ProgressBar::create(ProgressBarStyle::Level);
    bar->setAnchorPoint(ccp(0, 0.5f));
    bar->updateProgress(100.f);
    bar->setFillColor(ccWHITE);
    bar->setScale(barVisualWidth / bar->getContentSize().width);
    bar->setPosition(ccp(78, cellH / 2));

    CellRef ref = {bar, {}, preset.config};
    if (preset.config.type == Gradient && preset.config.smoothGradient)
    {
        bar->setFillColor(ccWHITE);
        if (const auto barSpr = bar->getChildByID("progress-bar"))
        {
            if (const auto fillSpr = barSpr->getChildByID("progress-bar-fill"))
            {
                const float visW = barSpr->getContentSize().width - 4;
                const int segs = static_cast<int>(Mod::get()->getSettingValue<int64_t>("gradient-segments"));
                const float segW = visW / segs;
                for (int i = 0; i < segs; i++)
                {
                    auto seg = CCLayerColor::create(ccc4(255, 255, 255, 255));
                    seg->ignoreAnchorPointForPosition(false);
                    seg->setAnchorPoint(ccp(0, 0));
                    const float x = segW * i;
                    seg->setContentSize(ccp(fminf(ceilf(segW) + 1, visW - x), fillSpr->getContentSize().height));
                    seg->setPosition(ccp(x, 0));
                    seg->setColor(preset.config.colorForGradient((static_cast<float>(i) + 0.5f) / segs));
                    ref.segments.push_back(seg);
                    fillSpr->addChild(seg, 1);
                }
            }
        }
    }
    else
        bar->setFillColor(preset.config.colorForConfig());

    m_cellRefs.push_back(ref);
    cell->addChild(bar);

    auto* cellMenu = CCMenu::create();
    cellMenu->setPosition(ccp(0, 0));

    auto* selectSpr = CCSprite::createWithSpriteFrameName("GJ_selectSongBtn_001.png");
    selectSpr->setScale(0.5f);
    auto* selectBtn = CCMenuItemSpriteExtra::create(selectSpr, this, menu_selector(PresetPopup::onSelectPreset));
    selectBtn->setTag(index);
    selectBtn->setPosition(ccp(width - 38, cellH / 2));
    cellMenu->addChild(selectBtn);

    auto* trashSpr = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
    trashSpr->setScale(0.45f);
    auto* trashBtn = CCMenuItemSpriteExtra::create(trashSpr, this, menu_selector(PresetPopup::onDeletePreset));
    trashBtn->setTag(index);
    trashBtn->setPosition(ccp(width - 15, cellH / 2));
    cellMenu->addChild(trashBtn);

    cell->addChild(cellMenu);
    return cell;
}

void PresetPopup::rebuildList()
{
    m_cellRefs.clear();
    const float width = m_scroll->getContentWidth();
    constexpr float cellH = 40.f;
    const float scrollH = m_scroll->getContentHeight();
    const float contentH = cellH * static_cast<float>(m_presets.size());
    const float totalH = std::max(contentH, scrollH);

    m_scroll->m_contentLayer->removeAllChildren();
    m_scroll->m_contentLayer->setContentSize(ccp(width, totalH));

    for (int i = 0; i < static_cast<int>(m_presets.size()); i++)
    {
        auto* cell = createCell(m_presets[i], i, width);
        cell->setPosition(ccp(0, totalH - cellH * i));
        m_scroll->m_contentLayer->addChild(cell);
    }

    m_scroll->m_contentLayer->setPositionY(-(totalH - scrollH));
}

void PresetPopup::onSelectPreset(CCObject* sender)
{
    if (const int idx = sender->getTag(); idx >= 0 && idx < static_cast<int>(m_presets.size()))
    {
        if (m_onSelect)
            m_onSelect(m_presets[idx].config);
        onClose(nullptr);
    }
}

void PresetPopup::onDeletePreset(CCObject* sender)
{
    if (const int idx = sender->getTag(); idx >= 0 && idx < static_cast<int>(m_presets.size()))
    {
        m_presets.erase(m_presets.begin() + idx);
        savePresets();
        rebuildList();
    }
}

void PresetPopup::onSavePreset(CCObject*)
{
    m_pendingPresetName = "Unnamed";

    auto* alert =
        FLAlertLayer::create(this, "Save Preset", "Enter a name for this preset:\n \n ", "Cancel", "Save", 280);

    auto* nameInput = TextInput::create(180, "Preset name...");
    nameInput->setCallback([this](const std::string& str) { m_pendingPresetName = str.empty() ? "Unnamed" : str; });
    nameInput->setPosition(alert->m_mainLayer->getContentSize() / 2 + ccp(0, -15));
    alert->m_mainLayer->addChild(nameInput, 10);

    alert->show();
}

void PresetPopup::FLAlert_Clicked(FLAlertLayer*, const bool btn2)
{
    if (!btn2)
        return;

    const auto& name = m_pendingPresetName;

    m_presets.push_back({name, m_currentConfig});
    savePresets();
    rebuildList();
}
