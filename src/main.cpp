#include <iostream>
#include <iomanip>
#include "findZott.h"

int main(int argc, char *argv[]) {

	if (argc != 3) {
		std::cout << "usage: pobr_app input_image output_image";
		return 1;
	}

	auto original_image = lib::loadImage(argv[1]);

	auto result = findZott(original_image, [](const real &precent) {
		std::stringstream ss;
		ss << std::fixed << std::setprecision(2) << precent * 100 << "%";
		std::string mystring = ss.str();
		std::cout << ss.str() << std::endl;
	});

	std::cout << "Found " << result.rects.size() << " Zott logos:" << std::endl;
	for (const auto &rect : result.rects) {
		std::cout << rect << std::endl;
	}

	lib::saveImage(argv[2], result.output_image);

	lib::showMat("result", result.output_image);
	cv::waitKey(-1);

	return 0;
}