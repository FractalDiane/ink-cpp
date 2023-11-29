#pragma once

#include "objects/ink_object.h"
#include "serialization.h"
#include "runtime/ink_story_structs.h"

#include <string>
#include <vector>
#include <unordered_map>

class InkStoryData {
private:
	std::unordered_map<std::string, Knot> knots;
	std::vector<std::string> knot_order;

	friend class InkStory;

public:
	InkStoryData(const std::vector<Knot>& story_knots);
	~InkStoryData();

	std::vector<std::uint8_t> get_serialized_bytes() const;

	void print_info() const;
};
