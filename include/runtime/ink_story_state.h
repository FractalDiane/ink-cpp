#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>

#include "runtime/ink_story_structs.h"

struct InkStoryState {
	enum class ChoiceMixPosition {
		Before,
		In,
		After,
	};

	struct KnotStatus {
		Knot* knot;
		std::size_t index;
	};

	std::vector<KnotStatus> current_knots_stack;
	//std::string_view current_knot_name;
	//std::string_view current_stitch;
	//std::vector<std::size_t> knot_index_stack;

	bool should_end_story = false;

	std::string_view new_knot_target;
	bool new_knot_from_choice = false;

	std::vector<std::string> current_tags;

	std::vector<std::string> current_choices;
	std::vector<struct InkChoiceEntry*> current_choice_structs;
	std::size_t selected_choice = -1;
	ChoiceMixPosition choice_mix_position = ChoiceMixPosition::Before;
	std::unordered_map<Knot*, std::unordered_set<std::size_t>> choices_taken;
	std::size_t total_choices_taken = 0;

	bool in_glue = false;
	bool check_for_glue_divert = false;
	bool in_choice_text = false;
	bool at_choice = false;

	std::unordered_map<std::string, std::size_t> knot_visit_counts;

	class InkObject* get_current_object(std::int64_t index_offset);
	bool has_choice_been_taken(std::size_t index);
	inline std::size_t index_in_knot() const { return current_knots_stack.back().index; }
	inline KnotStatus& current_knot() { return current_knots_stack.back(); }
	inline std::size_t current_knot_size() const { return current_knots_stack.back().knot->objects.size(); }
	KnotStatus& current_nonchoice_knot();
};

struct InkStoryEvalResult {
	std::string result;
	bool should_continue = true;
	std::string target_knot;
};
