#include "objects/ink_object_divert.h"

InkObjectDivert::~InkObjectDivert() {
	/*target_knot.dealloc_tokens();

	for (ExpressionParser::ShuntedExpression& argument : arguments) {
		argument.dealloc_tokens();
	}*/
}

std::vector<std::uint8_t> InkObjectDivert::to_bytes() const {
	VectorSerializer<ExpressionParserV2::Token> starget;
	Serializer<std::uint8_t> s8;
	Serializer<std::uint16_t> s16;
	
	ByteVec result = starget(target_knot.tokens);
	ByteVec result2 = s8(static_cast<std::uint8_t>(type));

	ByteVec result3 = s16(static_cast<std::uint16_t>(arguments.size()));
	result.insert(result.end(), result2.begin(), result2.end());
	result.insert(result.end(), result3.begin(), result3.end());
	for (const auto& arg : arguments) {
		ByteVec result_arg = starget(arg.tokens);
		result.insert(result.end(), result_arg.begin(), result_arg.end());
	}

	return result;
}

InkObject* InkObjectDivert::populate_from_bytes(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::uint8_t> ds8;
	Deserializer<std::uint16_t> ds16;
	VectorDeserializer<ExpressionParserV2::Token> dstarget;

	target_knot = ExpressionParserV2::ShuntedExpression(dstarget(bytes, index));
	type = static_cast<DivertType>(ds8(bytes, index));

	std::size_t arg_count = static_cast<std::size_t>(ds16(bytes, index));
	for (std::size_t i = 0; i < arg_count; ++i) {
		arguments.push_back(ExpressionParserV2::ShuntedExpression(dstarget(bytes, index)));
	}

	return this;
}

std::string InkObjectDivert::get_target(InkStoryState& story_state, const ExpressionParserV2::StoryVariableInfo& story_var_info) {
	std::string target;

	ExpressionParserV2::ExecuteResult target_var = ExpressionParserV2::execute_expression_tokens(target_knot.tokens, story_state.variable_info);
	if (target_var.has_value() && target_var->index() == ExpressionParserV2::Variant_String) {
		target = static_cast<std::string>(*target_var);
	} else if (!target_knot.tokens.empty()) {
		target = target_knot.tokens[0].variable_name;
	}

	return target;
}

void InkObjectDivert::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	story_state.update_local_knot_variables();
	std::string target = get_target(story_state, story_state.variable_info);

	bool is_done = target == "DONE";
	if (is_done && (story_state.current_thread_depth > 0 || !story_state.current_thread_entries.empty())) {
		story_state.apply_thread_choices();
		story_state.should_wrap_up_thread = true;
	}
	else if (is_done || target == "END") {
		story_state.should_end_story = true;
	} else {
		eval_result.target_knot = target;
		eval_result.divert_type = type;
		story_state.arguments_stack.push_back({});
		std::vector<std::pair<std::string, ExpressionParserV2::Variant>>& args = story_state.arguments_stack.back();
		for (ExpressionParserV2::ShuntedExpression& argument : arguments) {
			ExpressionParserV2::Variant result = ExpressionParserV2::execute_expression_tokens(argument.tokens, story_state.variable_info).value();
			args.push_back({
				argument.tokens.size() == 1 && argument.tokens[0].type == ExpressionParserV2::TokenType::Variable
					? argument.tokens[0].variable_name
					: std::string(),
				result,
			});
		}
	}
}
