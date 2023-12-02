#include "objects/ink_object_choice.h"

#include "ink_utils.h"

/*std::vector<std::uint8_t> InkObjectChoice::to_bytes() const {
	return {}
}

ObjectId InkObjectChoice::get_id() const {
	return ObjectId::Choice;
}*/

InkObjectChoice::~InkObjectChoice() {
	for (InkChoiceEntry& choice : choices) {
		for (InkObject* object : choice.text) {
			delete object;
		}

		for (InkObject* object : choice.result.objects) {
			delete object;
		}
	}
}

std::string InkObjectChoice::to_string() const {
	std::string result = "Choice (\n";
	for (int i = 0; i < choices.size(); ++i) {
		const InkChoiceEntry& choice = choices[i];
		for (InkObject* object : choice.text) {
			result += object->to_string();
		}

		result += "=>";

		for (InkObject* object : choice.result.objects) {
			result += object->to_string();
		}

		if (i < choices.size() - 1) {
			result += "\n";
		}
	}

	result += "\n)";
	return result;
}

void InkObjectChoice::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	if (story_state.selected_choice == -1 || story_state.current_choices.empty()) {
		story_state.in_choice_text = true;
		story_state.current_choices.clear();
		story_state.selected_choice = -1;

		std::size_t fallback_index = -1;
		for (std::size_t i = 0; i < choices.size(); ++i) {
			InkChoiceEntry& this_choice = choices[i];
			if (this_choice.sticky || !story_state.has_choice_been_taken(i)) {
				if (!this_choice.fallback) {
					// TODO: choice conditions

					story_state.choice_mix_position = InkStoryState::ChoiceMixPosition::Before;
					
					InkStoryEvalResult choice_eval_result;
					choice_eval_result.result.reserve(50);
					for (InkObject* object : this_choice.text) {
						object->execute(story_state, choice_eval_result);
					}

					story_state.current_choices.push_back(strip_string_edges(choice_eval_result.result, true, true, true));
					story_state.current_choice_structs.push_back(&this_choice);
				} else {
					fallback_index = i;
				}
			}
		}

		story_state.in_choice_text = false;

		if (!story_state.current_choices.empty()) {
			eval_result.should_continue = false;
			story_state.at_choice = true;
		} else {
			story_state.at_choice = false;
		}
	} else {
		story_state.choices_taken[story_state.current_knot().knot].insert(story_state.selected_choice);
		InkChoiceEntry* selected_choice_struct = story_state.current_choice_structs[story_state.selected_choice];

		story_state.choice_mix_position = InkStoryState::ChoiceMixPosition::Before;
		InkStoryEvalResult choice_eval_result;
		choice_eval_result.result.reserve(50);
		for (InkObject* object : selected_choice_struct->text) {
			object->execute(story_state, choice_eval_result);
			if (object->get_id() == ObjectId::LineBreak) {
				choice_eval_result.should_continue = false;
			}
		}

		eval_result.result = choice_eval_result.result;
		eval_result.should_continue = eval_result.result.empty();

		//story_state.current_knot = &(selected_choice_struct->result);
		story_state.current_knots_stack.push_back({&(selected_choice_struct->result), 0});
		story_state.choice_mix_position = InkStoryState::ChoiceMixPosition::Before;
		story_state.selected_choice = -1;
		++story_state.total_choices_taken;
		story_state.at_choice = false;
	}
}
