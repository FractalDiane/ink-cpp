#include "runtime/ink_story_state.h"

#include "objects/ink_object.h"
#include "objects/ink_object_choice.h"
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

void InkStoryState::update_weave_uuid() {
	Stitch* stitch_current = current_stitch();
	variable_info.current_weave_uuid = stitch_current ? stitch_current->uuid : current_nonchoice_knot().knot->uuid;
}

void InkStoryState::setup_next_stitch() {
	KnotStatus& current = current_nonchoice_knot();
	Stitch* stitch_current = current_stitch();
	std::vector<Stitch>& stitches = current.knot->stitches;
	if (stitch_current) {
		for (auto stitch = stitches.begin(); stitch != stitches.end(); ++stitch) {
			if (stitch_current == &*stitch) {
				if (stitch_current != &stitches.back()) {
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

			entry.applied = true;
		}
	}

	if (current_choice_structs.size() == 1 && current_choice_structs[0]->fallback) {
		current_knots_stack.push_back({&current_choice_structs[0]->result, 0});
		at_choice = false;
	}

	thread_entries_applied = true;
}

bool InkStoryEvalResult::has_any_contents(bool strip) {
	return strip ? !strip_string_edges(result, true, true, true).empty() : !result.empty();
}
