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
	VectorSerializer<ExpressionParserV2::Token> stokens;

	ByteVec result = sobjects(entry.text);
	ByteVec result2 = s8(static_cast<std::uint8_t>(entry.index));
	ByteVec result3 = sknot(entry.result);
	ByteVec result4 = s8(static_cast<std::uint8_t>(entry.sticky));
	ByteVec result5 = s8(static_cast<std::uint8_t>(entry.fallback));
	ByteVec result6 = s8(static_cast<std::uint8_t>(entry.immediately_continue_to_result));
	ByteVec result7 = sgatherpoint(entry.label);

	result.insert(result.end(), result2.begin(), result2.end());
	result.insert(result.end(), result3.begin(), result3.end());
	result.insert(result.end(), result4.begin(), result4.end());
	result.insert(result.end(), result5.begin(), result5.end());
	result.insert(result.end(), result6.begin(), result6.end());
	result.insert(result.end(), result7.begin(), result7.end());

	ByteVec result8 = s16(static_cast<std::uint16_t>(entry.conditions.size()));
	result.insert(result.end(), result8.begin(), result8.end());

	for (const auto& vec : entry.conditions) {
		ByteVec result_tokens = stokens(vec.tokens);
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
	VectorDeserializer<ExpressionParserV2::Token> dstokens;

	InkChoiceEntry result;
	result.text = dsobjects(bytes, index);
	result.index = static_cast<std::size_t>(ds8(bytes, index));
	result.result = dsknot(bytes, index);
	result.sticky = static_cast<bool>(ds8(bytes, index));
	result.fallback = static_cast<bool>(ds8(bytes, index));
	result.immediately_continue_to_result = static_cast<bool>(ds8(bytes, index));
	result.label = dsgatherpoint(bytes, index);

	std::uint16_t conditions_size = ds16(bytes, index);
	for (std::uint16_t i = 0; i < conditions_size; ++i) {
		result.conditions.push_back(ExpressionParserV2::ShuntedExpression(dstokens(bytes, index)));
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

InkObject::ExpressionsVec InkObjectChoice::get_all_expressions() {
	ExpressionsVec result;
	for (InkChoiceEntry& entry : choices) {
		for (ExpressionParserV2::ShuntedExpression& condition : entry.conditions) {
			result.push_back(&condition);
		}

		for (InkObject* object : entry.text) {
			ExpressionsVec object_expressions = object->get_all_expressions();
			if (!object_expressions.empty()) {
				result.insert(result.end(), object_expressions.begin(), object_expressions.end());
			}
		}
	}

	return result;
}

std::vector<GatherPoint*> InkObjectChoice::get_choice_labels() {
	std::vector<GatherPoint*> result;
	for (InkChoiceEntry& choice : choices) {
		if (!choice.label.name.empty()) {
			result.push_back(&choice.label);
		}
	}

	return result;
}

std::vector<Knot*> InkObjectChoice::get_choice_result_knots() {
	std::vector<Knot*> result;
	for (InkChoiceEntry& choice : choices) {
		if (!choice.result.objects.empty()) {
			result.push_back(&choice.result);
		}
	}

	return result;
}

InkObjectChoice::GetChoicesResult InkObjectChoice::get_choices(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	GetChoicesResult choices_result;

	if (!story_state.current_knot().returning_from_function) {
		conditions_fully_prepared.clear();
		text_objects_being_prepared.clear();
		text_objects_fully_prepared.clear();
	}

	choices_result.fallback_index = std::nullopt;
	for (std::size_t i = 0; i < choices.size(); ++i) {
		InkChoiceEntry& this_choice = choices[i];
		if (this_choice.sticky || !story_state.has_choice_been_taken(this, i)) {
			if (!this_choice.fallback) {
				bool include_choice = true;
				std::vector<ExpressionParserV2::ShuntedExpression>& conditions = this_choice.conditions;
				if (!conditions.empty() && story_state.choice_divert_index != i) {
					for (ExpressionParserV2::ShuntedExpression& condition : conditions) {
						if (!conditions_fully_prepared.contains(condition.uuid)) {
							ExpressionParserV2::ExecuteResult condition_result = prepare_next_function_call(condition, story_state, eval_result, story_state.variable_info);
							if (!condition_result.has_value() && condition_result.error().reason == ExpressionParserV2::NulloptResult::Reason::FoundKnotFunction) {
								choices_result.function_prep_type = FunctionPrepType::Generic;

								for (std::size_t j = 0; j < choices_result.choices.size(); ++j) {
									story_state.current_choices.pop_back();
								}

								return choices_result;
							} else {
								conditions_fully_prepared.insert({condition.uuid, static_cast<bool>(*condition_result)});
							}

							if (!*condition_result) {
								include_choice = false;
								break;
							}
						} else {
							include_choice &= conditions_fully_prepared[condition.uuid];
							if (!include_choice) {
								break;
							}
						}
					}
				}

				if (include_choice) {
					story_state.choice_mix_position = InkStoryState::ChoiceMixPosition::Before;
				
					InkStoryEvalResult choice_eval_result;
					choice_eval_result.result.reserve(50);
					choice_eval_result.return_value = eval_result.return_value;
					for (InkObject* object : this_choice.text) {
						if (object->get_id() == ObjectId::ChoiceTextMix && static_cast<InkObjectChoiceTextMix*>(object)->is_end()) {
							break;
						}
						
						std::string result_before = choice_eval_result.result;
						Uuid previous_preparation_uuid = story_state.current_knot().current_function_prep_expression;
						object->execute(story_state, choice_eval_result);
						if (text_objects_being_prepared.contains(object)) {
							text_objects_being_prepared.erase(object);
							
							std::string object_added_text = choice_eval_result.result;
							object_added_text.erase(object_added_text.begin(), object_added_text.begin() + result_before.size());
							text_objects_fully_prepared.emplace(object, object_added_text);

							eval_result.return_value = std::nullopt;
						} else if (choice_eval_result.imminent_function_prep) {
							if (auto prepared_value = text_objects_fully_prepared.find(object); prepared_value != text_objects_fully_prepared.end()) {
								choice_eval_result.result += prepared_value->second;
								choice_eval_result.imminent_function_prep = FunctionPrepType::None;
								story_state.current_knot().current_function_prep_expression = previous_preparation_uuid;
							} else {
								eval_result.target_knot = choice_eval_result.target_knot;
								eval_result.divert_args = choice_eval_result.divert_args;
								eval_result.divert_type = DivertType::Function;
								if (object->get_id() == ObjectId::Interpolation) {
									choices_result.function_prep_type = FunctionPrepType::ChoiceTextInterpolate;
								} else {
									choices_result.function_prep_type = FunctionPrepType::Generic;
								}

								for (std::size_t j = 0; j < choices_result.choices.size(); ++j) {
									story_state.current_choices.pop_back();
								}

								text_objects_being_prepared.insert(object);
								return choices_result;
							}
						}
					}

					choices_result.choices.push_back({
						strip_string_edges(choice_eval_result.result, true, true, true),
						&this_choice,
						i,
					});

					story_state.current_choices.emplace_back(choices_result.choices.back().text, false);
				}
			} else {
				choices_result.fallback_index = i;
			}
		}	
	}

	for (std::size_t i = 0; i < choices_result.choices.size(); ++i) {
		story_state.current_choices.pop_back();
	}

	return choices_result;
}

void InkObjectChoice::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	bool do_choice_setup = !story_state.selected_choice.has_value() || story_state.current_choices.empty();
	bool select_choice_immediately = false;
	if (do_choice_setup) {
		story_state.in_choice_text = true;
		story_state.selected_choice = std::nullopt;

		GetChoicesResult final_choices = get_choices(story_state, eval_result);
		if (final_choices.function_prep_type != FunctionPrepType::None) {
			eval_result.imminent_function_prep = final_choices.function_prep_type;
			return;
		}

		bool in_thread = story_state.current_thread_depth > 0;
		if (!in_thread) {
			story_state.apply_thread_choices();
		}
		
		for (const ChoiceComponents& choice : final_choices.choices) {
			if (in_thread) {
				story_state.current_thread_entries.emplace_back(
					choice.text, choice.entry, choice.index,
					story_state.current_knot().knot, story_state.current_stitch, story_state.current_knot().index,
					story_state.thread_arguments_stack.back()
				);
			} else {
				story_state.current_choices.emplace_back(choice.text, false);
				story_state.current_choice_structs.push_back(choice.entry);
			}
		}

		if (story_state.current_thread_depth > 0) {
			story_state.should_wrap_up_thread = true;
		}
		
		story_state.in_choice_text = false;
		story_state.choice_mix_position = InkStoryState::ChoiceMixPosition::Before;

		if (!story_state.current_choices.empty()) {
			story_state.at_choice = true;
		} else if (final_choices.fallback_index.has_value()) {
			InkChoiceEntry& fallback_choice = choices[*final_choices.fallback_index];
			if (in_thread) {
				story_state.current_thread_entries.emplace_back(
					std::string(), &fallback_choice, fallback_choice.index,
					story_state.current_knot().knot, story_state.current_stitch, story_state.current_knot().index,
					story_state.thread_arguments_stack.back()
				);
			} else {
				story_state.current_knots_stack.push_back({&(choices[*final_choices.fallback_index].result), 0});
				story_state.at_choice = false;
			}
		}

		if (story_state.choice_divert_index.has_value()) {
			story_state.selected_choice = *story_state.choice_divert_index;
			select_choice_immediately = true;
		}
	}
	
	if ((!do_choice_setup || select_choice_immediately) && story_state.current_thread_depth == 0) {
		InkChoiceEntry* selected_choice_struct = nullptr;
		if (story_state.choice_divert_index.has_value()) {
			for (InkChoiceEntry* choice : story_state.current_choice_structs) {
				if (choice->index == *story_state.choice_divert_index) {
					selected_choice_struct = choice;
					break;
				}
			}
		} else {
			selected_choice_struct = story_state.current_choice_structs[*story_state.selected_choice];
		}

		story_state.add_choice_taken(this, selected_choice_struct->index);
		++story_state.total_choices_taken;

		story_state.choice_mix_position = InkStoryState::ChoiceMixPosition::Before;
		InkStoryEvalResult choice_eval_result;
		choice_eval_result.result.reserve(50);
		for (InkObject* object : selected_choice_struct->text) {
			object->execute(story_state, choice_eval_result);
			if (choice_eval_result.imminent_function_prep) {
				eval_result.target_knot = choice_eval_result.target_knot;
				eval_result.divert_type = DivertType::Function;
				eval_result.divert_args = choice_eval_result.divert_args;

				if (object->get_id() == ObjectId::Interpolation) {
					eval_result.imminent_function_prep = FunctionPrepType::ChoiceTextInterpolate;
				} else {
					eval_result.imminent_function_prep = FunctionPrepType::Generic;
				}
				
				return;
			}
		}

		story_state.current_knot().knot->function_prep_type = FunctionPrepType::None;

		eval_result.result = choice_eval_result.result;
		eval_result.reached_newline = !selected_choice_struct->immediately_continue_to_result && eval_result.has_any_contents(true);

		story_state.current_knots_stack.push_back({&(selected_choice_struct->result), 0});
		story_state.choice_mix_position = InkStoryState::ChoiceMixPosition::Before;

		story_state.story_tracking.increment_turns_since();

		if (!selected_choice_struct->label.name.empty()) {
			story_state.story_tracking.increment_visit_count(story_state.current_nonchoice_knot().knot, story_state.current_stitch, &selected_choice_struct->label);
		}

		story_state.current_choices.clear();
		story_state.current_choice_structs.clear();
		story_state.current_thread_entries.clear();
		story_state.selected_choice = std::nullopt;
		story_state.at_choice = false;
		story_state.choice_divert_index = std::nullopt;
	}
}
