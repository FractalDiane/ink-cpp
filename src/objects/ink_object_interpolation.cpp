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
	ExpressionParser::TokenMap vars = story_state.get_variables_with_locals();
	//cparse::packToken result = cparse::calculator::calculate(deinkify_expression(what_to_interpolate).c_str(), story_state.get_variables_with_locals());
	ExpressionParser::PackedToken result = ExpressionParser::execute_expression_tokens(what_to_interpolate, vars);
	eval_result.result += result.to_printable_string();
}
