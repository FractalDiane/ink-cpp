#include "objects/ink_object_globalvariable.h"

#include "ink_utils.h"

#include "shunting-yard.h"

#include "expression_parser/expression_parser.h"

InkObjectGlobalVariable::~InkObjectGlobalVariable() {
	for (ExpressionParser::Token* token : value_shunted_tokens) {
		delete token;
	}
}

void InkObjectGlobalVariable::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	//story_state.variables[name] = cparse::calculator::calculate(deinkify_expression(value).c_str(), story_state.get_variables_with_locals());
	ExpressionParser::VariableMap vars = story_state.get_variables_with_locals();
	story_state.variables[name] = ExpressionParser::execute_expression_tokens(value_shunted_tokens, vars, story_state.functions).value();
}
