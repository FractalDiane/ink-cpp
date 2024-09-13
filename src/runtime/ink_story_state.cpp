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

KnotStatus& InkStoryState::previous_nonfunction_knot(bool offset_by_one) {
	if (current_knots_stack.size() >= 2) {
		for (auto it = current_knots_stack.rbegin() + 1; it != current_knots_stack.rend(); ++it) {
			if (!it->knot->is_function && !it->knot->name.empty()) {
				return !offset_by_one ? *it : *(it - 1);
			}
		}
	}

	return current_knots_stack.front();
}

KnotStatus& InkStoryState::current_nonchoice_knot() {
	for (auto it = current_knots_stack.rbegin(); it != current_knots_stack.rend(); ++it) {
		if (!it->knot->name.empty()) {
			return *it;
		}
	}

	return current_knots_stack.front();
}

void InkStoryState::setup_next_stitch() {
	KnotStatus& current = current_knot();
	std::vector<Stitch>& stitches = current.knot->stitches;
	if (current_stitch) {
		for (auto stitch = stitches.begin(); stitch != stitches.end(); ++stitch) {
			if (current_stitch == &*stitch) {
				if (current_stitch != &stitches.back()) {
					current.next_stitch = &*(stitch + 1);
				} else {
					current.next_stitch = nullptr;
				}

				return;
			}
		}
	} else if (!stitches.empty()) {
		current.next_stitch = &stitches[0];
	} else {
		current.next_stitch = nullptr;
	}
}

void InkStoryState::apply_thread_choices() {
	for (ThreadEntry& entry : current_thread_entries) {
		if (!entry.applied) {
			current_choices.emplace_back(entry.choice_text, true);
			current_choice_structs.emplace_back(entry.choice_entry);
			current_choice_indices.push_back(entry.choice_index);

			entry.applied = true;
		}
	}
}

bool InkStoryEvalResult::has_any_contents(bool strip) {
	return strip ? !strip_string_edges(result, true, true, true).empty() : !result.empty();
}
