#include "objects/ink_object_divert.h"

std::vector<std::uint8_t> InkObjectDivert::to_bytes() const {
	Serializer<std::string> s;
	return s(target_knot);
}

InkObject* InkObjectDivert::populate_from_bytes(const std::vector<std::uint8_t>& bytes, std::size_t& index) {
	Serializer<std::string> ds;
	target_knot = ds(bytes, index);
	return this;
}

void InkObjectDivert::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	if (target_knot == "END" || target_knot == "DONE") {
		story_state.should_end_story = true;
	} else {
		eval_result.target_knot = target_knot;
		for (const std::vector<ExpressionParser::Token*>& argument : arguments) {
			ExpressionParser::VariableMap knot_vars = story_state.story_tracking.get_visit_count_variables(story_state.current_knot().knot, story_state.current_stitch);
			ExpressionParser::Variant result = ExpressionParser::execute_expression_tokens(argument, story_state.variables, knot_vars, story_state.functions).value();
			eval_result.arguments.push_back(result);
		}
	}
}
