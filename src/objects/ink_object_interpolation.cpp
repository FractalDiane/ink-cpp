#include "objects/ink_object_interpolation.h"

#include "ink_utils.h"

#include "expression_parser/expression_parser.h"

InkObjectInterpolation::~InkObjectInterpolation() {
	for (ExpressionParser::Token* token : what_to_interpolate) {
		delete token;
	}
}

void InkObjectInterpolation::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	ExpressionParser::VariableMap story_constants = story_state.get_story_constants();
	std::optional<ExpressionParser::Variant> result = ExpressionParser::execute_expression_tokens(what_to_interpolate, story_state.variables, story_constants, story_state.variable_redirects, story_state.functions);
	if (result.has_value()) {
		eval_result.result += ExpressionParser::to_printable_string(*result);
	}
}
