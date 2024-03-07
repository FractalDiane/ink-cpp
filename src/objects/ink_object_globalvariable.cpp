#include "objects/ink_object_globalvariable.h"

#include "ink_utils.h"

#include "expression_parser/expression_parser.h"

InkObjectGlobalVariable::~InkObjectGlobalVariable() {
	for (ExpressionParser::Token* token : value_shunted_tokens) {
		delete token;
	}
}

void InkObjectGlobalVariable::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	ExpressionParser::VariableMap knot_vars = story_state.story_tracking.get_visit_count_variables(story_state.current_knot().knot, story_state.current_stitch);
	story_state.variables[name] = ExpressionParser::execute_expression_tokens(value_shunted_tokens, story_state.variables, knot_vars, story_state.functions).value();
}
