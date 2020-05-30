#pragma once

#include "types.h"
#include "calc.h"
#include "const.h"
#include "paramMX.h"

namespace param {


	real W3(const type::MatGray &fig) {
		auto img_perimeter = calc::filter(fig, edge_filter_matrix);
		auto S = calc::countArea(fig);
		auto L = calc::countPerimeter(img_perimeter);

		return (L / (2 * cv::sqrt(S * CV_PI))) - 1;
	}

	real W4(const type::MatGray &fig) {
		auto img_perimeter = calc::filter(fig, edge_filter_matrix);

		auto S = calc::countArea(fig);

		auto dash_i = param::m_pq(fig, 1, 0) / param::m_pq(fig, 0, 0);
		auto dash_j = param::m_pq(fig, 0, 1) / param::m_pq(fig, 0, 0);

		real calka = calc::sum<type::VecGray>(fig, [&](const auto &pixel, const int position[]) -> real {
			if (pixel[0] < 0.5) return 0;

			real r = cv::pow(dash_i - real(position[0]), 2) + cv::pow(dash_j - real(position[1]), 2);
//			r = cv::pow(cv::sqrt(r, 2), 2);
			return r;
		});

		return S / cv::sqrt(2 * CV_PI * calka);
	}

	real W6(const type::MatGray &fig) {
		auto img_perimeter = calc::filter(fig, edge_filter_matrix);

		auto n = calc::countArea(img_perimeter);

		auto dash_i = param::m_pq(fig, 1, 0) / param::m_pq(fig, 0, 0);
		auto dash_j = param::m_pq(fig, 0, 1) / param::m_pq(fig, 0, 0);

		real sigma_d = calc::sum<type::VecGray>(fig, [&](const auto &pixel, const int position[]) -> real {
			if (pixel[0] < 0.5) return 0;

			real r = cv::pow(dash_i - real(position[0]), 2) + cv::pow(dash_j - real(position[1]), 2);
			return cv::sqrt(r);
		});

		real sigma_d2 = calc::sum<type::VecGray>(fig, [&](const auto &pixel, const int position[]) -> real {
			if (pixel[0] < 0.5) return 0;

			real r = cv::pow(dash_i - real(position[0]), 2) + cv::pow(dash_j - real(position[1]), 2);
//			r = cv::pow(cv::sqrt(r), 2);
			return r;
		});

		auto sd2 = cv::pow(sigma_d, 2);

		return cv::sqrt(cv::pow(sigma_d, 2) / (n * sigma_d2 - 1));
	}
}

