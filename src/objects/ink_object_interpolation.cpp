#include "objects/ink_object_interpolation.h"

#include "ink_utils.h"

#include "expression_parser/expression_parser.h"

ByteVec InkObjectInterpolation::to_bytes() const {
	VectorSerializer<ExpressionParserV2::Token> s;
	return s(what_to_interpolate.tokens);
}

InkObject* InkObjectInterpolation::populate_from_bytes(const ByteVec& bytes, std::size_t& index) {
	VectorDeserializer<ExpressionParserV2::Token> ds;
	what_to_interpolate = ExpressionParserV2::ShuntedExpression(ds(bytes, index));
	return this;
}

InkObjectInterpolation::~InkObjectInterpolation() {
	
}

void InkObjectInterpolation::execute(InkStoryState& story_state, InkStoryEvalResult& eval_result) {
	if ((!story_state.selected_choice.has_value() && story_state.choice_mix_position != InkStoryState::ChoiceMixPosition::After)
	|| (story_state.selected_choice.has_value() && story_state.choice_mix_position != InkStoryState::ChoiceMixPosition::In)) {
		ExpressionParserV2::ExecuteResult interpolate_result = prepare_next_function_call(what_to_interpolate, story_state, eval_result, story_state.variable_info);
		if (!interpolate_result.has_value() && interpolate_result.error().reason == ExpressionParserV2::NulloptResult::Reason::FoundKnotFunction) {
			return;
		}

		if (interpolate_result.has_value()) {
			std::string result = interpolate_result->to_printable_string();
			
			// if we're preparing an interpolate in the text of a choice, content is treated as a return value
			// in EVERY OTHER CASE, it is simply run as a side effect
			if (story_state.current_knot().knot->function_prep_type != FunctionPrepType::ChoiceTextInterpolate) {
				eval_result.result += result;
				story_state.current_knot().any_new_content = !result.empty();
			} else {
				if (eval_result.return_value.has_value()) {
					*eval_result.return_value += result;
				} else {
					eval_result.return_value = result;
				}
			}
		}
	}
}
