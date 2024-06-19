#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <functional>
#include <variant>
#include <optional>
#include <expected>
#include <algorithm>
#include <initializer_list>

#include "expression_parser/token.h"

#include "serialization.h"
#include "uuid.h"

namespace ExpressionParserV2 {
	struct ShuntedExpression {
	Uuid uuid;
	std::vector<ExpressionParserV2::Token> tokens;

	struct StackEntry {
		std::vector<ExpressionParserV2::Token> function_prepared_tokens;
		std::size_t function_eval_index = SIZE_MAX;
		std::size_t argument_count = 0;
		bool preparation_finished = false;

		//std::unordered_set<Token*> tokens_to_dealloc;
	};

	std::vector<StackEntry> preparation_stack;

	ShuntedExpression() : tokens{}, preparation_stack{}, uuid{0} {}
	explicit ShuntedExpression(const std::vector<ExpressionParserV2::Token>& tokens) : tokens{tokens}, preparation_stack{}, uuid{0} {}
	explicit ShuntedExpression(std::vector<ExpressionParserV2::Token>&& tokens) : tokens{tokens}, preparation_stack{}, uuid{0} {}

	void push_entry() {
		StackEntry new_entry;
		new_entry.function_prepared_tokens = tokens;
		preparation_stack.push_back(new_entry);
	}

	void pop_entry() {
		preparation_stack.pop_back();
	}

	StackEntry& stack_back() {
		return preparation_stack.back();
	}

	bool stack_empty() const { 
		return preparation_stack.empty();
	}
};

std::vector<ExpressionParserV2::Token> tokenize_expression(const std::string& expression, ExpressionParserV2::StoryVariableInfo& story_variable_info);

std::vector<ExpressionParserV2::Token> shunt(const std::vector<ExpressionParserV2::Token>& infix);

struct NulloptResult {
	enum class Reason {
		NoReturnValue,
		FoundKnotFunction,
		Failed,
	} reason;

	ExpressionParserV2::Token function;
	std::size_t function_index;
	std::vector<ExpressionParserV2::Token> arguments;

	NulloptResult(Reason reason) : reason{reason}, function{}, function_index{0}, arguments{} {}
	NulloptResult(Reason reason, const ExpressionParserV2::Token& function, std::size_t function_index, const std::vector<ExpressionParserV2::Token>& arguments) : reason{reason}, function{function}, function_index{function_index}, arguments{arguments} {}
};


typedef std::expected<Variant, NulloptResult> ExecuteResult;

ExpressionParserV2::ExecuteResult execute_expression_tokens(std::vector<ExpressionParserV2::Token>& tokens, ExpressionParserV2::StoryVariableInfo& story_variable_info);
ExpressionParserV2::ExecuteResult execute_expression(const std::string& expression, ExpressionParserV2::StoryVariableInfo& story_variable_info);
ShuntedExpression tokenize_and_shunt_expression(const std::string& expression, ExpressionParserV2::StoryVariableInfo& story_variable_info);

}
