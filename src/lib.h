#pragma once

#include "opencv.h"
#include "types.h"

namespace lib {


	cv::Mat normalize(const cv::Mat_<cv::Vec3b> &matrix) {
		static const cv::Mat_<cv::Vec3b> type_mat;
		CV_Assert(type_mat.type() == matrix.type());

		auto output = type::MatColor(matrix.rows, matrix.cols);

		matrix.forEach([&](const auto &pixel, const int position[]) -> void {
			output(position[0], position[1]) = type::VecColor(pixel) / 255;
		});

		return output;
	}

	cv::Mat denormalize(const cv::Mat &matrix) {
		auto output = cv::Mat_<cv::Vec3b>(matrix.rows, matrix.cols);

		if (matrix.type() == type::MatColor().type()) {

			matrix.forEach<type::VecColor>([&](const auto &pixel, const int position[]) -> void {
				output(position[0], position[1]) = cv::Vec3b(pixel * 255);
			});

			return output;
		}

		if (matrix.type() == type::MatGray().type()) {

			matrix.forEach<type::VecGray>([&](const auto &pixel, const int position[]) -> void {
				auto val = (pixel * 255)[0];
				output(position[0], position[1]) = cv::Vec3b(val, val, val);
			});

			return output;
		}

		return output;
	}

	type::MatColor loadImage(const std::string &name) {
		return normalize(cv::imread(name));
	}

	void saveImage(const std::string &name, const type::MatColor &image) {
		cv::imwrite(name, denormalize(image));
	}

	void showMat(const std::string &title, const cv::Mat &matrix) {

		cv::imshow(title, denormalize(matrix));
	}

	void drawRectangleOnImg(
			type::MatColor &img,
			const cv::Rect2i &rect,
			type::VecColor &color,
			int thickness = 1
	) {
		auto area_rect = cv::Rect2i(cv::Point2i(0, 0), img.size());

		img(cv::Rect2i(
				rect.tl() + cv::Point2i(-thickness, -thickness),
				cv::Size2i(rect.width + thickness, thickness)
		) & area_rect) = color;

		img(cv::Rect2i(
				rect.tl() + cv::Point2i(-thickness, 0),
				cv::Size2i(thickness, rect.height + thickness)
		) & area_rect) = color;

		img(cv::Rect2i(
				rect.tl() + cv::Point2i(rect.width, -thickness),
				cv::Size2i(thickness, rect.height + thickness)
		) & area_rect) = color;

		img(cv::Rect2i(
				rect.tl() + cv::Point2i(0, rect.height),
				cv::Size2i(rect.width + thickness, thickness)
		) & area_rect) = color;
	}
}
