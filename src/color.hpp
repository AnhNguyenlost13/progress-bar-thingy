#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class colorutil
{
	public:
		static inline float va = 0.0f;

		static void update(const float dt)
		{
			va += dt;
		}

		static ccColor3B hsvToRgb(const ccHSVValue& hsv) {
			const float hue = hsv.h;
			const float saturation = hsv.s;
			const float value = hsv.v;

			const int hi = static_cast<int>(std::floor(hue / 60.0f)) % 6;
			const float f = hue / 60.0f - std::floor(hue / 60.0f);

			const float p = value * (1 - saturation);
			const float q = value * (1 - f * saturation);
			const float t = value * (1 - (1 - f) * saturation);

			float r, g, b;

			switch (hi) {
				case 0: r = value; g = t; b = p; break;
				case 1: r = q; g = value; b = p; break;
				case 2: r = p; g = value; b = t; break;
				case 3: r = p; g = q; b = value; break;
				case 4: r = t; g = p; b = value; break;
				case 5: r = value; g = p; b = q; break;
				default: r = g = b = 0; break;
			}

			return ccc3(static_cast<uint8_t>(r * 255), static_cast<uint8_t>(g * 255), static_cast<uint8_t>(b * 255));
		}

		static ccColor3B getChromaColour()
		{
			return hsvToRgb(cchsv((va * 180) / 10.0f, 1.0f, 1.0f, true, true));
		}

		static ccColor3B getPastelColour(int i = 0)
		{
			return hsvToRgb(cchsv((va * 180) / 10.0f, 155.0f / 255.0f, 1.0f, true, true));
		}
};