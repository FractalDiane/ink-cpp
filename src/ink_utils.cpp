#include "ink_utils.h"

#include <numeric>
#include <algorithm>

std::string strip_string_edges(const std::string& string, bool left, bool right, bool include_spaces) noexcept {
	std::string result;
	result.reserve(string.length());

	bool stripped_left = false;
	std::size_t first_left_index = 0;
	std::size_t last_right_index = 0;
	for (std::size_t i = 0; i < string.length(); ++i) {
		char chr = string[i];
		bool whitespace = chr < 32 + include_spaces;
		if (!left || stripped_left) {
			result += chr;
		} else if (!whitespace) {
			result += chr;
			first_left_index = i;
			stripped_left = true;
		}

		if (!whitespace) {
			last_right_index = i - first_left_index;
		}
	}

	if (right && !result.empty()) {
		result.resize(last_right_index + 1);
	}
	
	return result;
}

std::string remove_duplicate_spaces(const std::string& string) noexcept {
	std::string result = string;
	auto end = std::unique(result.begin(), result.end(), [](char lhs, char rhs) { return lhs == rhs && lhs == ' '; });
	result.erase(end, result.end());
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

std::int64_t randi_range(std::int64_t from, std::int64_t to, std::mt19937& generator) noexcept {
	std::uniform_int_distribution<std::int64_t> distribution{from, to};
	return distribution(generator);
}

std::string deinkify_expression(const std::string& expression) noexcept {
	std::string result;
	result.reserve(expression.size());

	bool looking_for_knot = false;
	bool found_knot_word = false;
	for (std::size_t i = 0; i < expression.size(); ++i) {
		if (expression.substr(i, 2) == "->") {
			looking_for_knot = true;
			++i;
		} else if (looking_for_knot) {
			if (!found_knot_word && expression[i] > 32) {
				result.push_back('"');
				result.push_back(expression[i]);
				found_knot_word = true;
				looking_for_knot = false;
			}
		} else {
			if (!found_knot_word || (expression[i] > 32 && expression[i] != ')')) {
				result.push_back(expression[i]);
			} else {
				result.push_back('"');
				result.push_back(')');
				found_knot_word = false;
			}
		}
	}

	return result;
}