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

	bool is_return = false;
	if (!contents_shunted_tokens.empty()) {
		Token* first = contents_shunted_tokens[0];
		if (first->get_type() == TokenType::Keyword && static_cast<TokenKeyword*>(first)->data == TokenKeyword::Type::Return) {
			is_return = true;
			eval_result.reached_function_return = true;
		}
	}

	ExpressionParser::VariableMap story_constants = story_state.get_story_constants();
	
	std::optional<ExpressionParser::Variant> result = ExpressionParser::execute_expression_tokens(contents_shunted_tokens, story_state.variables, story_constants, story_state.functions);
	if (is_return) {
		if (result.has_value()) {
			eval_result.result += ExpressionParser::to_printable_string(*result);
		}

		//story_state.current_knots_stack.pop_back();
		eval_result.return_value = result;
	}

	/*if (is_temp_declaration) {
		InkWeaveContent* var_owner = story_state.current_stitch
									? static_cast<InkWeaveContent*>(story_state.current_stitch)
									: static_cast<InkWeaveContent*>(story_state.current_knot().knot);

		var_owner->local_variables.push_back(temp_var_name);
	}*/
}
