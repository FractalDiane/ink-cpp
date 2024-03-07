#include "runtime/ink_story_state.h"

#include "objects/ink_object.h"
#include "ink_utils.h"

#include <format>

InkObject* InkStoryState::get_current_object(std::int64_t index_offset) {
	std::int64_t index = index_in_knot() + index_offset;
	if (index >= 0 && index < static_cast<std::int64_t>(current_knot_size())) {
		return current_knot().knot->objects[static_cast<std::size_t>(index)];
	}

	return nullptr;
}

bool InkStoryState::has_choice_been_taken(InkObject* choice_object, std::size_t index) {
	if (auto knot_choices_taken = choices_taken.find(current_knot().knot); knot_choices_taken != choices_taken.end()) {
		if (auto choice_indices_taken = knot_choices_taken->second.find(choice_object); choice_indices_taken != knot_choices_taken->second.end()) {
			return choice_indices_taken->second.contains(index);
		}
	}

	return false;
}

void InkStoryState::add_choice_taken(InkObject* choice_object, std::size_t index) {
	auto& knot_choices_taken = choices_taken[current_knot().knot];
	auto& choice_indices_taken = knot_choices_taken[choice_object];
	choice_indices_taken.insert(index);
}

InkStoryState::KnotStatus& InkStoryState::current_nonchoice_knot() {
	for (auto it = current_knots_stack.rbegin(); it != current_knots_stack.rend(); ++it) {
		if (!it->knot->name.empty()) {
			return *it;
		}
	}

	return current_knots_stack.front();
}

ExpressionParser::VariableMap InkStoryState::get_variables_with_locals() {
	ExpressionParser::VariableMap result = variables;

	/*Knot* knot = current_knot().knot;
	for (const Stitch& stitch : knot->stitches) {
		std::string stitch_name_full = std::format("{}.{}", knot->name, stitch.name);
		result[stitch.name] = result[stitch_name_full];
		if (stitch.name == current_stitch) {
			for (const GatherPoint& gather_point : stitch.gather_points) {
				std::string gather_point_full = std::format("{}.{}.{}", knot->name, stitch.name, gather_point.name);
				std::string gather_point_short = std::format("{}.{}", stitch.name, gather_point.name);
				result[gather_point_short] = result[gather_point_full];
			}
		}
	}

	for (const GatherPoint& gather_point : knot->gather_points) {
		std::string gather_point_full = std::format("{}.{}", knot->name, gather_point.name);
		result[gather_point.name] = result[gather_point_full];
	}*/

	return result;
}

bool InkStoryEvalResult::has_any_contents(bool strip) {
	return strip ? !strip_string_edges(result, true, true, true).empty() : !result.empty();
}
