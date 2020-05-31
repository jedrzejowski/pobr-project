#pragma once

#include "opencv.h"
#include "lib.h"
#include "color.h"
#include "paramWX.h"


void findZott(const std::string &name) {

	const auto original = lib::loadAsset(name);
	lib::showMat(name + "/original", original);

	auto original_hsl = color::matRGB2HSL(original.clone());

	auto red_image = type::MatGray(original_hsl.rows, original_hsl.cols);
	auto white_image = type::MatGray(original_hsl.rows, original_hsl.cols);

	original_hsl.forEach([&](auto &pixel, const int position[]) -> void {
		const auto &color = original_hsl(position[0], position[1]);
		const auto &H = color[type::HSL::Hi];
		const auto &L = color[type::HSL::Li];
		const auto &S = color[type::HSL::Si];

		red_image(position[0], position[1]) = (
													  (H < 45 || H > 360 - 45) &&
													  (0.25 < S) &&
													  (0.25 < L && L < 0.75)
											  ) ? 1 : 0;


		white_image(position[0], position[1]) = (
														(L > 0.6)
												) ? 1 : 0;
	});

	lib::showMat(name + "/white_image", white_image);
	lib::showMat(name + "/red_image", red_image);

//	lib::showMat(name + "/red_opened", red_opened);

	const auto red_opened = calc::opening(red_image);
	const auto red_dilation = calc::dilation(red_image);
	const auto white_dilation = calc::dilation(white_image);

	auto figures = calc::findGrayFigures(red_opened);

	std::atomic<int> i = 0;

	std::mutex mutex;

	// Szukamy kółka
	std::for_each(figures.begin(), figures.end(), [&](const type::MatGray &fig_opened) {
		int my_i = ++i;
		std::stringstream ss;
		ss << "\t" << my_i << " ";

		cv::Rect2i zott_rect;
		cv::Point2i zott_center;
		real circle_S, circle_r;

		// Sprawdzymy czy mamy doczynienia z kołem
		{
			circle_S = calc::countArea(fig_opened);
			if (circle_S < 10) return;

			auto W3 = param::W3(fig_opened);
			if (cv::abs(W3) > 0.25) return;

			auto W4 = param::W4(fig_opened);
			if (!calc::isInOffset(W4, 1, 0.25)) return;

			//Wyliczenie parametrów koła

			auto m_00 = param::m_pq(fig_opened, 0, 0);
			auto m_10 = param::m_pq(fig_opened, 1, 0);
			auto m_01 = param::m_pq(fig_opened, 0, 1);

			real dash_i = m_10 / m_00;
			real dash_j = m_01 / m_00;
			zott_center = cv::Point2i(dash_j, dash_i);

			circle_r = cv::sqrt(circle_S / CV_PI);
			auto offset = circle_r * 11;

			auto sub_rect = cv::Rect2d(dash_j - offset, dash_i - offset, 2 * offset, 2 * offset);
			auto img_rec = cv::Rect2d(cv::Point(0, 0), fig_opened.size());

			zott_rect = img_rec & sub_rect;
		}

		auto fig_red = red_image(zott_rect);
		auto fig_red_opened = red_opened(zott_rect);

		// sprawdza czy jest odpowiednio duzo czerwonego w obrazie
		if (calc::countArea(fig_red_opened) / zott_rect.area() < 0.10) return;

		auto fig_white = white_image(zott_rect);
//		auto fig_red_dilation = red_dilation(zott_rect);

		// wyliczamy czy kółko jest pełne
		{
			type::MatGray fig_sum = fig_white + fig_red;
			calc::trimImage(fig_sum);
			fig_sum = calc::dilation(fig_sum);

			std::atomic<int> sum = false;
			real zott_r2 = cv::pow(circle_r * 4, 2);

			cv::Point2i zott_center_here = zott_center - zott_rect.tl();

			fig_sum.forEach([&](const auto &pixel, const int position[]) {

				if (cv::pow(position[0] - zott_center_here.y, 2) +
					cv::pow(position[1] - zott_center_here.x, 2) < zott_r2 &&
					pixel[0] > 0.5) {
					sum++;
				}
			});

			if (sum < zott_r2 * CV_PI * 0.95) return;
		}

		auto fig_white_dilation = white_dilation(zott_rect);

		{
//			mutex.lock();
//			lib::showMat(name + "/fig/" + std::to_string(my_i) + "/fig_white_dilation", fig_white_dilation);
//			mutex.unlock();
		}
////		auto fig_red_eroded = calc::erosion(fig_red);
////		auto size = std::max(rect.width, rect.height);
////		for (int i = int(size / 100) - 1; i >= 0; --i) {
////			fig_red_eroded = calc::erosion(fig_red_eroded);
////		}
//
////		auto my_i = ++i;
////		std::stringstream ss;
////		ss << "\t" << my_i;
////		auto sub_figs = calc::findGrayFigures(fig_red_eroded);
//
//
//		bool was_top = false, was_bottom = false;
//
//		int w = 0;
//		for (const auto &sub_fig : sub_figs) {
//			w++;
//			auto M1 = param::M1(sub_fig);
////			auto M7 = param::M7(sub_fig);
////			auto W3 = param::W3(sub_fig);
////			auto W4 = param::W4(sub_fig);
//
//			if (M1 > 0.2) {
//				was_bottom = true;
//			}
//
////			ss << "\t\t" << w << " M1=" << M1 << " M7=" << M7 << " W3=" << W3 << " W4=" << W4 << std::endl;
////
////			mutex.lock();
////			lib::showMat(name + "/fig/" + std::to_string(my_i) + "/" + std::to_string(w), sub_fig);
////			mutex.unlock();
//		}
//
//		if (was_bottom) {

		mutex.lock();
		std::cout << ss.str() << zott_center << " " << circle_S << " " << circle_r << std::endl;
		lib::showMat(name + "/fig/" + std::to_string(my_i), fig_red);
		mutex.unlock();
//		}
	});
}
