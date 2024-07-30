#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <random>
#include <optional>

#include "runtime/ink_story_structs.h"
#include "runtime/ink_story_tracking.h"

#include "types/ink_list.h"

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
		bool reached_newline = false;
		Stitch* next_stitch = nullptr;
	};

	struct ThreadEntry {
		std::string choice_text;
		struct InkChoiceEntry* choice_entry = nullptr;
		std::size_t choice_index = 0;
		Knot* containing_knot = nullptr;
		Stitch* containing_stitch = nullptr;
		std::size_t index_in_knot = 0;
		std::vector<std::pair<std::string, ExpressionParserV2::Variant>> arguments;

		bool applied = false;
	};

	std::mt19937 rng{std::random_device()()};

	std::vector<KnotStatus> current_knots_stack;
	Stitch* current_stitch = nullptr;
	//Stitch* next_stitch = nullptr;

	bool should_end_story = false;

	std::vector<std::string> current_tags;

	struct StoryChoice {
		std::string text;
		bool from_thread;
	};

	std::vector<StoryChoice> current_choices;
	std::vector<struct InkChoiceEntry*> current_choice_structs;
	std::vector<std::size_t> current_choice_indices;
	std::optional<std::size_t> selected_choice = std::nullopt;
	ChoiceMixPosition choice_mix_position = ChoiceMixPosition::Before;
	std::unordered_map<Knot*, std::unordered_map<class InkObject*, std::unordered_set<std::size_t>>> choices_taken;
	std::size_t total_choices_taken = 0;

	std::vector<std::vector<std::pair<std::string, ExpressionParserV2::Variant>>> arguments_stack;
	std::vector<Knot*> function_call_stack;

	std::size_t current_thread_depth = 0;
	std::vector<ThreadEntry> current_thread_entries;
	std::vector<std::vector<std::pair<std::string, ExpressionParserV2::Variant>>> thread_arguments_stack;
	bool should_wrap_up_thread = false;

	InkStoryTracking story_tracking;

	bool in_glue = false;
	bool check_for_glue_divert = false;
	bool in_choice_text = false;
	bool at_choice = false;
	bool just_diverted_to_non_knot = false;
	std::optional<std::size_t> choice_divert_index = std::nullopt;

	ExpressionParserV2::StoryVariableInfo variable_info;

	class InkObject* get_current_object(std::int64_t index_offset);
	bool has_choice_been_taken(class InkObject* choice_object, std::size_t index);
	void add_choice_taken(class InkObject* choice_object, std::size_t index);
	inline std::size_t index_in_knot() const { return current_knots_stack.back().index; }
	inline KnotStatus& current_knot() { return current_knots_stack.back(); }
	KnotStatus& previous_nonfunction_knot(bool offset_by_one = false);
	inline std::size_t current_knot_size() const { return current_knots_stack.back().knot->objects.size(); }
	KnotStatus& current_nonchoice_knot();
	void setup_next_stitch();

	void update_local_knot_variables();

	void apply_thread_choices();
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
	std::optional<ExpressionParserV2::Variant> return_value;

	bool has_any_contents(bool strip);
};
