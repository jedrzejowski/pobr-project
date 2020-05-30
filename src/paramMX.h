#pragma once

#include <iostream>
#include "types.h"

namespace param {
	real m_pq(const type::MatGray &matrix, real p, real q) {
		real sum = 0;

		for (int i = 0; i < matrix.rows; ++i) {
			for (int j = 0; j < matrix.cols; ++j) {

				sum += cv::pow(i + 1, p) * cv::pow(j + 1, q) * matrix(i, j)[0];
			}
		}

		return sum;
	}

	real M_pq(const type::MatGray &matrix, real p, real q) {
		real sum = 0;

		auto m_00 = m_pq(matrix, 0, 0);
		auto m_10 = m_pq(matrix, 1, 0);
		auto m_01 = m_pq(matrix, 0, 1);

		real dash_i = m_10 / m_00;
		real dash_j = m_01 / m_00;

		for (int i = 0; i < matrix.rows; ++i) {
			for (int j = 0; j < matrix.cols; ++j) {

				sum += cv::pow(i + 1 - dash_i, p) * cv::pow(j + 1 - dash_j, q) * matrix(i, j)[0];
			}
		}

		return sum;
	}

	real M_11(const type::MatGray &image) {
		return m_pq(image, 1, 1) - (m_pq(image, 1, 0) * m_pq(image, 0, 1)) / m_pq(image, 0, 0);
	}

	real M_20(const type::MatGray &image) {
		return m_pq(image, 2, 0) - (cv::pow(m_pq(image, 1, 0), 2) / m_pq(image, 0, 0));
	}

	real M_02(const type::MatGray &image) {
		return m_pq(image, 0, 2) - (cv::pow(m_pq(image, 0, 1), 2) / m_pq(image, 0, 0));
	}

	real M1(const type::MatGray &image) {
		return (M_20(image) + M_02(image)) / cv::pow(m_pq(image, 0, 0), 2);
	}

	real M7(const type::MatGray &image) {
		return (M_20(image) * M_02(image) - cv::pow(M_11(image), 2)) / cv::pow(m_pq(image, 0, 0), 4);
	}
}