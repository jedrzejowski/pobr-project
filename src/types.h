#pragma once

#include "opencv.h"

using real = double;

namespace type {

	namespace RGB {
		const int Ri = 2;
		const int Gi = 1;
		const int Bi = 0;
	}

	namespace HSL {
		const int Hi = 0;
		const int Li = 1;
		const int Si = 2;
	}

	using VecColor = cv::Vec3f;
	using MatColor = cv::Mat_<VecColor>;

	using VecGray = cv::Vec<real, 1>;
	using MatGray = cv::Mat_<VecGray>;

	VecColor createRGB(const real &R, const real &G, const real &B) {
		return VecColor(B, G, R);
	}

	VecColor createHLS(const real &H, const real &L, const real &S) {
		return VecColor(H, L, S);
	}
}