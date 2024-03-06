#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <random>

#include "runtime/ink_story_structs.h"
#include "runtime/ink_story_tracking.h"
#include "shunting-yard.h"

#include "expression_parser/expression_parser.h"

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

	std::mt19937 rng{std::random_device()()};

	std::vector<KnotStatus> current_knots_stack;
	Stitch* current_stitch = nullptr;

	bool should_end_story = false;

	std::vector<std::string> current_tags;

	std::vector<std::string> current_choices;
	std::vector<struct InkChoiceEntry*> current_choice_structs;
	std::vector<std::size_t> current_choice_indices;
	std::size_t selected_choice = SIZE_MAX;
	ChoiceMixPosition choice_mix_position = ChoiceMixPosition::Before;
	std::unordered_map<Knot*, std::unordered_map<class InkObject*, std::unordered_set<std::size_t>>> choices_taken;
	std::size_t total_choices_taken = 0;

	/*std::unordered_map<Knot*, std::size_t> knot_visit_counts;
	std::unordered_map<Stitch*, std::size_t> stitch_visit_counts;
	std::unordered_map<std::string, std::size_t> gather_point_visit_counts;*/
	//std::unordered_map<std::string, std::size_t> turns_since_knots;

	InkStoryTracking story_tracking;

	bool in_glue = false;
	bool check_for_glue_divert = false;
	bool in_choice_text = false;
	bool at_choice = false;
	bool just_diverted_to_non_knot = false;

	ExpressionParser::VariableMap variables;
	ExpressionParser::FunctionMap functions;

	class InkObject* get_current_object(std::int64_t index_offset);
	bool has_choice_been_taken(class InkObject* choice_object, std::size_t index);
	void add_choice_taken(class InkObject* choice_object, std::size_t index);
	inline std::size_t index_in_knot() const { return current_knots_stack.back().index; }
	inline KnotStatus& current_knot() { return current_knots_stack.back(); }
	inline std::size_t current_knot_size() const { return current_knots_stack.back().knot->objects.size(); }
	KnotStatus& current_nonchoice_knot();

	ExpressionParser::VariableMap get_variables_with_locals();
};

struct InkStoryEvalResult {
	std::string result;
	bool should_continue = true;
	bool reached_newline = false;
	std::string target_knot;

	bool has_any_contents(bool strip);
};
