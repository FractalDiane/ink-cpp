#include "expression_parser/expression_parser.h"

#include <cctype>
#include <unordered_set>
#include <cmath>
#include <stack>
#include <deque>
#include <functional>
#include <format>
#include <stdexcept>

using namespace ExpressionParserV2;

#define O OperatorType
#define C(i) static_cast<std::uint8_t>(i)

namespace {
	static const std::unordered_map<std::string, std::pair<OperatorType, OperatorUnaryType>> OperatorKeywords = {
		//{"temp", TokenKeyword::Type::Temp},
		//{"true", TokenKeyword::Type::True},
		//{"false", TokenKeyword::Type::False},
		{"and", {OperatorType::And, OperatorUnaryType::NotUnary}},
		{"or", {OperatorType::Or, OperatorUnaryType::NotUnary}},
		{"not", {OperatorType::Not, OperatorUnaryType::Prefix}},
		{"mod", {OperatorType::Modulus, OperatorUnaryType::NotUnary}},
	};

	static const std::unordered_map<OperatorType, std::uint8_t> OperatorPrecedences = {
		{O::Increment, C(2)},
		{O::Decrement, C(2)},
		{O::Negative, C(2)},
		{O::Not, C(2)},

		{O::Multiply, C(5)},
		{O::Divide, C(5)},
		{O::Modulus, C(5)},

		{O::Plus, C(6)},
		{O::Minus, C(6)},

		{O::ShiftLeft, C(7)},
		{O::ShiftRight, C(7)},

		{O::Less, C(9)},
		{O::Greater, C(9)},
		{O::LessEqual, C(9)},
		{O::GreaterEqual, C(9)},

		{O::Equal, C(10)},
		{O::NotEqual, C(10)},

		{O::BitAnd, C(11)},

		{O::BitXor, C(12)},

		{O::BitOr, C(13)},

		{O::And, C(14)},

		{O::Or, C(15)},

		{O::Assign, C(16)},
	};

	///////////////////////////////////////////////////////////////////////////////////////////////

	Variant builtin_pow(const std::vector<Variant>& args) {
		const Variant& base = args[0];
		const Variant& exponent = args[1];
		return std::pow(static_cast<double>(base), static_cast<double>(exponent));
	}

	Variant builtin_int(const std::vector<Variant>& args) {
		return static_cast<std::int64_t>(args[0]);
	}

	Variant builtin_float(const std::vector<Variant>& args) {
		return static_cast<double>(args[0]);
	}

	Variant builtin_floor(const std::vector<Variant>& args) {
		return std::floor(static_cast<double>(args[0]));
	}

	Variant builtin_ceil(const std::vector<Variant>& args) {
		return std::ceil(static_cast<double>(args[0]));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////

	static const std::unordered_map<std::string, std::pair<InkFunction, std::uint8_t>> BuiltinFunctions = {
		{"POW", {builtin_pow, (std::uint8_t)2}},
		{"INT", {builtin_int, (std::uint8_t)1}},
		{"FLOAT", {builtin_float, (std::uint8_t)1}},
		{"FLOOR", {builtin_floor, (std::uint8_t)1}},
		{"CEILING", {builtin_ceil, (std::uint8_t)1}},
	};

	///////////////////////////////////////////////////////////////////////////////////////////////

	char next_char(const std::string& string, std::size_t index) {
		return index + 1 < string.length() ? string[index + 1] : 0;
	}
}

#undef O
#undef C

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

void try_add_word(const std::string& expression, std::size_t index, std::vector<Token>& result, std::string& word, const StoryVariableInfo& story_var_info, bool in_knot_name) {
	if (!word.empty()) {
		bool found_result = false;
		if (auto keyword = OperatorKeywords.find(word); keyword != OperatorKeywords.end()) {
			result.push_back(Token::operat(keyword->second.first, keyword->second.second));
			found_result = true;
		} else if (word == "true") {
			result.push_back(Token::literal_bool(true));
			found_result = true;
		} else if (word == "false") {
			result.push_back(Token::literal_bool(false));
			found_result = true;
		} else if (auto builtin_func = BuiltinFunctions.find(word); builtin_func != BuiltinFunctions.end()) {
			result.push_back(Token::function_builtin(word, builtin_func->second.first, builtin_func->second.second));
			found_result = true;
		} else if (auto func = story_var_info.builtin_functions.find(word); func != story_var_info.builtin_functions.end()) {
			result.push_back(Token::function_builtin(word, func->second.first, func->second.second));
			found_result = true;
		} else if (story_var_info.external_functions.contains(word)) {
			result.push_back(Token::function_external(word));
			found_result = true;
		} else if (std::optional<Uuid> list_item_origin = story_var_info.get_list_entry_origin(word); list_item_origin.has_value()) {
			InkList new_list{story_var_info.defined_lists};
			new_list.add_item(word);
			result.push_back(Token::literal_list(new_list));
			found_result = true;
		} else if (word == "temp") {
			result.push_back(Token::keyword(KeywordType::Temp));
			found_result = true;
		} else if (word == "return") {
			result.push_back(Token::keyword(KeywordType::Return));
			found_result = true;
		} else {
			bool is_num = true;
			for (const char& chr : word) {
				if (chr != '.' && !std::isdigit(chr)) {
					is_num = false;
					break;
				}
			}

			if (is_num) {
				if (word.contains(".")) {
					try {
						double word_float = std::stod(word);
						Token token = Token::literal_float(word_float);
						result.push_back(token);
						found_result = true;
					} catch (...) {
						
					}
				} else {
					try {
						std::int64_t word_int = std::stoll(word);
						Token token = Token::literal_int(word_int);
						result.push_back(token);
						found_result = true;
					} catch (...) {
						
					}
				}
			}
		}

		if (!found_result) {
			if (index < expression.length() - 1 && expression[index] == '(') {
				if (story_var_info.defined_lists.contains_list_name(word)) {
					result.push_back(Token::function_list_subscript(word));
				} else {
					result.push_back(Token::function_story_knot(word));
				}
				
				found_result = true;
			}
		}
		

		if (!found_result) {
			if (in_knot_name) {
				Token token = Token::literal_knotname(word, true);
				result.push_back(token);
			} else {
				Token token = Token::variable(word);
				result.push_back(token);
			}
		}

		word.clear();
	}
}

void try_coalesce_list(const std::string& expression, std::vector<Token>& result, const StoryVariableInfo& story_var_info) {
	std::size_t i = result.size() - 1;
	std::vector<const Token*> list_entries;
	std::size_t end_index = 0;

	bool continue_loop = true;
	while (continue_loop) {
		--i;
		const Token& this_token = result[i];
		switch (this_token.type) {
			case TokenType::ParenComma: {
				switch (this_token.paren_comma_type) {
					case ParenCommaType::Comma: {

					} break;

					case ParenCommaType::LeftParen: {
						if (i == 0 || result[i - 1].type != TokenType::Function) {
							end_index = i;
							continue_loop = false;
						} else {
							return;
						}
					} break;

					case ParenCommaType::RightParen:
					default: {
						return;
					} break;
				}
			} break;

			case TokenType::LiteralList: {
				list_entries.push_back(&this_token);
			} break;

			default: {
				return;
			} break;
		}

		if (i == 0) {
			break;
		}
	}

	InkList new_list{story_var_info.defined_lists};
	for (const Token* token : list_entries) {
		new_list.add_item(static_cast<InkList>(token->value).single_item());
	}

	while (result.size() > end_index) {
		result.pop_back();
	}

	result.push_back(Token::literal_list(new_list));
}

}

std::vector<Token> ExpressionParserV2::tokenize_expression(const std::string& expression, ExpressionParserV2::StoryVariableInfo& story_variable_info) {
	std::vector<Token> result;
	result.reserve(128);

	std::string current_word;
	current_word.reserve(32);

	bool in_quotes = false;
	std::string current_string;
	current_string.reserve(32);

	bool in_knot_name = false;

	#define TRY_ADD_WORD() try_add_word(expression, index, result, current_word, story_variable_info, in_knot_name)

	std::size_t index = 0;
	while (index < expression.length()) {
		using UnaryType = OperatorUnaryType;
		char this_char = expression[index];
		if (!in_quotes) {
			switch (this_char) {
				case '+': {
					if (next_char(expression, index) == '+') {
						if (next_char(expression, index + 1) <= 32) {
							TRY_ADD_WORD();
							result.push_back(Token::operat(OperatorType::Increment, UnaryType::Postfix));
						} else {
							result.push_back(Token::operat(OperatorType::Increment, UnaryType::Prefix));
						}
						
						++index;
					} else {
						result.push_back(Token::operat(OperatorType::Plus, UnaryType::NotUnary));
					}
				} break;

				case '-': {
					if (next_char(expression, index) == '-') {
						if (next_char(expression, index + 1) <= 32) {
							TRY_ADD_WORD();
							result.push_back(Token::operat(OperatorType::Decrement, UnaryType::Postfix));
						} else {
							result.push_back(Token::operat(OperatorType::Decrement, UnaryType::Prefix));
						}
						
						++index;
					} else if (next_char(expression, index) == '>') {
						in_knot_name = true;
						++index;
					} else {
						if (next_char(expression, index) > 32 && (result.empty() || result.back().type == TokenType::Operator)) {
							result.push_back(Token::operat(OperatorType::Negative, UnaryType::Prefix));
						} else {
							result.push_back(Token::operat(OperatorType::Minus, UnaryType::NotUnary));
						}
					}
				} break;

				case '=': {
					TRY_ADD_WORD();
					if (next_char(expression, index) == '=') {
						result.push_back(Token::operat(OperatorType::Equal, UnaryType::NotUnary));
						++index;
					} else {
						result.push_back(Token::operat(OperatorType::Assign, UnaryType::NotUnary));
					}
				} break;

				case '<': {
					TRY_ADD_WORD();
					if (next_char(expression, index) == '=') {
						result.push_back(Token::operat(OperatorType::LessEqual, UnaryType::NotUnary));
						++index;
					} else {
						result.push_back(Token::operat(OperatorType::Less, UnaryType::NotUnary));
					}
				} break;
				
				case '>': {
					TRY_ADD_WORD();
					if (next_char(expression, index) == '=') {
						result.push_back(Token::operat(OperatorType::GreaterEqual, UnaryType::NotUnary));
						++index;
					} else {
						result.push_back(Token::operat(OperatorType::Greater, UnaryType::NotUnary));
					}
				} break;

				case '*': {
					TRY_ADD_WORD();
					result.push_back(Token::operat(OperatorType::Multiply, UnaryType::NotUnary));
				} break;

				case '/': {
					TRY_ADD_WORD();
					result.push_back(Token::operat(OperatorType::Divide, UnaryType::NotUnary));
				} break;

				case '%': {
					TRY_ADD_WORD();
					result.push_back(Token::operat(OperatorType::Modulus, UnaryType::NotUnary));
				} break;

				case '?': {
					TRY_ADD_WORD();
					result.push_back(Token::operat(OperatorType::Substring, UnaryType::NotUnary));
				} break;

				case '!': {
					if (next_char(expression, index) == '=') {
						result.push_back(Token::operat(OperatorType::NotEqual, UnaryType::NotUnary));
						++index;
					} else if (next_char(expression, index) > 32) {
						result.push_back(Token::operat(OperatorType::Not, UnaryType::Prefix));
					}
				} break;

				case '&': {
					TRY_ADD_WORD();
					if (next_char(expression, index) == '&') {
						result.push_back(Token::operat(OperatorType::And, UnaryType::NotUnary));
						++index;
					}
				} break;

				case '|': {
					TRY_ADD_WORD();
					if (next_char(expression, index) == '|') {
						result.push_back(Token::operat(OperatorType::Or, UnaryType::NotUnary));
						++index;
					}
				} break;

				case '^': {
					TRY_ADD_WORD();
					result.push_back(Token::operat(OperatorType::Intersect, UnaryType::NotUnary));
				} break;

				case '(': {
					TRY_ADD_WORD();
					result.push_back(Token::paren_comma(ParenCommaType::LeftParen));
				} break;

				case ')': {
					TRY_ADD_WORD();
					result.push_back(Token::paren_comma(ParenCommaType::RightParen));
					try_coalesce_list(expression, result, story_variable_info);
				} break;

				case ',': {
					TRY_ADD_WORD();
					result.push_back(Token::paren_comma(ParenCommaType::Comma));
				} break;

				case '"': {
					if (!in_quotes) {
						in_quotes = true;
					}
				} break;

				default: {
					if (this_char > 32) {
						current_word.push_back(this_char);
					} else {
						TRY_ADD_WORD();
					}
				} break;
			}
		} else {
			if (this_char != '"') {
				current_string.push_back(this_char);
			} else {
				Token token = Token::literal_string(current_string);
				result.push_back(token);
				current_string.clear();
				in_quotes = false;
			}
		}

		++index;
	}

	TRY_ADD_WORD();

	std::vector<std::pair<std::reference_wrapper<Token>, std::uint8_t>> arg_count_stack;
	for (Token& token : result) {
		switch (token.type) {
			case TokenType::Function: {
				if (!arg_count_stack.empty() && arg_count_stack.back().second == 0) {
					arg_count_stack.back().second = 1;
				}
				
				arg_count_stack.emplace_back(token, static_cast<std::uint8_t>(0));
			} break;

			case TokenType::ParenComma: {
				switch (token.paren_comma_type) {
					case ParenCommaType::Comma: {
						++arg_count_stack.back().second;
					} break;

					case ParenCommaType::RightParen: {
						if (!arg_count_stack.empty()) {
							arg_count_stack.back().first.get().function_argument_count = arg_count_stack.back().second;
							arg_count_stack.pop_back();
						}
					} break;

					default: break;
				}
			} break;

			default: {
				if (!arg_count_stack.empty() && arg_count_stack.back().second == 0) {
					arg_count_stack.back().second = 1;
				}
			} break;
		}
	}

	#undef TRY_ADD_WORD

	return result;
}

std::vector<Token> ExpressionParserV2::shunt(const std::vector<Token>& infix) {
	std::vector<std::reference_wrapper<const Token>> postfix;
	std::stack<std::reference_wrapper<const Token>> stack;

	std::size_t index = 0;
	while (index < infix.size()) {
		const Token& this_token = infix[index];

		switch (this_token.type) {
			case TokenType::LiteralBool:
			case TokenType::LiteralNumberInt:
			case TokenType::LiteralNumberFloat:
			case TokenType::LiteralString:
			case TokenType::LiteralKnotName:
			case TokenType::LiteralList:
			case TokenType::Variable: {
				postfix.push_back(this_token);
			} break;

			case TokenType::Operator: {
				OperatorUnaryType unary_type = this_token.operator_unary_type;
				if (unary_type != OperatorUnaryType::NotUnary) {
					if (unary_type == OperatorUnaryType::Postfix) {
						postfix.push_back(this_token);
					} else {
						stack.push(this_token);
					}
				} else {
					while (!stack.empty()) {
						const Token& next_op = stack.top();

						bool higher_precedence = false;
						if (next_op.type == TokenType::Operator) {
							OperatorType this_type = this_token.operator_type;
							OperatorType that_type = next_op.operator_type;
							higher_precedence = OperatorPrecedences.at(that_type) <= OperatorPrecedences.at(this_type);
						} else {
							break;
						}

						if (higher_precedence) {
							postfix.push_back(next_op);
							stack.pop();
						} else {
							break;
						}
					}

					stack.push(this_token);
				}
			} break;

			case TokenType::ParenComma: {
				switch (this_token.paren_comma_type) {
					case ParenCommaType::LeftParen: {
						stack.push(this_token);
					} break;

					case ParenCommaType::RightParen: {
						while (!stack.empty()) {
							const Token& next = stack.top();
							if (next.type == TokenType::ParenComma && next.paren_comma_type == ParenCommaType::LeftParen) {
								break;
							} else {
								stack.pop();
								postfix.push_back(next);
							}
						}

						if (stack.empty()) {
							throw;
						}

						stack.pop();
						if (!stack.empty()) {
							const Token& new_top = stack.top();
							if (new_top.type == TokenType::Function) {
								postfix.push_back(new_top);
								stack.pop();
							}
						}
					} break;

					case ParenCommaType::Comma:
					default: {
						while (!stack.empty()) {
							const Token& next = stack.top();
							if (next.type == TokenType::ParenComma) {
								if (next.paren_comma_type != ParenCommaType::LeftParen) {
									postfix.push_back(next);
									stack.pop();
								} else {
									break;
								}
							} else {
								throw;
							}
						}
					} break;
				}
			} break;

			case TokenType::Function: {
				stack.push(this_token);
			} break;
			
			case TokenType::Keyword: {
				postfix.push_back(this_token);
			} break;

			default: {

			} break;
		}

		++index;
	}

	while (!stack.empty()) {
		postfix.push_back(stack.top());
		stack.pop();
	}

	std::vector<Token> result;
	result.reserve(postfix.size());
	for (auto& token : postfix) {
		result.push_back(token);
	}

	return result;
}

#define OP_BIN(type, op) case OperatorType::type: {\
	const Token& right = stack.back();\
	const Token& left = stack[stack.size() - 2];\
	Token result = Token::from_variant(left.value op right.value);\
	stack.pop_back();\
	stack.pop_back();\
	stack.push_back(result);\
} break;

#define OP_BIN_F(type, func) case OperatorType::type: {\
	const Token& right = stack.back();\
	const Token& left = stack[stack.size() - 2];\
	Token result = Token::from_variant(left.value.operator_##func(right.value));\
	stack.pop_back();\
	stack.pop_back();\
	stack.push_back(result);\
} break;

#define OP_UN_PRE(type, op) case OperatorType::type: {\
	const Token& operand = stack.back();\
	Token result = Token::from_variant(op operand.value);\
	stack.pop_back();\
	stack.push_back(result);\
} break;

ExpressionParserV2::ExecuteResult ExpressionParserV2::execute_expression_tokens(std::vector<Token>& expression_tokens, StoryVariableInfo& story_variable_info) {
	std::vector<Token> stack;
	std::size_t index = 0;
	while (index < expression_tokens.size()) {
		Token& this_token = expression_tokens[index];

		switch (this_token.type) {
			case TokenType::Variable: {
				this_token.fetch_variable_value(story_variable_info);
				[[fallthrough]];
			}
			case TokenType::LiteralBool:
			case TokenType::LiteralNumberInt:
			case TokenType::LiteralNumberFloat:
			case TokenType::LiteralString:
			case TokenType::LiteralKnotName:
			case TokenType::LiteralList: {
				stack.push_back(this_token);
			} break;

			case TokenType::Operator: {
				switch (this_token.operator_type) {
					OP_BIN(Plus, +);
					OP_BIN(Minus, -);
					OP_BIN(Multiply, *);
					OP_BIN(Divide, /);
					OP_BIN(Modulus, %);

					OP_BIN(Equal, ==);
					OP_BIN(NotEqual, !=);
					OP_BIN(Less, <);
					OP_BIN(Greater, >);
					OP_BIN(LessEqual, <=);
					OP_BIN(GreaterEqual, >=);

					OP_BIN_F(Substring, contains);

					OP_BIN(And, &&);
					OP_BIN(Or, ||);

					OP_UN_PRE(Negative, -);
					OP_UN_PRE(Not, !);

					case OperatorType::Increment:
					case OperatorType::Decrement: {
						Token& operand = stack.back();

						bool postfix = this_token.operator_unary_type == OperatorUnaryType::Postfix;
						if (this_token.operator_type == OperatorType::Increment) {
							operand.increment(postfix, story_variable_info);
						} else {
							operand.decrement(postfix, story_variable_info);
						}

						stack.pop_back();
					} break;

					case OperatorType::Assign: {
						Token& value = stack.back();
						Token& var = stack[stack.size() - 2];
						if (var.type != TokenType::Variable) {
							throw std::runtime_error("Invalid use of assignment operator");
						}

						std::string var_name = var.variable_name;
						if (value.type == TokenType::Variable && var.variable_name == value.variable_name) {
							break;
						}

						var.assign_variable(value, story_variable_info);

						stack.pop_back();
						stack.pop_back();
					} break;

					default: {

					} break;
				}
			} break;

			case TokenType::Function: {
				this_token.fetch_function_value(story_variable_info);

				std::vector<Token> func_args;
				for (std::uint8_t i = 0; i < this_token.function_argument_count; ++i) {
					func_args.push_back(stack[stack.size() - (this_token.function_argument_count - i)]);
				}

				if (this_token.function_fetch_type == FunctionFetchType::StoryKnot) {
					return std::unexpected(NulloptResult(NulloptResult::Reason::FoundKnotFunction, this_token, index, func_args));
				}

				std::vector<Variant> arg_values;
				if (this_token.function_argument_count > 0) {
					for (const Token& token : func_args) {
						arg_values.push_back(token.value);
					}

					for (std::uint8_t i = 0; i < this_token.function_argument_count; ++i) {
						stack.pop_back();
					}
				}
				
				if (auto builtin_func = BuiltinFunctions.find(this_token.value); builtin_func != BuiltinFunctions.end()) {
					if (Variant result = (builtin_func->second.first)(arg_values); result.has_value()) {
						stack.push_back(Token::from_variant(result));
					}
				} else if (Variant result = this_token.call_function(arg_values, story_variable_info); result.has_value()) {
					stack.push_back(Token::from_variant(result));
				}
			} break;

			default: {

			} break;
		}

		++index;
	}

	if (!stack.empty() && stack.back().value.has_value()) {
		return stack.back().value;
	}

	return std::unexpected(NulloptResult(NulloptResult::Reason::NoReturnValue));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ExpressionParserV2::ExecuteResult ExpressionParserV2::execute_expression(const std::string& expression, StoryVariableInfo& story_variable_info) {
	std::vector<Token> tokenized = ExpressionParserV2::tokenize_expression(expression, story_variable_info);
	std::vector<Token> shunted = ExpressionParserV2::shunt(tokenized);

	ExecuteResult result = ExpressionParserV2::execute_expression_tokens(shunted, story_variable_info);

	return result;
}

ShuntedExpression ExpressionParserV2::tokenize_and_shunt_expression(const std::string& expression, StoryVariableInfo& story_variable_info) {
	std::vector<Token> tokenized = ExpressionParserV2::tokenize_expression(expression, story_variable_info);
	std::vector<Token> shunted = ExpressionParserV2::shunt(tokenized);

	return ShuntedExpression(shunted);
}
