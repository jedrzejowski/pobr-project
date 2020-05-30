#pragma once

#include "types.h"

static const cv::Mat_<real> edge_filter_matrix = (
		cv::Mat_<real>(3, 3)
				<<
				0, -1, 0,
				-1, 4, -1,
				0, -1, 0
);
