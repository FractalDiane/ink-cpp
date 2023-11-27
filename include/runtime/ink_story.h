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
	explicit InkStory(InkStoryData* data) : story_data{data} {}
	explicit InkStory(const std::string& inkb_file);
	~InkStory() { delete story_data; }

	InkStoryData* get_story_data() const { return story_data; }
	void print_info() const;
};
