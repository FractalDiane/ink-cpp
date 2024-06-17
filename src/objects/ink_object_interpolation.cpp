#include "objects/ink_object_interpolation.h"

#include "ink_utils.h"

#include "expression_parser/expression_parser.h"

ByteVec InkObjectInterpolation::to_bytes() const {
	VectorSerializer<ExpressionParserV2::Token> s;
	return s(what_to_interpolate.tokens);
}

InkObject* InkObjectInterpolation::populate_from_bytes(const ByteVec& bytes, std::size_t& index) {
	VectorDeserializer<ExpressionParserV2::Token> ds;
	what_to_interpolate = ExpressionParserV2::ShuntedExpression(ds(bytes, index));
	return this;
}

InkObjectInterpolation::~InkObjectInterpolation() {
	//what_to_interpolate.dealloc_tokens();
}

void InkObjectInterpolation::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	//ExpressionParser::VariableMap story_constants = story_state.get_story_constants();
	story_state.update_local_knot_variables();

	ExpressionParserV2::ExecuteResult interpolate_result = prepare_next_function_call(what_to_interpolate, story_state, eval_result, story_state.variable_info);
	if (!interpolate_result.has_value() && interpolate_result.error().reason == ExpressionParserV2::NulloptResult::Reason::FoundKnotFunction) {
		return;
	}

	//ExpressionParser::ExecuteResult result = ExpressionParser::execute_expression_tokens(what_to_interpolate.function_prepared_tokens, story_state.variables, story_constants, story_state.variable_redirects, story_state.functions);
	if (interpolate_result.has_value()) {
		std::string result = interpolate_result->to_printable_string();
		eval_result.result += result;
		story_state.current_nonchoice_knot().any_new_content = !result.empty();
	}
}
