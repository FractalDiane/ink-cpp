#pragma once

#include "objects/ink_object.h"

#include <string>
#include <unordered_map>
#include <vector>

class InkStory {
private:
	std::unordered_map<std::string, std::vector<InkObject*>> knots;
	std::vector<std::string> knot_order;

public:
	InkStory() {}
	~InkStory();
};