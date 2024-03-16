#include "objects/ink_object_conditional.h"

#include "ink_utils.h"

ByteVec Serializer<InkObjectConditional::Entry>::operator()(const InkObjectConditional::Entry& entry) {
	VectorSerializer<ExpressionParser::Token*> stokens;
	Serializer<Knot> sknot;

	ByteVec result = stokens(entry.first);
	ByteVec result2 = sknot(entry.second);
	result.append_range(result2);

	return result;
}

InkObjectConditional::Entry Deserializer<InkObjectConditional::Entry>::operator()(const ByteVec& bytes, std::size_t& index) {
	VectorDeserializer<ExpressionParser::Token*> dstokens;
	Deserializer<Knot> dsknot;
	
	InkObjectConditional::Entry result;
	result.first = dstokens(bytes, index);
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
		ByteVec result2 = stokens(switch_expression);
		result.append_range(result2);
	}

	ByteVec result3 = sentries(branches);
	ByteVec result4 = sknot(branch_else);

	result.append_range(result3);
	result.append_range(result4);

	return result;
}

InkObject* InkObjectConditional::populate_from_bytes(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::uint8_t> ds8;
	VectorDeserializer<Entry> dsentries;
	Deserializer<Knot> dsknot;
	VectorDeserializer<ExpressionParser::Token*> dstokens;

	is_switch = static_cast<bool>(ds8(bytes, index));
	if (is_switch) {
		switch_expression = dstokens(bytes, index);
	}

	branches = dsentries(bytes, index);
	branch_else = dsknot(bytes, index);

	return this;
}

InkObjectConditional::~InkObjectConditional() {
	for (auto& entry : branches) {
		for (ExpressionParser::Token* token : entry.first) {
			delete token;
		}

		for (InkObject* object : entry.second.objects) {
			delete object;
		}
	}

	for (InkObject* object : branch_else.objects) {
		delete object;
	}

	for (ExpressionParser::Token* token : switch_expression) {
		delete token;
	}
}

void InkObjectConditional::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	ExpressionParser::VariableMap story_constants = story_state.get_story_constants();
	
	if (!is_switch) {
		for (auto& entry : branches) {
			ExpressionParser::Variant condition_result = ExpressionParser::execute_expression_tokens(entry.first, story_state.variables, story_constants, story_state.variable_redirects, story_state.functions).value();
			if (ExpressionParser::as_bool(condition_result)) {
				story_state.current_knots_stack.push_back({&(entry.second), 0});
				return;
			}
		}
	} else {
		// TODO: this might be redundant and strictly worse performance than the above version
		ExpressionParser::Variant result = ExpressionParser::execute_expression_tokens(switch_expression, story_state.variables, story_constants, story_state.variable_redirects, story_state.functions).value();
		for (auto& entry : branches) {
			ExpressionParser::Variant condition_result = ExpressionParser::execute_expression_tokens(entry.first, story_state.variables, story_constants, story_state.variable_redirects, story_state.functions).value();
			if (condition_result == result) {
				story_state.current_knots_stack.push_back({&(entry.second), 0});
				return;
			}
		}
	}

	story_state.current_knots_stack.push_back({&branch_else, 0});
}
