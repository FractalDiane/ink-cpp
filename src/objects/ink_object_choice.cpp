#include "objects/ink_object_choice.h"

#include "ink_utils.h"

#include "shunting-yard.h"
#include "builtin-features.inc"

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
	for (std::size_t i = 0; i < choices.size(); ++i) {
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
	if (story_state.selected_choice == SIZE_MAX || story_state.current_choices.empty()) {
		story_state.in_choice_text = true;
		story_state.current_choices.clear();
		story_state.current_choice_structs.clear();
		story_state.current_choice_indices.clear();
		story_state.selected_choice = SIZE_MAX;

		std::size_t fallback_index = SIZE_MAX;
		for (std::size_t i = 0; i < choices.size(); ++i) {
			InkChoiceEntry& this_choice = choices[i];
			if (this_choice.sticky || !story_state.has_choice_been_taken(this, i)) {
				if (!this_choice.fallback) {
					bool include_choice = true;
					const std::vector<std::string>& conditions = this_choice.conditions;
					if (!conditions.empty()) {
						for (const std::string& condition : conditions) {
							cparse::packToken result = cparse::calculator::calculate(deinkify_expression(condition).c_str(), story_state.variables);
							if (!result.asBool()) {
								include_choice = false;
								break;
							}
						}
					}

					if (include_choice) {
						story_state.choice_mix_position = InkStoryState::ChoiceMixPosition::Before;
					
						InkStoryEvalResult choice_eval_result;
						choice_eval_result.result.reserve(50);
						for (InkObject* object : this_choice.text) {
							object->execute(story_state, choice_eval_result);
						}

						story_state.current_choices.push_back(strip_string_edges(choice_eval_result.result, true, true, true));
						story_state.current_choice_structs.push_back(&this_choice);
						story_state.current_choice_indices.push_back(i);
					}
				} else {
					fallback_index = i;
				}
			}
		}

		story_state.in_choice_text = false;
		story_state.choice_mix_position = InkStoryState::ChoiceMixPosition::Before;

		//story_state.choice_gather_stack.push_back(has_gather);

		if (!story_state.current_choices.empty()) {
			eval_result.should_continue = false;
			story_state.at_choice = true;
		} else if (fallback_index != SIZE_MAX) {
			story_state.current_knots_stack.push_back({&(choices[fallback_index].result), 0});
			story_state.at_choice = false;
		}
	} else {
		story_state.add_choice_taken(this, static_cast<std::size_t>(story_state.current_choice_indices[story_state.selected_choice]));
		InkChoiceEntry* selected_choice_struct = story_state.current_choice_structs[story_state.selected_choice];

		story_state.choice_mix_position = InkStoryState::ChoiceMixPosition::Before;
		InkStoryEvalResult choice_eval_result;
		choice_eval_result.result.reserve(50);
		for (InkObject* object : selected_choice_struct->text) {
			object->execute(story_state, choice_eval_result);
			if (object->get_id() == ObjectId::LineBreak) {
				choice_eval_result.should_continue = story_state.in_glue;
			}
		}

		eval_result.result = choice_eval_result.result;
		eval_result.should_continue = story_state.in_glue
									  || strip_string_edges(eval_result.result, true, true, true).empty()
									  || selected_choice_struct->immediately_continue_to_result
									  || (!selected_choice_struct->result.objects.empty() && selected_choice_struct->result.objects[0]->get_id() == ObjectId::Choice);

		story_state.current_knots_stack.push_back({&(selected_choice_struct->result), 0});
		story_state.choice_mix_position = InkStoryState::ChoiceMixPosition::Before;

		for (auto& entry : story_state.turns_since_knots) {
			++entry.second;
		}

		story_state.current_choices.clear();
		story_state.selected_choice = SIZE_MAX;
		++story_state.total_choices_taken;
		story_state.at_choice = false;
	}
}

bool InkObjectChoice::will_choice_take_fallback(InkStoryState& story_state) {
	std::size_t fallback_index = SIZE_MAX;
	for (std::size_t i = 0; i < choices.size(); ++i) {
		InkChoiceEntry& this_choice = choices[i];
		if (this_choice.sticky || !story_state.has_choice_been_taken(this, i)) {
			if (!this_choice.fallback) {
				return false;
			} else {
				fallback_index = i;
			}
		}
	}

	return fallback_index != SIZE_MAX;
}
