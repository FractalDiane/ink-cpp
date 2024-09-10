#include "objects/ink_object_logic.h"

#include "runtime/ink_story.h"
#include "ink_utils.h"

#include "expression_parser/expression_parser.h"

ByteVec InkObjectLogic::to_bytes() const {
	VectorSerializer<ExpressionParserV2::Token> s;
	return s(contents_shunted_tokens.tokens);
}

InkObject* InkObjectLogic::populate_from_bytes(const ByteVec& bytes, std::size_t& index) {
	VectorDeserializer<ExpressionParserV2::Token> ds;
	contents_shunted_tokens = ExpressionParserV2::ShuntedExpression(ds(bytes, index));
	return this;
}

InkObjectLogic::~InkObjectLogic() {
	
}

void InkObjectLogic::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	story_state.update_local_knot_variables();

	ExpressionParserV2::ExecuteResult logic_result = prepare_next_function_call(contents_shunted_tokens, story_state, eval_result, story_state.variable_info);
	if (!logic_result.has_value() && logic_result.error().reason == ExpressionParserV2::NulloptResult::Reason::FoundKnotFunction) {
		return;
	}

	bool is_return = false;
	if (!contents_shunted_tokens.tokens.empty()) {
		const ExpressionParserV2::Token& first = contents_shunted_tokens.tokens[0];
		if (first.type == ExpressionParserV2::TokenType::Keyword && first.keyword_type == ExpressionParserV2::KeywordType::Return) {
			is_return = true;
			eval_result.reached_function_return = true;
		}
	}

	if (is_return) {
		if (logic_result.has_value()) {
			eval_result.return_value = *logic_result;
		} else {
			eval_result.return_value = std::nullopt;
		}
	}
}
