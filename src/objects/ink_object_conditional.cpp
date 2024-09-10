#include "objects/ink_object_conditional.h"

#include "ink_utils.h"

ByteVec Serializer<InkObjectConditional::Entry>::operator()(const InkObjectConditional::Entry& entry) {
	VectorSerializer<ExpressionParserV2::Token> stokens;
	Serializer<Knot> sknot;

	ByteVec result = stokens(entry.first.tokens);
	ByteVec result2 = sknot(entry.second);
	result.insert(result.end(), result2.begin(), result2.end());

	return result;
}

InkObjectConditional::Entry Deserializer<InkObjectConditional::Entry>::operator()(const ByteVec& bytes, std::size_t& index) {
	VectorDeserializer<ExpressionParserV2::Token> dstokens;
	Deserializer<Knot> dsknot;
	
	InkObjectConditional::Entry result;
	result.first = ExpressionParserV2::ShuntedExpression(dstokens(bytes, index));
	result.second = dsknot(bytes, index);

	return result;
}

ByteVec InkObjectConditional::to_bytes() const {
	Serializer<std::uint8_t> s8;
	VectorSerializer<Entry> sentries;
	Serializer<Knot> sknot;
	VectorSerializer<ExpressionParserV2::Token> stokens;

	ByteVec result = s8(static_cast<std::uint8_t>(is_switch));
	if (is_switch) {
		ByteVec result2 = stokens(switch_expression.tokens);
		result.insert(result.end(), result2.begin(), result2.end());
	}

	ByteVec result3 = sentries(branches);
	ByteVec result4 = sknot(branch_else);

	result.insert(result.end(), result3.begin(), result3.end());
	result.insert(result.end(), result4.begin(), result4.end());

	return result;
}

InkObject* InkObjectConditional::populate_from_bytes(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::uint8_t> ds8;
	VectorDeserializer<Entry> dsentries;
	Deserializer<Knot> dsknot;
	VectorDeserializer<ExpressionParserV2::Token> dstokens;

	is_switch = static_cast<bool>(ds8(bytes, index));
	if (is_switch) {
		switch_expression = ExpressionParserV2::ShuntedExpression(dstokens(bytes, index));
	}

	branches = dsentries(bytes, index);
	branch_else = dsknot(bytes, index);

	return this;
}

InkObjectConditional::~InkObjectConditional() {
	for (auto& entry : branches) {
		for (InkObject* object : entry.second.objects) {
			delete object;
		}
	}

	for (InkObject* object : branch_else.objects) {
		delete object;
	}
}

void InkObjectConditional::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	story_state.update_local_knot_variables();
	
	if (!is_switch) {
		for (Entry& entry : branches) {
			ExpressionParserV2::ExecuteResult condition_result = prepare_next_function_call(entry.first, story_state, eval_result, story_state.variable_info);
			if (!condition_result.has_value() && condition_result.error().reason == ExpressionParserV2::NulloptResult::Reason::FoundKnotFunction) {
				return;
			}
			
			if (condition_result.has_value() && (*condition_result)) {
				entry.second.function_prep_type = story_state.current_knot().knot->function_prep_type;
				story_state.current_knots_stack.push_back({&(entry.second), 0});
				return;
			}
		}
	} else {
		// TODO: this might be redundant and strictly worse performance than the above version
		ExpressionParserV2::ExecuteResult result = prepare_next_function_call(switch_expression, story_state, eval_result, story_state.variable_info);
		if (!result.has_value() && result.error().reason == ExpressionParserV2::NulloptResult::Reason::FoundKnotFunction) {
			return;
		}

		for (auto& entry : branches) {
			ExpressionParserV2::ExecuteResult condition_result = prepare_next_function_call(entry.first, story_state, eval_result, story_state.variable_info);
			if (!condition_result.has_value() && condition_result.error().reason == ExpressionParserV2::NulloptResult::Reason::FoundKnotFunction) {
				return;
			}

			if (condition_result.has_value()) {
				if (*condition_result == result) {
					entry.second.function_prep_type = story_state.current_knot().knot->function_prep_type;
					story_state.current_knots_stack.push_back({&(entry.second), 0});
					return;
				}
			}
		}
	}

	branch_else.function_prep_type = story_state.current_knot().knot->function_prep_type;
	story_state.current_knots_stack.push_back({&branch_else, 0});
}

bool InkObjectConditional::contributes_content_to_knot() const {
	for (const Entry& branch : branches) {
		for (const InkObject* object : branch.second.objects) {
			if (object->contributes_content_to_knot()) {
				return true;
			}
		}
	}

	if (!is_switch) {
		for (const InkObject* object : branch_else.objects) {
			if (object->contributes_content_to_knot()) {
				return true;
			}
		}
	}

	return false;
}
