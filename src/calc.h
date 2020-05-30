#pragma once

#include <queue>
#include <list>
#include <atomic>
#include "types.h"
#include "const.h"

namespace calc {
	real trimNorm(real number) {
		if (number > 1) return 1;
		if (number < 0)return 0;
		return number;
	}

	real trimHue(real hue) {
		while (hue < 0)
			hue += 360;
		while (hue >= 360)
			hue -= 360;
		return hue;
	}

	bool isPointInSize(const cv::Point &point, const cv::Size &size) {
		return point.x >= 0 && point.y >= 0 &&
			   point.x < size.width && point.y < size.height;
	}

	bool isColorWithInTolerance(
			const type::VecColor &pixel,
			const type::VecColor &color,
			const type::VecColor &tolerance = type::VecColor(0, 0, 0)
	) {
		return std::abs(color[0] - pixel[0]) <= tolerance[0] &&
			   std::abs(color[1] - pixel[1]) <= tolerance[1] &&
			   std::abs(color[2] - pixel[2]) <= tolerance[2];
	}

	bool isInOffset(const real &num, const real &compare, const real &offset = 0) {
		return compare - offset <= num && num <= compare + offset;
	}

	template<typename VecType>
	real sum(
			cv::Mat_<VecType> mat,
			std::function<real(VecType &, const int[])> functor
	) {
		real sum = 0;

		int position[] = {0, 0};

		for (int i = 0; i <= mat.rows; i++) {
			position[0] = i;
			for (int j = 0; j <= mat.cols; j++) {
				position[1] = j;

				sum += functor(mat(i, j), position);
			}
		}

		return sum;
	}

	type::MatGray findColorWithTolerance(
			const type::MatColor &matrix,
			const type::VecColor &color,
			const type::VecColor &tolerance = type::VecColor(0, 0, 0)
	) {
		auto output = type::MatGray(matrix.rows, matrix.cols);

		output.forEach([&](auto &pixel, const int position[]) -> void {
			pixel[0] = isColorWithInTolerance(
					matrix(position[0], position[1]),
					color,
					tolerance
			) ? 1 : 0;
		});

		return output;
	}


	type::MatGray erosion(
			const type::MatGray &matrix,
			int size = 1
	) {
		auto output = matrix.clone();

		auto mat_rect = cv::Rect2i(cv::Point(0, 0), matrix.size());

		output.forEach([&](auto &pixel, const int position[]) -> void {
			auto sub_rect = cv::Rect2i(
					position[1] - size, position[0] - size,
					2 * size + 1, 2 * size + 1
			) & mat_rect;

			auto sub_matrix = matrix(sub_rect);

			for (const auto &val : sub_matrix) {
				pixel = std::min(pixel[0], val[0]);
			}
		});

		return output;
	}

	type::MatGray dilation(
			const type::MatGray &matrix,
			int size = 1
	) {
		auto output = matrix.clone();

		auto mat_rect = cv::Rect2i(cv::Point(0, 0), matrix.size());

		output.forEach([&](auto &pixel, const int position[]) -> void {
			auto sub_rect = cv::Rect2i(
					position[1] - size, position[0] - size,
					2 * size + 1, 2 * size + 1
			) & mat_rect;

			auto sub_matrix = matrix(sub_rect);

			for (const auto &val : sub_matrix) {
				pixel = std::max(pixel[0], val[0]);
			}
		});

		return output;
	}

	type::MatGray opening(const type::MatGray &matrix, int size = 1) {
		return dilation(erosion(matrix, size), size);
	}

	type::MatGray closing(const type::MatGray &matrix, int size = 1) {
		return erosion(dilation(matrix, size), size);
	}

	template<typename MatrixType, typename FilterType>
	cv::Mat_<MatrixType> filter(
			const cv::Mat_<MatrixType> &image,
			const cv::Mat_<FilterType> &matrix
	) {
		CV_Assert(matrix.rows % 2 == 1);
		CV_Assert(matrix.cols % 2 == 1);

		auto size = image.size();

		cv::Mat_<MatrixType> output = cv::Mat(size, image.type());

		auto get_pixel = [&](int x, int y) -> MatrixType {
			if (x < 0) x = 0;
			if (x >= size.width) x = size.width - 1;

			if (y < 0) y = 0;
			if (y >= size.height) y = size.height - 1;

			return image.template at<MatrixType>(cv::Point(x, y));
		};

		int dx = (matrix.cols - 1) / 2;
		int dy = (matrix.rows - 1) / 2;
		auto multiply = [&](const int &cx, const int &cy) {

			MatrixType sum_pixel;

			for (int mx = 0; mx < matrix.cols; ++mx) {
				for (int my = 0; my < matrix.rows; ++my) {

					auto pixel = get_pixel(cx - dx + mx, cy - dy + my);
					auto factor = matrix(cv::Point(mx, my));

					sum_pixel += (pixel * factor);
				}
			}


			output(cv::Point(cx, cy)) = sum_pixel;
		};

		for (int x = 0; x < image.cols; ++x) {
			for (int y = 0; y < image.rows; ++y) {
				multiply(x, y);
			}
		}

		return output;
	}

	type::MatGray findGrayFigure(
			const type::MatGray &image,
			const cv::Point2i &seed
	) {

		const auto size = image.size();
		cv::Mat done = cv::Mat::zeros(size, CV_8U);
		type::MatGray output = type::MatGray::zeros(size);

		struct Step {
			cv::Point point;
			type::VecGray color;
		};

		auto queue = std::queue<Step>();
		auto point = seed;

		auto push = [&](const cv::Point &point, const type::VecGray &pixel) -> void {
			if (!isPointInSize(point, size)) {
				return;
			}

			if (done.at<char>(point) == 1) {
				return;
			}

			done.at<char>(point) = 1;
			queue.push(Step{point, pixel});
		};

		push(point, image(point));

		// tu nie może byc for_each
		do {
			auto step = queue.front();

			auto old_pixel = step.color;
			point = step.point;

			auto new_pixel = image(point);

			if (old_pixel == new_pixel) {
				output(point) = 1;

				push(point + cv::Point(+1, 0), new_pixel);
				push(point + cv::Point(-1, 0), new_pixel);
				push(point + cv::Point(0, +1), new_pixel);
				push(point + cv::Point(0, -1), new_pixel);
			}

			queue.pop();
		} while (!queue.empty());

		return output;
	}

	std::list<type::MatGray> findGrayFigures(
			const type::MatGray &image
	) {
		auto size = image.size();
		std::list<type::MatGray> output;

		for (int x = 0; x < size.width; x++) {
			for (int y = 0; y < size.height; y++) {

				auto point = cv::Point2i(x, y);

				if (image(point)[0] < 0.5) {
					continue;
				}

				// sprawdzamy czy punkt jest już w innej figurze
				for (const auto &prev : output) {
					if (prev(point)[0] == 1) {
						goto endloop;
					}
				}


				output.push_back(findGrayFigure(image, point));

				endloop:
				int qq;
			}
		}

		return output;
	}

	real countPerimeter(const type::MatGray &img) {
		auto size = img.size();
		std::atomic<int> halfs = 0, squares = 0;

		img.forEach([&](const type::VecGray &weksel, const int position[]) -> void {

			if (weksel[0] < 0.5) {
				return;
			}

			auto isPerimeterToo = [&](const int &dx, const int &dy) -> bool {
				// tu jest na odwrót, ale to dobrze,
				auto next = cv::Point(position[1] + dx, position[0] + dy);
				return isPointInSize(next, size) && img(next)[0] >= 0.5;
			};

			if (isPerimeterToo(-1, -1)) ++squares;
			if (isPerimeterToo(+1, -1)) ++squares;
			if (isPerimeterToo(+1, +1)) ++squares;
			if (isPerimeterToo(-1, +1)) ++squares;

			if (isPerimeterToo(0, -1)) ++halfs;
			if (isPerimeterToo(+1, 0)) ++halfs;
			if (isPerimeterToo(-1, 0)) ++halfs;
			if (isPerimeterToo(0, +1)) ++halfs;
		});

		return halfs / 2 + squares * cv::sqrt(2) / 2;
	}

	real countArea(const type::MatGray &img) {
		std::atomic<int> sum = 0;

		img.forEach([&](const type::VecGray &weksel, const int position[]) -> void {
			if (weksel[0] >= 0.5)
				sum += 1;
		});

		return sum;
	}
}