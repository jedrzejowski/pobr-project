#pragma once

#include "opencv.h"
#include "types.h"
#include "assert.h"


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

	type::MatColor loadAsset(const std::string &name) {
		static const std::string prefix = "../assets/";
		auto image = cv::imread(prefix + name);

		return normalize(image);
	}

	void showMat(const std::string title, const cv::Mat &matrix) {

		cv::imshow(title, denormalize(matrix));
	}
}
