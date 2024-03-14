#include "objects/ink_object_conditional.h"

#include "ink_utils.h"

InkObjectConditional::~InkObjectConditional() {
	for (auto& entry : branches) {
		for (ExpressionParser::Token* token : entry.first) {
			delete token;
		}

		for (InkObject* object : entry.second.objects) {
			delete object;
		}
	}

	for (InkObject* object : branch_else.objects) {
		delete object;
	}

	for (ExpressionParser::Token* token : switch_expression) {
		delete token;
	}
}

void InkObjectConditional::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	ExpressionParser::VariableMap story_constants = story_state.get_story_constants();
	
	if (!is_switch) {
		for (auto& entry : branches) {
			ExpressionParser::Variant condition_result = ExpressionParser::execute_expression_tokens(entry.first, story_state.variables, story_constants, story_state.variable_redirects, story_state.functions).value();
			if (ExpressionParser::as_bool(condition_result)) {
				story_state.current_knots_stack.push_back({&(entry.second), 0});
				return;
			}
		}
	} else {
		// TODO: this might be redundant and strictly worse performance than the above version
		ExpressionParser::Variant result = ExpressionParser::execute_expression_tokens(switch_expression, story_state.variables, story_constants, story_state.variable_redirects, story_state.functions).value();
		for (auto& entry : branches) {
			ExpressionParser::Variant condition_result = ExpressionParser::execute_expression_tokens(entry.first, story_state.variables, story_constants, story_state.variable_redirects, story_state.functions).value();
			if (condition_result == result) {
				story_state.current_knots_stack.push_back({&(entry.second), 0});
				return;
			}
		}
	}

	story_state.current_knots_stack.push_back({&branch_else, 0});
}
