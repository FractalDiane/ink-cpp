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

/*std::vector<std::uint8_t> InkObjectDivert::to_bytes() const {
	Serializer<std::string> s;
	return s(target_knot);
}

InkObject* InkObjectDivert::populate_from_bytes(const std::vector<std::uint8_t>& bytes, std::size_t& index) {
	Serializer<std::string> ds;
	target_knot = ds(bytes, index);
	return this;
}*/

void InkObjectDivert::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	ExpressionParser::VariableMap knot_vars = story_state.story_tracking.get_visit_count_variables(story_state.current_knot().knot, story_state.current_stitch);

	std::string target;	
	std::optional<ExpressionParser::Variant> target_var = ExpressionParser::execute_expression_tokens(target_knot, story_state.variables, knot_vars, story_state.functions);
	if (target_var.has_value() && target_var->index() == ExpressionParser::Variant_String) {
		target = ExpressionParser::as_string(*target_var);
	} else {
		target = target_knot[0]->to_printable_string();
	}

	if (target == "END" || target == "DONE") {
		story_state.should_end_story = true;
	} else {
		eval_result.target_knot = target;
		for (const std::vector<ExpressionParser::Token*>& argument : arguments) {
			ExpressionParser::Variant result = ExpressionParser::execute_expression_tokens(argument, story_state.variables, knot_vars, story_state.functions).value();
			eval_result.arguments.push_back(result);
		}
	}
}
