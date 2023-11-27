#pragma once

#include "objects/ink_object.h"
#include "runtime/ink_story_data.h"

#include <string>
#include <unordered_map>
#include <vector>

class InkStory {
private:
	InkStoryData* story_data;

public:
	InkStory(InkStoryData* data) : story_data{data} {}
	~InkStory() { delete story_data; }

	void print_info() const;
};
