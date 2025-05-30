#pragma once

#include "color.hpp" // Thanks TheSillyDoggo

template<unsigned N>
struct TemplateStr {
	char buf[N + 1]{};

	constexpr TemplateStr(char const* s) {
		for (unsigned i = 0; i != N; ++i) buf[i] = s[i];
	}

	constexpr operator char const*() const { return buf; }
	constexpr operator std::string_view() const { return std::string_view(buf); }
};

template<unsigned N> TemplateStr(char const (&)[N]) -> TemplateStr<N - 1>;


template <TemplateStr settingName, typename T>
/**
 * @brief getSettingValue but FAST :money_mouth:
 *
 * @tparam T The type of the setting value
 * @return The cached setting value
 */
T fastGetSetting() {
	static T cachedSetting = (geode::listenForSettingChanges<T>(settingName, [](T v) {
	    cachedSetting = v;
	}), geode::Mod::get()->getSettingValue<T>(settingName));
	return cachedSetting;
}

enum class WorkingMode {
	PlayerCol1,
	PlayerCol2,
	PlayerGlow,
	Chroma,
	Pastel,
	Custom,
	// Gradient, // who knows
	Unknown     // This should never happen either lmao
};

enum class Context {
	Normal,
	Practice,
	NewBest,
	Unknown // hate you
};

/**
 * @brief string to enum converter real
 *
 * @param mode Working mode string
 * @return the enum
 */
inline WorkingMode getWorkingMode(const std::string& mode)
{
    static const std::unordered_map<std::string, WorkingMode> modeMap = {
        {"Player Col 1", WorkingMode::PlayerCol1}, // 0
        {"Player Col 2", WorkingMode::PlayerCol2}, // 1
        {"Player Glow", WorkingMode::PlayerGlow}, // 2
        {"Chroma", WorkingMode::Chroma}, // 3
        {"Pastel", WorkingMode::Pastel}, // 4
        {"Custom", WorkingMode::Custom} // 5
        // {"Gradient", WorkingMode::Gradient} /* Placeholder */
    };

    const auto x = modeMap.find(mode);
    return x != modeMap.end() ? x->second : WorkingMode::Unknown;
}

inline std::string contextKey(const Context context) {
	static const std::unordered_map<Context, std::string> contextMap = {
        {Context::Normal, "normal"}, {Context::Practice, "practice"}, {Context::NewBest, "enby"}};

    const auto ret = contextMap.find(context);
	return ret != contextMap.end() ? ret->second : "normal";
}

class Catgirl {
public:
	static Catgirl* getInstance() {
		if (meow == nullptr) {
			meow = new Catgirl();
		}
		return meow;
	}

	void updateSettings() {
		normalWorkingMode = getWorkingMode(fastGetSetting<"normal-working-mode", std::string>());
		practiceWorkingMode = getWorkingMode(fastGetSetting<"practice-working-mode", std::string>());
		newBestWorkingMode = getWorkingMode(fastGetSetting<"enby-working-mode", std::string>());

		normalCustomColor = fastGetSetting<"normal-custom-color", ccColor3B>();
		practiceCustomColor = fastGetSetting<"practice-custom-color", ccColor3B>();
		enbyCustomColor = fastGetSetting<"enby-custom-color", ccColor3B>();

		practiceOverride = fastGetSetting<"practice-override", bool>();

		practiceRgbSpeed = fastGetSetting<"practice-rgb-speed", double_t>();
		enbyRgbSpeed = fastGetSetting<"enby-rgb-speed", double_t>();
		normalRgbSpeed = fastGetSetting<"normal-rgb-speed", double_t>();

		// Unused
		// if (normalWorkingMode == WorkingMode::Chroma || normalWorkingMode == WorkingMode::Pastel || practiceWorkingMode == WorkingMode::Chroma || practiceWorkingMode == WorkingMode::Pastel || newBestWorkingMode == WorkingMode::Chroma || newBestWorkingMode == WorkingMode::Pastel) { 
		// 	dynamic = true; 
		// } else { 
		// 	dynamic = false; 
		// }
	}
	
	WorkingMode normalWorkingMode;
	WorkingMode practiceWorkingMode;
	WorkingMode newBestWorkingMode;

	bool practiceToggle;
	bool practiceOverride;
	double_t practiceRgbSpeed;
	double_t enbyRgbSpeed;
	double_t normalRgbSpeed;
	ccColor3B normalCustomColor;
	ccColor3B practiceCustomColor;
	ccColor3B enbyCustomColor;
	Context context;

	bool dynamic;

private:
	static Catgirl* meow;
	
	Catgirl() {
		updateSettings();
	}

	Catgirl(const Catgirl&) = delete;
	Catgirl& operator=(const Catgirl&) = delete;

protected:
	// your mom
};

Catgirl* Catgirl::meow = nullptr;
/**
 * @brief this also gets... the color, but this depends on the level state as well!
 *
 * @return c o l o r
 */
inline ccColor3B paint() {
    const Catgirl* delegate = Catgirl::getInstance();
    const Context context = delegate->context;
	WorkingMode workingMode;
    const auto gm = GameManager::sharedState();

	// Check for the context first
	switch (context) {
		case Context::Normal:
			workingMode = delegate->normalWorkingMode;
			break;
		case Context::Practice:
			workingMode = delegate->practiceWorkingMode;
			break;
		case Context::NewBest:
			workingMode = delegate->newBestWorkingMode;
			break;
		default:
			workingMode = delegate->normalWorkingMode;
			break;
	}

	// Decide the working mode
	switch (workingMode) {
		case WorkingMode::PlayerCol1:
			return gm->colorForIdx(gm->getPlayerColor());
		case WorkingMode::PlayerCol2:
			return gm->colorForIdx(gm->getPlayerColor2());
		case WorkingMode::PlayerGlow:
			return gm->colorForIdx(gm->getPlayerGlowColor());
		case WorkingMode::Chroma:
			return colorutil::getChromaColour();
		case WorkingMode::Pastel:
			return colorutil::getPastelColour();
		case WorkingMode::Custom:
			switch (context) {
				case Context::Normal:
					return delegate->normalCustomColor;
				case Context::Practice:
					return delegate->practiceCustomColor;
				case Context::NewBest:
					return delegate->enbyCustomColor;
				default:
					return {0, 0, 0};
			}
		default:
			return {252, 181, 255}; // If your color is stuck at this please report
	}
}

/**
 * @brief Gets the color change rate based on some factors.
 * @return float
 */
inline float getSpeed() {

    switch (const Catgirl* delegate = Catgirl::getInstance(); delegate->context) {
		case Context::Normal:
			return delegate->normalRgbSpeed;
		case Context::Practice:
			return delegate->practiceRgbSpeed;
		case Context::NewBest:
			return delegate->enbyRgbSpeed;
		default:
			return delegate->normalRgbSpeed;
	};
}