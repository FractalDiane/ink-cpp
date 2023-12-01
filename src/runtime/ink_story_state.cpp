#include "runtime/ink_story_state.h"

#include "objects/ink_object.h"

InkObject* InkStoryState::get_current_object(std::int64_t index_offset) const {
	std::int64_t index = index_in_knot + index_offset;
	if (current_knot && index >= 0 && index < current_knot->objects.size()) {
		return current_knot->objects[index];
	}

	return nullptr;
}
bool InkStoryState::has_choice_been_taken(std::size_t index) const {
	if (auto knot_choices_taken = choices_taken.find(current_knot); knot_choices_taken != choices_taken.end()) {
		return knot_choices_taken->second.contains(index);
	}

	return false;
}
