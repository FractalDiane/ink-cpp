#pragma once

#include "objects/ink_object.h"

#include <string>
#include <vector>
#include <unordered_map>

struct InkStoryData {
	std::unordered_map<std::string, std::vector<InkObject*>> knots;
	std::vector<std::string> knot_order;
};
