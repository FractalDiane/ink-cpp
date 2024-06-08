#include "objects/ink_object_globalvariable.h"

#include "ink_utils.h"

#include "expression_parser/expression_parser.h"

ByteVec InkObjectGlobalVariable::to_bytes() const {
	Serializer<std::uint8_t> s8;
	Serializer<std::string> sstring;
	VectorSerializer<ExpressionParser::Token*> vstoken;
	
	ByteVec result = sstring(name);
	ByteVec result2 = s8(static_cast<std::uint8_t>(is_constant));
	ByteVec result3 = vstoken(value_shunted_tokens.tokens);

	result.insert(result.end(), result2.begin(), result2.end());
	result.insert(result.end(), result3.begin(), result3.end());

	return result;
}

InkObject* InkObjectGlobalVariable::populate_from_bytes(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::uint8_t> ds8;
	Deserializer<std::string> dsstring;
	VectorDeserializer<ExpressionParser::Token*> vdstoken;

	name = dsstring(bytes, index);
	is_constant = static_cast<bool>(ds8(bytes, index));
	value_shunted_tokens = ExpressionParser::ShuntedExpression(vdstoken(bytes, index));

	return this;
}

InkObjectGlobalVariable::~InkObjectGlobalVariable() {
	value_shunted_tokens.dealloc_tokens();
}

void InkObjectGlobalVariable::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	ExpressionParser::VariableMap story_constants = story_state.get_story_constants();
	auto& map = is_constant ? story_state.constants : story_state.variables;
	
	ExpressionParser::ExecuteResult result = prepare_next_function_call(value_shunted_tokens, story_state, eval_result, story_state.variables, story_constants, story_state.variable_redirects);
	if (!result.has_value() && result.error().reason == ExpressionParser::NulloptResult::Reason::FoundKnotFunction) {
		return;
	}

	if (result.has_value()) {
		map[name] = *result;
	}
}
