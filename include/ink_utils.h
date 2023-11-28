#pragma once

#include <string>
#include <numeric>
#include <vector>

std::string strip_string_edges(const std::string& string, bool left = true, bool right = true) noexcept {
	std::string result;
	result.reserve(string.length());

	bool stripped_left = false;
	size_t first_left_index = 0;
	size_t last_right_index = 0;
	for (size_t i = 0; i < string.length(); ++i) {
		char chr = string[i];
		bool whitespace = chr <= 32;
		if (!left || stripped_left) {
			result += chr;
		} else if (!whitespace) {
			result += chr;
			stripped_left = true;
		}

		if (!whitespace) {
			last_right_index = i;
		}
	}

	if (right) {
		result.shrink_to_fit();
	}
	
	return result;
}

std::string join_string_vector(const std::vector<std::string>& vector, std::string&& delimiter) noexcept {
	if (!vector.empty()) {
		return std::accumulate(
			std::next(vector.begin()), vector.end(), vector[0],
			[&](const std::string& a, const std::string& b) { return a + delimiter + b; }
		);
	} else {
		return std::string();
	}
}