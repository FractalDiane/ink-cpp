#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "runtime/ink_story_structs.h"

struct InkStoryState {
	enum class ChoiceMixPosition {
		Before,
		In,
		After,
	};

	Knot* current_knot = nullptr;
	std::string_view current_knot_name;
	std::string_view current_stitch;
	std::size_t index_in_knot = 0;

	std::string_view new_knot_target;
	bool new_knot_from_choice = false;

	std::vector<std::string> current_tags;

	std::vector<std::string> current_choices;
	std::size_t selected_choice = -1;
	ChoiceMixPosition choice_mix_position;
	std::unordered_map<Knot*, std::unordered_set<std::size_t>> choices_taken;
	std::size_t total_choices_taken = 0;

	bool in_glue = false;
	bool in_choice_text = false;
	bool at_choice = false;

	class InkObject* get_current_object(std::size_t index_offset) const;
	bool has_choice_been_taken(std::size_t index) const;
};

struct InkStoryEvalResult {
	std::string result;
	bool should_continue;
};
