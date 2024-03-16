#include "objects/ink_object_logic.h"

#include "runtime/ink_story.h"
#include "ink_utils.h"

#include "expression_parser/expression_parser.h"

ByteVec InkObjectLogic::to_bytes() const {
	VectorSerializer<ExpressionParser::Token*> s;
	return s(contents_shunted_tokens);
}

InkObject* InkObjectLogic::populate_from_bytes(const ByteVec& bytes, std::size_t& index) {
	VectorDeserializer<ExpressionParser::Token*> ds;
	contents_shunted_tokens = ds(bytes, index);
	return this;
}

InkObjectLogic::~InkObjectLogic() {
	for (ExpressionParser::Token* token : contents_shunted_tokens) {
		delete token;
	}
}

void InkObjectLogic::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	using namespace ExpressionParser;

	bool is_return = false;
	if (!contents_shunted_tokens.empty()) {
		Token* first = contents_shunted_tokens[0];
		if (first->get_type() == TokenType::Keyword && static_cast<TokenKeyword*>(first)->data == TokenKeyword::Type::Return) {
			is_return = true;
			eval_result.reached_function_return = true;
		}
	}

	ExpressionParser::VariableMap story_constants = story_state.get_story_constants();
	
	std::optional<ExpressionParser::Variant> result = ExpressionParser::execute_expression_tokens(contents_shunted_tokens, story_state.variables, story_constants, story_state.variable_redirects, story_state.functions);
	if (is_return) {
		if (result.has_value()) {
			eval_result.result += ExpressionParser::to_printable_string(*result);
		}

		eval_result.return_value = result;
	}
}
