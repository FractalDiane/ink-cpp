#include "objects/ink_object_interpolation.h"

#include "ink_utils.h"

#include "expression_parser/expression_parser.h"

ByteVec InkObjectInterpolation::to_bytes() const {
	VectorSerializer<ExpressionParser::Token*> s;
	return s(what_to_interpolate.tokens);
}

InkObject* InkObjectInterpolation::populate_from_bytes(const ByteVec& bytes, std::size_t& index) {
	VectorDeserializer<ExpressionParser::Token*> ds;
	what_to_interpolate = ExpressionParser::ShuntedExpression(ds(bytes, index));
	return this;
}

InkObjectInterpolation::~InkObjectInterpolation() {
	what_to_interpolate.dealloc_tokens();
}

void InkObjectInterpolation::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	ExpressionParser::VariableMap story_constants = story_state.get_story_constants();
	if (prepare_next_function_call(what_to_interpolate, story_state, eval_result, story_state.variables, story_constants, story_state.variable_redirects)) {
		return;
	}

	std::optional<ExpressionParser::Variant> result = ExpressionParser::execute_expression_tokens(what_to_interpolate.function_prepared_tokens, story_state.variables, story_constants, story_state.variable_redirects, story_state.functions);
	if (result.has_value()) {
		eval_result.result += ExpressionParser::to_printable_string(*result);
	}
}
