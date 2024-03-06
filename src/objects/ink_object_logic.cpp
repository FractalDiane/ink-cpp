#include "objects/ink_object_logic.h"

#include "runtime/ink_story.h"
#include "ink_utils.h"

#include "shunting-yard.h"

#include "expression_parser/expression_parser.h"

InkObjectLogic::~InkObjectLogic() {
	for (ExpressionParser::Token* token : contents_shunted_tokens) {
		delete token;
	}
}

void InkObjectLogic::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	//cparse::calculator::calculate(deinkify_expression(contents).c_str(), story_state.get_variables_with_locals());
	ExpressionParser::VariableMap vars = story_state.get_variables_with_locals();
	//ExpressionParser::execute_expression(contents, vars);
	static_cast<void>(ExpressionParser::execute_expression_tokens(contents_shunted_tokens, vars, story_state.functions));
}
