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
	for (ExpressionParser::Token* token : function_return_values) {
		delete token;
	}
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

ExpressionParser::ExecuteResult InkObject::prepare_next_function_call(ExpressionParser::ShuntedExpression& expression, InkStoryState& story_state, InkStoryEvalResult& eval_result, ExpressionParser::VariableMap& variables, const ExpressionParser::VariableMap& constants, ExpressionParser::RedirectMap& redirects) {
	//if (expression.preparation_finished) {
	//	return false;
	//}

	if (!story_state.current_knot().returning_from_function) {
		expression.push_entry();
	}

	ExpressionParser::ShuntedExpression::StackEntry& expression_entry = expression.stack_back();

	if (story_state.current_knot().returning_from_function) {
		if (eval_result.return_value.has_value()) {
			ExpressionParser::Token* value = ExpressionParser::variant_to_token(*eval_result.return_value);
			function_return_values.push_back(value);
			expression_entry.function_prepared_tokens[expression_entry.function_eval_index] = value;
		} else {
			expression_entry.function_prepared_tokens.erase(expression_entry.function_prepared_tokens.begin() + expression_entry.function_eval_index);
		}

		for (std::uint8_t i = 0; i < story_state.arguments_stack.back().size(); ++i) {
			expression_entry.function_prepared_tokens.erase(expression_entry.function_prepared_tokens.begin() + (expression_entry.function_eval_index - 1));
			--expression_entry.function_eval_index;
		}

		story_state.current_knot().returning_from_function = false;
	}

	ExpressionParser::ExecuteResult result = ExpressionParser::execute_expression_tokens(expression.stack_back().function_prepared_tokens, variables, constants, redirects, story_state.functions);
	if (result.has_value()) {
		//expression.preparation_finished = true;
		expression.pop_entry();
		return *result;
	} else if (result.error().reason == ExpressionParser::NulloptResult::Reason::NoReturnValue) {
		//expression.preparation_finished = true;
		expression.pop_entry();
		return std::unexpected(result.error());
	}

	/*if (!expression.preparation_finished) {
		if (expression.function_eval_index == SIZE_MAX) {
			expression.function_eval_index = 0;
		} else {
			//std::uint8_t arg_count = static_cast<ExpressionParser::TokenFunction*>(expression.function_prepared_tokens[expression.function_eval_index])->data.argument_count;
			if (eval_result.return_value.has_value()) {
				ExpressionParser::Token* value = ExpressionParser::variant_to_token(*eval_result.return_value);
				function_return_values.push_back(value);
				expression.function_prepared_tokens.back()[expression.function_eval_index] = value;
			} else {
				expression.function_prepared_tokens.erase(expression.function_prepared_tokens.begin() + expression.function_eval_index);
			}

			/*for (std::uint8_t i = 0; i < arg_count; ++i) {
				expression.function_prepared_tokens.erase(expression.function_prepared_tokens.begin() + (expression.function_eval_index - 1));
				--expression.function_eval_index;
			}

			for (std::uint8_t i = 0; i < story_state.arguments_stack.back().size(); ++i) {
				expression.function_prepared_tokens.back().erase(expression.function_prepared_tokens.back().begin() + (expression.function_eval_index - 1));
				--expression.function_eval_index;
			}

			++expression.function_eval_index;

			story_state.arguments_stack.pop_back();
		}
	}*/
	
	

	const ExpressionParser::NulloptResult& nullopt_result = result.error();
	if (nullopt_result.reason == ExpressionParser::NulloptResult::Reason::FoundKnotFunction) {
		story_state.arguments_stack.push_back({});
		if (nullopt_result.function->data.argument_count > 0) {
			std::vector<ExpressionParser::Variant>& args = story_state.arguments_stack.back();

			for (ExpressionParser::Token* token : nullopt_result.arguments) {
				std::optional<ExpressionParser::Variant> arg_value = token->get_variant_value(variables, constants, redirects);
				args.push_back(*arg_value);
			}
		}

		eval_result.target_knot = nullopt_result.function->data.name;
		eval_result.divert_type = DivertType::Function;
		expression_entry.function_eval_index = nullopt_result.function_index;

		return std::unexpected(nullopt_result);
	} else {
		throw std::runtime_error("Error while executing expression tokens");
	}

	

	
	
	
	/*(for (std::size_t i = expression.function_eval_index; i < expression.function_prepared_tokens.size(); ++i) {
		ExpressionParser::Token* this_token = expression.function_prepared_tokens[i];
		if (this_token->get_type() == ExpressionParser::TokenType::Function) {
			auto* token_func = static_cast<ExpressionParser::TokenFunction*>(this_token);
			if (token_func->data.fetch_method == ExpressionParser::TokenFunction::FetchMethod::StoryKnot) {
				eval_result.target_knot = token_func->data.name;
				eval_result.divert_type = DivertType::Function;
				expression.function_eval_index = i;
				//expression.function_eval_arg_count = token_func->data.argument_count;

				for (std::uint8_t a = token_func->data.argument_count; a > 0; --a) {
					std::optional<ExpressionParser::Variant> arg_value = expression.function_prepared_tokens[i - a]->get_variant_value(variables, constants, redirects);
					args.push_back(*arg_value);
				}

				return true;
			}
		} else {
			ExpressionParser::RedirectMap dummy;
			if (std::optional<ExpressionParser::Variant> value = this_token->get_variant_value(variables, constants, redirects); value.has_value()) {
				args.push_back(*value);
			}
		}
	}

	story_state.arguments_stack.pop_back();
	
	expression.preparation_finished = true;
	return false;*/
}
