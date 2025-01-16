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


template <TemplateStr setting, typename T>
/**
 * @brief getSettingValue but FAST :money_mouth:
 * 
 * @tparam T The type of the setting value
 * @return The cached setting value
 */
T fastGetSetting() {
	static T cachedSetting = Mod::get()->getSettingValue<T>(setting);
	geode::listenForSettingChanges<T>(setting, [](T v) {
		cachedSetting = v;
	});

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

/**
 * @brief string to enum converter real
 * 
 * @param mode Working mode string
 * @return the enum
 */
WorkingMode getWorkingMode(const std::string& mode) {
	static const std::unordered_map<std::string, WorkingMode> modeMap = {
		{"Player Col 1", WorkingMode::PlayerCol1},
		{"Player Col 2", WorkingMode::PlayerCol2},
		{"Player Glow", WorkingMode::PlayerGlow},
		{"Chroma", WorkingMode::Chroma},
		{"Pastel", WorkingMode::Pastel},
		{"Custom", WorkingMode::Custom}
		// {"Gradient", WorkingMode::Gradient} /* Placeholder */
	};

	auto idkWhatToNameThis = modeMap.find(mode);
	return idkWhatToNameThis != modeMap.end() ? idkWhatToNameThis->second : WorkingMode::Unknown;
}

/**
 * @brief Gets the color for a given working mode.
 * 
 * @param mode the working mode
 * @param customColorKey (only used when working mode is custom)
 * @return c o l o r
 */
ccColor3B getColor(WorkingMode mode, const std::string& customColorKey) {
	auto gm = GameManager::sharedState();
	switch (mode) {
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
			return Mod::get()->getSettingValue<ccColor3B>(customColorKey);
		default:
			return {255, 255, 255}; // Default to white, though this should never happen
	}
}

/**
 * @brief this also gets... the color, but this depends on the level state as well!
 * 
 * @return c o l o r
 */
ccColor3B paint() {
	auto gm = GameManager::sharedState(); // For convenience
	auto gjbgl = GJBaseGameLayer::get();
	auto pl = PlayLayer::get();

	auto normalWorkingMode = getWorkingMode(fastGetSetting<"normal-working-mode", std::string>());
	auto practiceWorkingMode = getWorkingMode(fastGetSetting<"practice-working-mode", std::string>());
	auto newBestWorkingMode = getWorkingMode(fastGetSetting<"enby-working-mode", std::string>());
	auto practiceToggle = fastGetSetting<"practice-mode-toggle", bool>();
	auto practiceOverride = fastGetSetting<"practice-override", bool>();

	auto levelPercentage = as<float>(PlayLayer::get()->getCurrentPercentInt()); // Level progress
	auto levelBest = pl->m_level->m_normalPercent; // Level best

	if (levelPercentage > levelBest) {
		if (gjbgl->m_isPracticeMode && practiceOverride) {
			return getColor(practiceWorkingMode, "practice-custom-color");
		} else {
			return getColor(newBestWorkingMode, "enby-custom-color");
		}
	} else {
		if (gjbgl->m_isPracticeMode && practiceToggle) {
			return getColor(practiceWorkingMode, "practice-custom-color");
		} else {
			return getColor(normalWorkingMode, "normal-custom-color");
		}
	}
}

/**
 * @brief Gets the color change rate based on some factors.
 * @return float
 */
float getSpeed() {
	auto gjbgl = GJBaseGameLayer::get();
	auto pl = PlayLayer::get();

	auto practiceToggle = fastGetSetting<"practice-mode-toggle", bool>();
	auto practiceOverride = fastGetSetting<"practice-override", bool>();

	auto levelPercentage = as<float>(PlayLayer::get()->getCurrentPercentInt());
	auto levelBest = pl->m_level->m_normalPercent;

	if (levelPercentage > levelBest) {
		if (gjbgl->m_isPracticeMode && practiceOverride) {
			return fastGetSetting<"practice-rgb-speed", double_t>();
		} else {
			return fastGetSetting<"enby-rgb-speed", double_t>();
		}
	} else {
		if (gjbgl->m_isPracticeMode && practiceToggle) {
			return fastGetSetting<"practice-rgb-speed", double_t>();
		} else {
			return fastGetSetting<"normal-rgb-speed", double_t>();
		}
	}
}