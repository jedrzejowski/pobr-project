#pragma once

#include "opencv.h"
#include "lib.h"
#include "color.h"
#include "paramWX.h"


cv::Rect2i optentialZottArea(const type::MatGray &figure) {
	auto S = calc::countArea(figure);

	auto m_00 = param::m_pq(figure, 0, 0);
	auto m_10 = param::m_pq(figure, 1, 0);
	auto m_01 = param::m_pq(figure, 0, 1);

	real dash_i = m_10 / m_00;
	real dash_j = m_01 / m_00;

	auto r = cv::sqrt(S / CV_PI);
	auto offset = r * 11;

	auto sub_rect = cv::Rect2i(dash_j - offset, dash_i - offset, 2 * offset, 2 * offset);
	auto img_rec = cv::Rect2i(cv::Point(0, 0), figure.size());

	return img_rec & sub_rect;
}

void findZott(const std::string &name) {

	const auto original = lib::loadAsset(name);
	lib::showMat(name + "/original", original);

	auto original_hsl = color::matRGB2HSL(original.clone());

	auto red_image = type::MatGray(original_hsl.rows, original_hsl.cols);

	red_image.forEach([&](auto &pixel, const int position[]) -> void {
		const auto &color = original_hsl(position[0], position[1]);
		const auto &H = color[type::HSL::Hi];
		const auto &L = color[type::HSL::Li];
		const auto &S = color[type::HSL::Si];

		pixel[0] = (
						   (H < 45 || H > 360 - 45) &&
						   (0.25 < S) &&
						   (0.25 < L && L < 0.75)
				   ) ? 1 : 0;
	});

	lib::showMat(name + "/red_image", red_image);

	auto red_opened = calc::opening(red_image);
	lib::showMat(name + "/red_opened", red_opened);

	auto figures = calc::findGrayFigures(red_opened);
	std::list<cv::Rect2i> potential_zotts;

	std::atomic<int> i = 0;

	std::mutex mutex;

	// Szukamy kółka
	std::for_each(figures.begin(), figures.end(), [&](const auto &fig) {

		auto W3 = param::W3(fig);
		if (cv::abs(W3) > 0.25) return;

		auto W4 = param::W4(fig);
		if (!calc::isInOffset(W4, 1, 0.25)) return;

		auto S = calc::countArea(fig);
		if (S < 10) return;

		auto rect = optentialZottArea(fig);

		mutex.lock();
		potential_zotts.push_back(rect);
		mutex.unlock();
	});


	std::for_each(potential_zotts.begin(), potential_zotts.end(), [&](const cv::Rect2i &rect) {
		auto fig_red = red_image(rect);
		auto fig_red_opened = red_opened(rect);

		// obrazy, które maiją za moało czerwieni są odrzucane
		auto S = calc::countArea(fig_red);
		if (S / rect.area() < 0.12) return;

		auto center = cv::Point2i(rect.width / 2, rect.height / 2);
		auto circle = calc::findGrayFigure(fig_red_opened, center);
		auto circle_S = calc::countArea(circle);
		auto circle_r = cv::sqrt(S / CV_PI);

		auto fig_red_eroded = calc::erosion(fig_red);
		auto size = std::max(rect.width, rect.height);
		for (int i = int(size / 100) - 1; i >= 0; --i) {
			fig_red_eroded = calc::erosion(fig_red_eroded);
		}

		auto my_i = ++i;
		std::stringstream ss;
		ss << "\t" << my_i;
		auto sub_figs = calc::findGrayFigures(fig_red_eroded);


		bool was_top = false, was_bottom = false;

		int w = 0;
		for (const auto &sub_fig : sub_figs) {
			w++;
			auto M1 = param::M1(sub_fig);
//			auto M7 = param::M7(sub_fig);
//			auto W3 = param::W3(sub_fig);
//			auto W4 = param::W4(sub_fig);

			if (M1 > 0.2) {
				was_bottom = true;
			}

//			ss << "\t\t" << w << " M1=" << M1 << " M7=" << M7 << " W3=" << W3 << " W4=" << W4 << std::endl;
//
//			mutex.lock();
//			lib::showMat(name + "/fig/" + std::to_string(my_i) + "/" + std::to_string(w), sub_fig);
//			mutex.unlock();
		}

		if (was_bottom) {

			mutex.lock();
			std::cout << ss.str() << std::endl;
			lib::showMat(name + "/fig/" + std::to_string(my_i), fig_red_eroded);
			mutex.unlock();
		}
	});
}
