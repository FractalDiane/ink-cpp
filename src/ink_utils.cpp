#include "ink_utils.h"

#include <numeric>
#include <algorithm>
#include <regex>
#include <unordered_map>

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
	static std::regex spaces_regex{R"(\s{2,})"};
	return std::regex_replace(string, spaces_regex, " ");
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

std::vector<std::string> split_string(const std::string& string, char delimiter, bool ignore_delim_spaces, bool paren_arguments) noexcept {
	if (!string.empty()) {
		std::vector<std::string> result;
		std::string current_string;

		std::size_t paren_level = 0;
		current_string.reserve(50);
		for (auto chr = string.begin(); chr != string.end(); ++chr) {
			if (paren_arguments) {
				if (*chr == '(') {
					++paren_level;
				} else if (*chr == ')') {
					--paren_level;
				}
			}

			if (*chr != delimiter || (paren_arguments && paren_level > 0)) {
				current_string.push_back(*chr);
			} else {
				result.push_back(current_string);
				current_string.clear();
				if (ignore_delim_spaces) {
					do {
						++chr;
					} while (*chr <= 32);

					--chr;
				}
			}
		}

		if (!current_string.empty()) {
			result.push_back(current_string);
		}

		return result;
	} else {
		return {};
	}
}

std::int64_t randi_range(std::int64_t from, std::int64_t to, std::mt19937& generator) noexcept {
	std::uniform_int_distribution<std::int64_t> distribution{from, to};
	return distribution(generator);
}

std::string deinkify_expression(const std::string& expression) noexcept {
	std::string result = expression;
	//result.reserve(expression.size());

	/*bool looking_for_knot = false;
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
	}*/

	// HACK: MAKE THIS WAY LESS BAD
	static const std::unordered_map<std::string, std::string> replacements = {
		// not
		{R"(\bnot\b)", "!"},
		// or
		{R"(\bor\b)", "||"},
		// and
		{R"(\band\b)", "&&"},
		// -> knot
		{R"(->\s*([\w.]+))", R"("$1")"},
		// var++
		{R"(\b([\w.]+)\b\+\+)", "$1 = $1 + 1"},
		// ++var
		{R"(\+\+\b([\w.]+)\b)", "$1 = $1 + 1"},
		// var--
		{R"(\b([\w.]+)\b--)", "$1 = $1 - 1"},
		// --var
		{R"(--\b([\w.]+)\b)", "$1 = $1 - 1"},
		// temp
	};

	for (const auto& entry : replacements) {
		result = std::regex_replace(result, std::regex(entry.first), entry.second);
	}

	return result;
}
