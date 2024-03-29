#include "objects/ink_object_choice.h"

#include "ink_utils.h"

#include "objects/ink_object_choicetextmix.h"

#include "expression_parser/expression_parser.h"

ByteVec Serializer<InkChoiceEntry>::operator()(const InkChoiceEntry& entry) {
	Serializer<std::uint8_t> s8;
	Serializer<std::uint16_t> s16;
	Serializer<Knot> sknot;
	Serializer<GatherPoint> sgatherpoint;
	VectorSerializer<InkObject*> sobjects;
	VectorSerializer<ExpressionParser::Token*> stokens;

	ByteVec result = sobjects(entry.text);
	ByteVec result2 = sknot(entry.result);
	ByteVec result3 = s8(static_cast<std::uint8_t>(entry.sticky));
	ByteVec result4 = s8(static_cast<std::uint8_t>(entry.fallback));
	ByteVec result5 = s8(static_cast<std::uint8_t>(entry.immediately_continue_to_result));
	ByteVec result6 = sgatherpoint(entry.label);

	result.insert(result.end(), result2.begin(), result2.end());
	result.insert(result.end(), result3.begin(), result3.end());
	result.insert(result.end(), result4.begin(), result4.end());
	result.insert(result.end(), result5.begin(), result5.end());
	result.insert(result.end(), result6.begin(), result6.end());

	ByteVec result7 = s16(static_cast<std::uint16_t>(entry.conditions.size()));
	result.insert(result.end(), result7.begin(), result7.end());

	for (const auto& vec : entry.conditions) {
		ByteVec result_tokens = stokens(vec);
		result.insert(result.end(), result_tokens.begin(), result_tokens.end());
	}

	return result;
}

InkChoiceEntry Deserializer<InkChoiceEntry>::operator()(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::uint8_t> ds8;
	Deserializer<std::uint16_t> ds16;
	Deserializer<Knot> dsknot;
	Deserializer<GatherPoint> dsgatherpoint;
	VectorDeserializer<InkObject*> dsobjects;
	VectorDeserializer<ExpressionParser::Token*> dstokens;

	InkChoiceEntry result;
	result.text = dsobjects(bytes, index);
	result.result = dsknot(bytes, index);
	result.sticky = static_cast<bool>(ds8(bytes, index));
	result.fallback = static_cast<bool>(ds8(bytes, index));
	result.immediately_continue_to_result = static_cast<bool>(ds8(bytes, index));
	result.label = dsgatherpoint(bytes, index);

	std::uint16_t conditions_size = ds16(bytes, index);
	for (std::uint16_t i = 0; i < conditions_size; ++i) {
		result.conditions.push_back(dstokens(bytes, index));
	}

	return result;
}

ByteVec InkObjectChoice::to_bytes() const {
	VectorSerializer<InkChoiceEntry> sentries;
	return sentries(choices);
}

InkObject* InkObjectChoice::populate_from_bytes(const ByteVec& bytes, std::size_t& index) {
	VectorDeserializer<InkChoiceEntry> dsentries;
	choices = dsentries(bytes, index);
	return this;
}

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
