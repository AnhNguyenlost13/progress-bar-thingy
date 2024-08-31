#pragma once

#include <Geode/Geode.hpp>

class colorutil
{
	public:

		static cocos2d::ccColor3B cc3x(int hexValue) {
			if (hexValue <= 0xf)
				return geode::prelude::ccColor3B{
					static_cast<GLubyte>(hexValue * 17),
					static_cast<GLubyte>(hexValue * 17),
					static_cast<GLubyte>(hexValue * 17)};
			if (hexValue <= 0xff)
				return geode::prelude::ccColor3B{
					static_cast<GLubyte>(hexValue),
					static_cast<GLubyte>(hexValue),
					static_cast<GLubyte>(hexValue)};
			if (hexValue <= 0xfff)
				return geode::prelude::ccColor3B{
					static_cast<GLubyte>((hexValue >> 8 & 0xf) * 17),
					static_cast<GLubyte>((hexValue >> 4 & 0xf) * 17),
					static_cast<GLubyte>((hexValue >> 0 & 0xf) * 17)};
			else
				return geode::prelude::ccColor3B{
					static_cast<GLubyte>(hexValue >> 16 & 0xff),
					static_cast<GLubyte>(hexValue >> 8 & 0xff),
					static_cast<GLubyte>(hexValue >> 0 & 0xff)};
		}
		

		static inline int pastel = 0;

		static inline float va = 0.0f;
		static inline float totalSessionTime = 0.0f;

		static void update(float dt)
		{
			va += dt;
		}

		static geode::prelude::ccColor3B hsvToRgb(const geode::prelude::ccHSVValue& hsv) {
			float hue = hsv.h;
			float saturation = hsv.s;
			float value = hsv.v;

			int hi = static_cast<int>(std::floor(hue / 60.0f)) % 6;
			float f = hue / 60.0f - std::floor(hue / 60.0f);

			float p = value * (1 - saturation);
			float q = value * (1 - f * saturation);
			float t = value * (1 - (1 - f) * saturation);

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

			return geode::prelude::ccc3(static_cast<uint8_t>(r * 255), static_cast<uint8_t>(g * 255), static_cast<uint8_t>(b * 255));
		}

		static geode::prelude::ccColor3B getChromaColour()
		{
			return hsvToRgb(geode::prelude::cchsv((va * 180) / 10.0f, 1.0f, 1.0f, true, true));
		}

		static geode::prelude::ccColor3B getPastelColour(int i = 0)
		{
			return hsvToRgb(geode::prelude::cchsv((va * 180) / 10.0f, 155.0f / 255.0f, 1.0f, true, true));
		}
};