#pragma once

#include <string>
#include <vector>
#include <initializer_list>

class InkList {
	std::vector<std::string> values;
	std::size_t current_value;

public:
	InkList(std::initializer_list<std::string> values, std::size_t initial_value) : values(values), current_value(initial_value) {}

	void set_value_index(std::size_t index) { current_value = index; }
	void set_value(const std::string& value) {
		std::size_t index = 0;
		for (const std::string& entry : values) {
			if (entry == value) {
				current_value = index;
				return;
			}

			++index;
		}
	}
};
