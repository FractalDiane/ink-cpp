#include "objects/ink_object_logic.h"

#include "runtime/ink_story.h"
#include "ink_utils.h"

#include "expression_parser/expression_parser.h"

ByteVec InkObjectLogic::to_bytes() const {
	VectorSerializer<ExpressionParser::Token*> s;
	return s(contents_shunted_tokens.tokens);
}

InkObject* InkObjectLogic::populate_from_bytes(const ByteVec& bytes, std::size_t& index) {
	VectorDeserializer<ExpressionParser::Token*> ds;
	contents_shunted_tokens = ExpressionParser::ShuntedExpression(ds(bytes, index));
	return this;
}

InkObjectLogic::~InkObjectLogic() {
	contents_shunted_tokens.dealloc_tokens();
}

void InkObjectLogic::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	using namespace ExpressionParser;

	ExpressionParser::VariableMap story_constants = story_state.get_story_constants();

	ExpressionParser::ExecuteResult logic_result = prepare_next_function_call(contents_shunted_tokens, story_state, eval_result, story_state.variables, story_constants, story_state.variable_redirects);
	if (!logic_result.has_value() && logic_result.error().reason == ExpressionParser::NulloptResult::Reason::FoundKnotFunction) {
		return;
	}

	bool is_return = false;
	if (!contents_shunted_tokens.tokens.empty()) {
		Token* first = contents_shunted_tokens.tokens[0];
		if (first->get_type() == TokenType::Keyword && static_cast<TokenKeyword*>(first)->data == TokenKeyword::Type::Return) {
			is_return = true;
			eval_result.reached_function_return = true;
		}
	}

	//ExpressionParser::ExecuteResult result = ExpressionParser::execute_expression_tokens(contents_shunted_tokens.function_prepared_tokens, story_state.variables, story_constants, story_state.variable_redirects, story_state.functions);
	if (is_return) {
		//if (result.has_value()) {
		//	eval_result.result += ExpressionParser::to_printable_string(*result);
		//}

		if (logic_result.has_value()) {
			eval_result.return_value = *logic_result;
		} else {
			eval_result.return_value = std::nullopt;
		}
	}
}
