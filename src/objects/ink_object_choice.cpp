#include "objects/ink_object_choice.h"

#include "ink_utils.h"

#include "objects/ink_object_choicetextmix.h"

#include "expression_parser/expression_parser.h"

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
		
		for (std::vector<ExpressionParser::Token*>& condition : choice.conditions) {
			for (ExpressionParser::Token* token : condition) {
				delete token;
			}
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
					const std::vector<std::vector<ExpressionParser::Token*>>& conditions = this_choice.conditions;
					if (!conditions.empty()) {
						for (const std::vector<ExpressionParser::Token*>& condition : conditions) {
							ExpressionParser::VariableMap story_constants = story_state.get_story_constants();
							ExpressionParser::Variant result = ExpressionParser::execute_expression_tokens(condition, story_state.variables, story_constants, story_state.variable_redirects, story_state.functions).value();
							if (!ExpressionParser::as_bool(result)) {
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
							if (object->get_id() == ObjectId::ChoiceTextMix && static_cast<InkObjectChoiceTextMix*>(object)->is_end()) {
								break;
							}
							
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

		if (!story_state.current_choices.empty()) {
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
		}

		eval_result.result = choice_eval_result.result;
		eval_result.reached_newline = !selected_choice_struct->immediately_continue_to_result && eval_result.has_any_contents(true);

		story_state.current_knots_stack.push_back({&(selected_choice_struct->result), 0});
		story_state.choice_mix_position = InkStoryState::ChoiceMixPosition::Before;

		if (!selected_choice_struct->label.name.empty()) {
			story_state.story_tracking.increment_visit_count(story_state.current_nonchoice_knot().knot, story_state.current_stitch, &selected_choice_struct->label);
		}

		story_state.story_tracking.increment_turns_since();

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
