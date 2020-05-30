#pragma once

#include "types.h"
#include "calc.h"

namespace color {
	type::VecColor vecRGB2HSL(const type::VecColor &input) {
		// https://www.rapidtables.com/convert/color/rgb-to-hsl.html

		const auto R = calc::trimNorm(input[type::RGB::Ri]);
		const auto G = calc::trimNorm(input[type::RGB::Gi]);
		const auto B = calc::trimNorm(input[type::RGB::Bi]);
		real H, S, L;

		real Cmax = std::max(std::max(R, G), B);
		real Cmin = std::min(std::min(R, G), B);

		L = (Cmax + Cmin) / 2;

		real delta = Cmax - Cmin;

		if (Cmax == R) {
			H = 60 * fmod((G - B) / delta, 6);
		} else if (Cmax == G) {
			H = 60 * ((B - R) / delta + 2);
		} else if (Cmax == B) {
			H = 60 * ((R - G) / delta + 4);
		} else {
			H = 0;
		}


		if (L < 0.5) {
			S = (Cmax - Cmin) / (Cmax + Cmin);
		} else {
			S = (Cmax - Cmin) / (2.0 - Cmax - Cmin);
		}

		return type::createHLS(
				calc::trimHue(H),
				calc::trimNorm(L),
				calc::trimNorm(S)
		);
	}

	type::VecColor vecHSL2RGB(const type::VecColor &input) {
		// https://www.rapidtables.com/convert/color/hsl-to-rgb.html

		const auto H = calc::trimHue(input[type::HSL::Hi]);
		const auto S = calc::trimNorm(input[type::HSL::Si]);
		const auto L = calc::trimNorm(input[type::HSL::Li]);
		real R = 0, G = 0, B = 0;

		real C = (1 - std::abs(2 * L - 1)) * S;

		real X = C * (1 - std::abs(fmod(H / 60, 2) - 1));

		real m = L - (C / 2);

		if (H < 60) {
			R = C;
			G = X;
			B = 0;
		} else if (H < 120) {
			R = X;
			G = C;
			B = 0;
		} else if (H < 180) {
			R = 0;
			G = C;
			B = X;
		} else if (H < 240) {
			R = 0;
			G = X;
			B = C;
		} else if (H < 300) {
			R = X;
			G = 0;
			B = C;
		} else if (H < 360) {
			R = C;
			G = 0;
			B = X;
		}

//		m = 0;
		return type::createRGB(
				calc::trimNorm(R + m),
				calc::trimNorm(G + m),
				calc::trimNorm(B + m)
		);
	}


	type::MatColor matRGB2HSL(const type::MatColor &original) {
		auto original_hsl = original.clone();

		original_hsl.forEach([&](auto &pixel, const int position[]) -> void {
			pixel = color::vecRGB2HSL(pixel);
		});

		return original_hsl;
	}
}