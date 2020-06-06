#pragma once

#include "opencv.h"
#include "lib.h"
#include "color.h"
#include "paramWX.h"
#include "logger.h"


struct findZottResult {
	std::vector<cv::Rect2i> rects;
	type::MatColor output_image;
};


findZottResult findZott(
		const type::MatColor &original_image,
		type::ProgressF &progress = [](const real &) {}
) {

	findZottResult result;

	auto original_hsl = color::matRGB2HSL(original_image);

	auto red_image = type::MatGray(original_hsl.rows, original_hsl.cols);
	auto white_image = type::MatGray(original_hsl.rows, original_hsl.cols);

	original_hsl.forEach([&](auto &pixel, const int position[]) -> void {
		const auto &color = original_hsl(position[0], position[1]);
		const auto &H = color[type::HSL::Hi];
		const auto &L = color[type::HSL::Li];
		const auto &S = color[type::HSL::Si];

		red_image(position[0], position[1]) = (
													  (H < 20 || H > 360 - 20) &&
													  (0.25 < S) &&
													  (0.25 < L && L < 0.75)
											  ) ? 1 : 0;

		if (L > 0.75) {
			white_image(position[0], position[1]) = 1;
		} else if (L > 0.5 && S < 0.33) {
			white_image(position[0], position[1]) = 1;
		}
	});

	const auto red_opened = calc::opening(red_image);
	const auto white_closed = calc::closing(white_image);

	// std::atomic<int> i = 0;

	// Szukamy kółka
	calc::findGrayFiguresLowRam(red_opened, [&](const type::MatGray &fig_opened) {
		// auto logger = Logger(std::to_string(++i));

		cv::Rect2i zott_rect;
		cv::Point2i zott_center;
		real circle_S, circle_r;

		// Sprawdzymy czy mamy doczynienia z kołem
		{
			circle_S = calc::countArea(fig_opened);
			if (circle_S < 10) return;

			auto m_00 = param::m_pq(fig_opened, 0, 0);
			auto m_10 = param::m_pq(fig_opened, 1, 0);
			auto m_01 = param::m_pq(fig_opened, 0, 1);

			real dash_i = m_10 / m_00;
			real dash_j = m_01 / m_00;
			zott_center = cv::Point2i(dash_j, dash_i);

			auto img_perimeter = calc::filter(fig_opened, edge_filter_matrix);
			auto L = calc::countPerimeter(img_perimeter);

			auto W3 = (L / (2 * cv::sqrt(circle_S * CV_PI))) - 1;
			if (cv::abs(W3) > 0.25) return;

			{// W4
				real calka = calc::sum<type::VecGray>(fig_opened, [&](const auto &pixel, const int position[]) -> real {
					if (pixel[0] < 0.5) return 0;

					real r = cv::pow(dash_i - real(position[0]), 2) + cv::pow(dash_j - real(position[1]), 2);
					// r = cv::pow(cv::sqrt(r, 2), 2);
					return r;
				});

				auto W4 = circle_S / cv::sqrt(2 * CV_PI * calka);
				if (!calc::isInOffset(W4, 1, 0.25)) return;
			}

			//Wyliczenie parametrów koła

			circle_r = cv::sqrt(circle_S / CV_PI);
			auto offset = circle_r * 11;

			auto sub_rect = cv::Rect2d(dash_j - offset, dash_i - offset, 2 * offset, 2 * offset);
			auto img_rec = cv::Rect2d(cv::Point(0, 0), fig_opened.size());

			zott_rect = img_rec & sub_rect;
		}

		const auto img_red = red_image(zott_rect);
		const auto img_red_opened = red_opened(zott_rect);
		const auto img_white = white_image(zott_rect);

		// sprawdza czy jest odpowiednio duzo czerwonego w obrazie
		if (calc::countArea(img_red_opened) / zott_rect.area() < 0.10) return;


		// wyliczamy czy kółko jest pełne
		{
			type::MatGray img_sum = img_white + img_red;
			calc::trimImage(img_sum);
			img_sum = calc::dilation(img_sum);

			std::atomic<int> sum = false;
			real zott_r2 = cv::pow(circle_r * 4, 2);

			cv::Point2i zott_center_here = zott_center - zott_rect.tl();

			img_sum.forEach([&](const auto &pixel, const int position[]) {

				if (cv::pow(position[0] - zott_center_here.y, 2) +
					cv::pow(position[1] - zott_center_here.x, 2) < zott_r2 &&
					pixel[0] > 0.5) {
					sum++;
				}
			});

			if (sum < zott_r2 * CV_PI * 0.95) return;
		}

		auto img_white_closed = white_closed(zott_rect);

		// znajdujemy "długi" biały obiekt
		{
			std::atomic<bool> found = false;
			calc::findGrayFiguresLowRam(img_white_closed, [&](const type::MatGray &fig) {
				auto S = calc::countArea(fig);
				auto perimeter = calc::filter(fig, edge_filter_matrix);
				auto L = calc::countPerimeter(perimeter);

				if (L / S < 1) {
					found = true;
				}
			});
			if (!found) return;
		}


		{ // znajdujemy spore kwałki czerwieni
			const auto area = zott_rect.area();
			cv::Point2i zott_center_here = zott_center - zott_rect.tl();
			real zott_r2 = cv::pow(circle_r * 5, 2);
			std::atomic<int> found_count = 0;

			calc::findGrayFiguresLowRam(img_red_opened, [&](const type::MatGray &fig) {
				auto S = calc::countArea(fig);

				auto m_00 = param::m_pq(fig, 0, 0);
				auto m_10 = param::m_pq(fig, 1, 0);
				auto m_01 = param::m_pq(fig, 0, 1);

				real dash_i = m_10 / m_00;
				real dash_j = m_01 / m_00;

				auto ratio = S / area;

				// jest odpowiedniej wielkości
				if (!(0.03 < ratio && ratio < 0.10)) return;

				// jego środek jest blisko
				if (cv::pow(dash_i - zott_center_here.y, 2) +
					cv::pow(dash_j - zott_center_here.x, 2) > zott_r2)
					return;

				// jeżeli nie dotyka ścianek
				for (int n = 0; n < fig.rows; n++) {
					if (fig(n, 0)[0] > 0.5) return;
					if (fig(n, fig.cols - 1)[0] > 0.5) return;
				}
				for (int n = 0; n < fig.cols; n++) {
					if (fig(0, n)[0] > 0.5) return;
					if (fig(fig.rows - 1, n)[0] > 0.5) return;
				}

				++found_count;
			});

			if (found_count == 0) return;
		}


		// logger.log("zott_center", zott_center).log("circle_S", circle_S).log("circle_r", circle_r);
		// logger.print();
		// lib::showMat(name + "/fig/" + logger.getName(), img_red_opened);
		result.rects.push_back(zott_rect);
	}, progress);

	result.output_image = original_image.clone();

	int thickness = (real(std::min(original_image.size[0], original_image.size[1])) * 0.005)+ 1;
	std::for_each(result.rects.begin(), result.rects.end(), [&](auto &rect) {
		static auto border_color = type::VecColor(1, 0, 1);
		lib::drawRectangleOnImg(result.output_image, rect, border_color, thickness);
	});

	return result;
}
