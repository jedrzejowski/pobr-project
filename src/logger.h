#pragma once

#include <mutex>
#include <string>
#include <sstream>
#include <utility>
#include <iostream>

std::mutex mutex;

class Logger {
	const std::string name;
	std::stringstream ss;

public:
	Logger(const std::string &name) : name(name) {
		ss << name;
	}

	template<typename T>
	inline Logger &log(const std::string &name, const T &obj) {
		ss << std::endl << "\t" << name << "=" << obj;
		return *this;
	}

	const std::string &getName() const {
		return name;
	}

	void print() const {
		mutex.lock();
		std::cout << ss.str() << std::endl;
		mutex.unlock();
	}
};
