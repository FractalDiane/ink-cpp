#include "runtime/ink_story_state.h"

#include "objects/ink_object.h"

#include "shunting-yard.h"

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

void InkStoryState::increment_visit_count(const std::string& knot) {
	std::size_t current_count = static_cast<std::size_t>(variables[knot].asInt());
	variables[knot] = current_count + 1;

	turns_since_knots[knot] = 0;
}
