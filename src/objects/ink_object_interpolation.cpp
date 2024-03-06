#include "objects/ink_object_interpolation.h"

#include "ink_utils.h"

#include "shunting-yard.h"

#include "expression_parser/expression_parser.h"

InkObjectInterpolation::~InkObjectInterpolation() {
	for (ExpressionParser::Token* token : what_to_interpolate) {
		delete token;
	}
}

void InkObjectInterpolation::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	ExpressionParser::VariableMap knot_vars = story_state.story_tracking.get_visit_count_variables(story_state.current_knot().knot, story_state.current_stitch);
	//cparse::packToken result = cparse::calculator::calculate(deinkify_expression(what_to_interpolate).c_str(), story_state.get_variables_with_locals());
	ExpressionParser::Variant result = ExpressionParser::execute_expression_tokens(what_to_interpolate, story_state.variables, knot_vars, story_state.functions).value();
	eval_result.result += ExpressionParser::to_printable_string(result);
}
