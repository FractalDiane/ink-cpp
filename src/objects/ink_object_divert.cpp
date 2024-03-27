#include "objects/ink_object_divert.h"

InkObjectDivert::~InkObjectDivert() {
	for (ExpressionParser::Token* token : target_knot) {
		delete token;
	}

	for (const std::vector<ExpressionParser::Token*>& argument : arguments) {
		for (ExpressionParser::Token* token : argument) {
			delete token;
		}
	}
}

std::vector<std::uint8_t> InkObjectDivert::to_bytes() const {
	VectorSerializer<ExpressionParser::Token*> starget;
	Serializer<std::uint8_t> s8;
	Serializer<std::uint16_t> s16;
	
	ByteVec result = starget(target_knot);
	ByteVec result2 = s8(static_cast<std::uint8_t>(type));

	ByteVec result3 = s16(static_cast<std::uint16_t>(arguments.size()));
	result.insert(result.end(), result2.begin(), result2.end());
	result.insert(result.end(), result3.begin(), result3.end());
	for (const auto& arg : arguments) {
		ByteVec result_arg = starget(arg);
		result.insert(result.end(), result_arg.begin(), result_arg.end());
	}

	return result;
}

InkObject* InkObjectDivert::populate_from_bytes(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::uint8_t> ds8;
	Deserializer<std::uint16_t> ds16;
	VectorDeserializer<ExpressionParser::Token*> dstarget;

	target_knot = dstarget(bytes, index);
	type = static_cast<DivertType>(ds8(bytes, index));

	std::size_t arg_count = static_cast<std::size_t>(ds16(bytes, index));
	for (std::size_t i = 0; i < arg_count; ++i) {
		arguments.push_back(dstarget(bytes, index));
	}

	return this;
}

std::string InkObjectDivert::get_target(InkStoryState& story_state, const ExpressionParser::VariableMap& story_constants) {
	std::string target;	

	std::optional<ExpressionParser::Variant> target_var = ExpressionParser::execute_expression_tokens(target_knot, story_state.variables, story_constants, story_state.variable_redirects, story_state.functions);
	if (target_var.has_value() && target_var->index() == ExpressionParser::Variant_String) {
		target = ExpressionParser::as_string(*target_var);
	} else if (!target_knot.empty()) {
		target = target_knot[0]->to_printable_string();
	}

	return target;
}

void InkObjectDivert::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	ExpressionParser::VariableMap story_constants = story_state.get_story_constants();
	std::string target = get_target(story_state, story_constants);

	if (target == "END" || target == "DONE") {
		story_state.should_end_story = true;
	} else {
		eval_result.target_knot = target;
		eval_result.divert_type = type;
		for (const std::vector<ExpressionParser::Token*>& argument : arguments) {
			ExpressionParser::Variant result = ExpressionParser::execute_expression_tokens(argument, story_state.variables, story_constants, story_state.variable_redirects, story_state.functions).value();
			eval_result.arguments.push_back(result);
		}
	}
}
