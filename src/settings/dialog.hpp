#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "config.hpp"

using namespace geode::prelude;

class SetupColorConfigUI : public Popup, public ColorPickerDelegate, public TextInputDelegate
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
    TextInput* gradientLinePositionInput = nullptr;
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
    CCMenuItemToggler* gradientMirrorToggle = nullptr;
    CCLabelBMFont* gradientMirrorLabel = nullptr;
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
    bool init(const std::function<void(ColorConfig)>& onFinishFunc, bool allowEffects);

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
    void onToggleGradientMirror(CCObject* sender);
    void onSpreadInfo(CCObject*);
    void onProgressInfo(CCObject*);
    void onScrollInfo(CCObject*);
    void onPresets(CCObject*);
    void onDistributeStops(CCObject*);
    void onCloseLoop(CCObject*);

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
};

struct Preset
{
    std::string name;
    ColorConfig config;
};

class PresetPopup : public Popup, public FLAlertLayerProtocol
{
    struct CellRef
    {
        ProgressBar* bar;
        std::vector<CCLayerColor*> segments;
        ColorConfig config;
    };

    std::function<void(ColorConfig)> m_onSelect;
    ColorConfig m_currentConfig;
    ScrollLayer* m_scroll = nullptr;
    std::vector<Preset> m_presets;
    std::vector<CellRef> m_cellRefs;
    float m_previewTime = 0.f;
    std::string m_pendingPresetName;

    void rebuildList();
    CCNode* createCell(const Preset& preset, int index, float width);
    void loadPresets();
    void savePresets();

public:
    static PresetPopup* create(const ColorConfig& current, std::function<void(ColorConfig)> onSelect);
    bool init(const ColorConfig& current, std::function<void(ColorConfig)> onSelect);
    void show() override;
    void update(float dt) override;
    void onSavePreset(CCObject*);
    void onDeletePreset(CCObject* sender);
    void onSelectPreset(CCObject* sender);
    void FLAlert_Clicked(FLAlertLayer* alert, bool btn2) override;
};
