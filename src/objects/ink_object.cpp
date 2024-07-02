#include "objects/ink_object.h"

ByteVec Serializer<InkObject*>::operator()(const InkObject* value) {
	return value->get_serialized_bytes();
}

InkObject* Deserializer<InkObject*>::operator()(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::uint8_t> ds8;

	ObjectId id = static_cast<ObjectId>(ds8(bytes, index));
	InkObject* result = InkObject::create_from_id(id);
	return result->populate_from_bytes(bytes, index);
}

std::string InkObject::to_string() const {
	return "NO TO_STRING";
}

std::vector<std::uint8_t> InkObject::to_bytes() const {
	return {};
}

InkObject::~InkObject() {
	/*for (ExpressionParser::Token* token : function_return_values) {
		delete token;
	}*/
}

InkObject* InkObject::populate_from_bytes(const std::vector<std::uint8_t>& bytes, std::size_t& index) {
	return this;
}

ByteVec InkObject::get_serialized_bytes() const {
	ByteVec result = {static_cast<std::uint8_t>(get_id())};
	ByteVec serialization = to_bytes();
	result.insert(result.end(), serialization.begin(), serialization.end());
	return result;
}

#include "objects/ink_object_choice.h"
#include "objects/ink_object_choicetextmix.h"
#include "objects/ink_object_conditional.h"
#include "objects/ink_object_divert.h"
#include "objects/ink_object_globalvariable.h"
#include "objects/ink_object_glue.h"
#include "objects/ink_object_interpolation.h"
#include "objects/ink_object_linebreak.h"
#include "objects/ink_object_logic.h"
#include "objects/ink_object_sequence.h"
#include "objects/ink_object_tag.h"
#include "objects/ink_object_text.h"

InkObject* InkObject::create_from_id(ObjectId id) {
	switch (id) {
		case ObjectId::Text: {
			return new InkObjectText();
		} break;

		case ObjectId::Choice: {
			return new InkObjectChoice({});
		} break;

		case ObjectId::LineBreak: {
			return new InkObjectLineBreak();
		} break;

		case ObjectId::Glue: {
			return new InkObjectGlue();
		} break;

		case ObjectId::Divert: {
			return new InkObjectDivert({}, {}, DivertType::ToKnot);
		} break;

		case ObjectId::Interpolation: {
			return new InkObjectInterpolation({});
		} break;

		case ObjectId::Conditional: {
			return new InkObjectConditional({}, {});
		} break;

		case ObjectId::Sequence: {
			return new InkObjectSequence(InkSequenceType::Sequence, false, {});
		} break;

		case ObjectId::ChoiceTextMix: {
			return new InkObjectChoiceTextMix(false);
		} break;

		case ObjectId::Tag: {
			return new InkObjectTag("");
		} break;

		case ObjectId::GlobalVariable: {
			return new InkObjectGlobalVariable("", false, {});
		} break;

		case ObjectId::Logic: {
			return new InkObjectLogic({});
		} break;

		default: {
			throw std::runtime_error(std::format("Tried to create an inkb object with an unknown object ID ({})", static_cast<std::uint8_t>(id)));
		} break;
	}
}

ExpressionParserV2::ExecuteResult InkObject::prepare_next_function_call(ExpressionParserV2::ShuntedExpression& expression, InkStoryState& story_state, InkStoryEvalResult& eval_result, ExpressionParserV2::StoryVariableInfo& story_variable_info) {
	bool continuing_preparation = story_state.current_knot().returning_from_function && story_state.current_knot().current_function_prep_expression == expression.uuid;
	if (!continuing_preparation) {
		expression.push_entry();
	}

	ExpressionParserV2::ShuntedExpression::StackEntry& expression_entry = expression.stack_back();

	if (continuing_preparation) {
		if (eval_result.return_value.has_value()) {
			ExpressionParserV2::Variant value = *eval_result.return_value;
			function_return_values.push_back(value);
			expression_entry.function_prepared_tokens[expression_entry.function_eval_index] = ExpressionParserV2::Token::from_variant(value);
		} else {
			expression_entry.function_prepared_tokens.erase(expression_entry.function_prepared_tokens.begin() + expression_entry.function_eval_index);
		}

		--expression_entry.function_eval_index;

		// thanks Ryan
		std::size_t args_expected = expression_entry.argument_count;
		while (args_expected > 0 && !expression_entry.function_prepared_tokens.empty()) {
			ExpressionParserV2::Token& this_token = expression_entry.function_prepared_tokens[expression_entry.function_eval_index];
			if (this_token.type == ExpressionParserV2::TokenType::Operator || this_token.type == ExpressionParserV2::TokenType::Function) {
				++args_expected;
			} else {
				--args_expected;
			}

			expression_entry.function_prepared_tokens.erase(expression_entry.function_prepared_tokens.begin() + expression_entry.function_eval_index);
			--expression_entry.function_eval_index;
		}

		story_state.current_knot().returning_from_function = false;
		story_state.current_knot().current_function_prep_expression = UINT32_MAX;
	}

	ExpressionParserV2::ExecuteResult result = ExpressionParserV2::execute_expression_tokens(expression.stack_back().function_prepared_tokens, story_variable_info);
	if (result.has_value()) {
		expression.pop_entry();
		return *result;
	} else if (result.error().reason == ExpressionParserV2::NulloptResult::Reason::NoReturnValue) {
		expression.pop_entry();
		return std::unexpected(result.error());
	}

	const ExpressionParserV2::NulloptResult& nullopt_result = result.error();
	if (nullopt_result.reason == ExpressionParserV2::NulloptResult::Reason::FoundKnotFunction) {
		story_state.arguments_stack.push_back({});
		expression_entry.argument_count = nullopt_result.function.function_argument_count;
		if (nullopt_result.function.function_argument_count > 0) {
			std::vector<std::pair<std::string, ExpressionParserV2::Variant>>& args = story_state.arguments_stack.back();

			for (const ExpressionParserV2::Token& token : nullopt_result.arguments) {
				std::pair<std::string, ExpressionParserV2::Variant> arg;
				arg.second = token.value;
				if (token.type == ExpressionParserV2::TokenType::Variable) {
					arg.first = token.variable_name;
				}

				args.push_back(arg);
			}
		}

		eval_result.target_knot = static_cast<std::string>(nullopt_result.function.value);
		eval_result.divert_type = DivertType::Function;
		eval_result.imminent_function_prep = true;
		story_state.current_knot().current_function_prep_expression = expression.uuid;
		expression_entry.function_eval_index = nullopt_result.function_index;

		return std::unexpected(nullopt_result);
	} else {
		throw std::runtime_error("Error while executing expression tokens");
	}
}
