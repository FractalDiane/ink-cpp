#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <random>

#include "runtime/ink_story_structs.h"
#include "runtime/ink_story_tracking.h"

#include "expression_parser/expression_parser.h"

struct InkStoryState {
	enum class ChoiceMixPosition {
		Before,
		In,
		After,
	};

	struct KnotStatus {
		Knot* knot;
		std::size_t index = 0;
		bool returning_from_function = false;
		Uuid current_function_prep_expression = UINT32_MAX;
		bool any_new_content = false;
	};

	struct ThreadEntry {
		InkObject* choice = nullptr;
		Knot* knot = nullptr;
		std::size_t index = 0;
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

	std::vector<std::vector<std::pair<std::string, ExpressionParser::Variant>>> arguments_stack;
	std::vector<Knot*> function_call_stack;

	std::vector<ThreadEntry> current_thread_entries;

	InkStoryTracking story_tracking;

	bool in_glue = false;
	bool check_for_glue_divert = false;
	bool in_choice_text = false;
	bool at_choice = false;
	bool just_diverted_to_non_knot = false;

	ExpressionParser::VariableMap variables;
	ExpressionParser::VariableMap constants;
	ExpressionParser::FunctionMap functions;
	//std::unordered_map<Uuid, ExpressionParser::VariableMap> local_variables;
	std::unordered_map<Uuid, std::unordered_map<std::string, std::string>> variable_redirects;

	class InkObject* get_current_object(std::int64_t index_offset);
	bool has_choice_been_taken(class InkObject* choice_object, std::size_t index);
	void add_choice_taken(class InkObject* choice_object, std::size_t index);
	inline std::size_t index_in_knot() const { return current_knots_stack.back().index; }
	inline KnotStatus& current_knot() { return current_knots_stack.back(); }
	KnotStatus& previous_nonfunction_knot();
	inline std::size_t current_knot_size() const { return current_knots_stack.back().knot->objects.size(); }
	KnotStatus& current_nonchoice_knot();

	ExpressionParser::VariableMap get_story_constants();
};

struct InkStoryEvalResult {
	std::string result;
	bool should_continue = true;
	bool reached_newline = false;

	std::string target_knot;
	DivertType divert_type = DivertType::ToKnot;
	bool imminent_function_prep = false;
	Uuid function_prep_expression = UINT32_MAX;
	
	std::size_t argument_count = 0;
	bool reached_function_return = false;
	std::optional<ExpressionParser::Variant> return_value;

	bool has_any_contents(bool strip);
};
