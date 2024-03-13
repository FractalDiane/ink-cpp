#include "objects/ink_object_globalvariable.h"

#include "ink_utils.h"

#include "expression_parser/expression_parser.h"

InkObjectGlobalVariable::~InkObjectGlobalVariable() {
	for (ExpressionParser::Token* token : value_shunted_tokens) {
		delete token;
	}
}

void InkObjectGlobalVariable::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	ExpressionParser::VariableMap story_constants = story_state.get_story_constants();
	auto& map = is_constant ? story_state.constants : story_state.variables;
	map[name] = ExpressionParser::execute_expression_tokens(value_shunted_tokens, story_state.variables, story_constants, story_state.functions).value();
}
