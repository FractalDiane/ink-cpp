#include "objects/ink_object_conditional.h"

#include "ink_utils.h"

ByteVec Serializer<InkObjectConditional::Entry>::operator()(const InkObjectConditional::Entry& entry) {
	VectorSerializer<ExpressionParser::Token*> stokens;
	Serializer<Knot> sknot;

	ByteVec result = stokens(entry.first.tokens);
	ByteVec result2 = sknot(entry.second);
	result.insert(result.end(), result2.begin(), result2.end());

	return result;
}

InkObjectConditional::Entry Deserializer<InkObjectConditional::Entry>::operator()(const ByteVec& bytes, std::size_t& index) {
	VectorDeserializer<ExpressionParser::Token*> dstokens;
	Deserializer<Knot> dsknot;
	
	InkObjectConditional::Entry result;
	result.first = ExpressionParser::ShuntedExpression(dstokens(bytes, index));
	result.second = dsknot(bytes, index);

	return result;
}

ByteVec InkObjectConditional::to_bytes() const {
	Serializer<std::uint8_t> s8;
	VectorSerializer<Entry> sentries;
	Serializer<Knot> sknot;
	VectorSerializer<ExpressionParser::Token*> stokens;

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
	VectorDeserializer<ExpressionParser::Token*> dstokens;

	is_switch = static_cast<bool>(ds8(bytes, index));
	if (is_switch) {
		switch_expression = ExpressionParser::ShuntedExpression(dstokens(bytes, index));
	}

	branches = dsentries(bytes, index);
	branch_else = dsknot(bytes, index);

	return this;
}

InkObjectConditional::~InkObjectConditional() {
	for (auto& entry : branches) {
		entry.first.dealloc_tokens();

		for (InkObject* object : entry.second.objects) {
			delete object;
		}
	}

	for (InkObject* object : branch_else.objects) {
		delete object;
	}

	switch_expression.dealloc_tokens();
}

void InkObjectConditional::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	ExpressionParser::VariableMap story_constants = story_state.get_story_constants();
	
	if (!is_switch) {
		for (Entry& entry : branches) {
			//ExpressionParser::Variant condition_result = ExpressionParser::execute_expression_tokens(entry.first.tokens, story_state.variables, story_constants, story_state.variable_redirects, story_state.functions).value();
			ExpressionParser::ExecuteResult condition_result = prepare_next_function_call(entry.first, story_state, eval_result, story_state.variables, story_constants, story_state.variable_redirects);
			if (!condition_result.has_value() && condition_result.error().reason == ExpressionParser::NulloptResult::Reason::FoundKnotFunction) {
				return;
			}
			
			if (condition_result.has_value() && ExpressionParser::as_bool(*condition_result)) {
				story_state.current_knots_stack.push_back({&(entry.second), 0});
				return;
			}
		}
	} else {
		// TODO: this might be redundant and strictly worse performance than the above version
		//ExpressionParser::Variant result = ExpressionParser::execute_expression_tokens(switch_expression.tokens, story_state.variables, story_constants, story_state.variable_redirects, story_state.functions).value();
		ExpressionParser::ExecuteResult result = prepare_next_function_call(switch_expression, story_state, eval_result, story_state.variables, story_constants, story_state.variable_redirects);
		if (!result.has_value() && result.error().reason == ExpressionParser::NulloptResult::Reason::FoundKnotFunction) {
			return;
		}

		for (auto& entry : branches) {
			//ExpressionParser::Variant condition_result = ExpressionParser::execute_expression_tokens(entry.first.tokens, story_state.variables, story_constants, story_state.variable_redirects, story_state.functions).value();
			ExpressionParser::ExecuteResult condition_result = prepare_next_function_call(entry.first, story_state, eval_result, story_state.variables, story_constants, story_state.variable_redirects);
			if (!condition_result.has_value() && condition_result.error().reason == ExpressionParser::NulloptResult::Reason::FoundKnotFunction) {
				return;
			}

			if (condition_result.has_value()) {
				if (*condition_result == result) {
					story_state.current_knots_stack.push_back({&(entry.second), 0});
					return;
				}
			}
		}
	}

	story_state.current_knots_stack.push_back({&branch_else, 0});
}
