#include "objects/ink_object_logic.h"

#include "runtime/ink_story.h"
#include "ink_utils.h"

#include "expression_parser/expression_parser.h"

InkObjectLogic::~InkObjectLogic() {
	for (ExpressionParser::Token* token : contents_shunted_tokens) {
		delete token;
	}
}

void InkObjectLogic::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	using namespace ExpressionParser;

	/*bool is_temp_declaration = false;
	std::string temp_var_name;
	if (contents_shunted_tokens.size() > 2) {
		Token* first = contents_shunted_tokens[0];
		if (first->get_type() == TokenType::Keyword && static_cast<TokenKeyword*>(first)->data == TokenKeyword::Type::Temp) {
			Token* second = contents_shunted_tokens[1];
			if (second->get_type() == ExpressionParser::TokenType::Variable) {
				temp_var_name = static_cast<TokenVariable*>(second)->data;
				story_state.variables[temp_var_name] = false;
				is_temp_declaration = true;
			}
		}
	}*/

	ExpressionParser::VariableMap knot_vars = story_state.story_tracking.get_visit_count_variables(story_state.current_knot().knot, story_state.current_stitch);
	static_cast<void>(ExpressionParser::execute_expression_tokens(contents_shunted_tokens, story_state.variables, knot_vars, story_state.functions));

	/*if (is_temp_declaration) {
		InkWeaveContent* var_owner = story_state.current_stitch
									? static_cast<InkWeaveContent*>(story_state.current_stitch)
									: static_cast<InkWeaveContent*>(story_state.current_knot().knot);

		var_owner->local_variables.push_back(temp_var_name);
	}*/
}
