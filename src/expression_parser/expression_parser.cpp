#include "expression_parser/expression_parser.h"

#include <cctype>
#include <stack>
#include <unordered_set>
#include <cmath>

using namespace ExpressionParser;

#define O TokenOperator::Type
#define C(i) static_cast<std::uint8_t>(i)

namespace {
	static const std::unordered_map<std::string, TokenKeyword::Type> Keywords = {
		{"temp", TokenKeyword::Type::Temp},
		{"true", TokenKeyword::Type::True},
		{"false", TokenKeyword::Type::False},
		{"and", TokenKeyword::Type::And},
		{"or", TokenKeyword::Type::Or},
		{"not", TokenKeyword::Type::Not},
		{"mod", TokenKeyword::Type::Mod},
	};

	static const std::unordered_map<TokenOperator::Type, std::uint8_t> OperatorPrecedences = {
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

	char next_char(const std::string& string, std::size_t index) {
		return index + 1 < string.length() ? string[index + 1] : 0;
	}

	Token* next_token(const std::vector<Token*>& tokens, std::size_t index) {
		return index + 1 < tokens.size() ? tokens[index + 1] : nullptr;
	}
}

#undef O
#undef C

Token* Token::operator_plus(const Token* other) const { throw; }
Token* Token::operator_minus(const Token* other) const { throw; }
Token* Token::operator_multiply(const Token* other) const { throw; }
Token* Token::operator_divide(const Token* other) const { throw; }
Token* Token::operator_mod(const Token* other) const { throw; }

Token* Token::operator_inc_pre() { throw; }
Token* Token::operator_inc_post() { throw; }
Token* Token::operator_dec_pre() { throw; }
Token* Token::operator_dec_post() { throw; }
Token* Token::operator_not() const { throw; }
Token* Token::operator_negative() const { throw; }

Token* Token::operator_equal(const Token* other) const { throw; }
Token* Token::operator_notequal(const Token* other) const { throw; }
Token* Token::operator_less(const Token* other) const { throw; }
Token* Token::operator_greater(const Token* other) const { throw; }
Token* Token::operator_lessequal(const Token* other) const { throw; }
Token* Token::operator_greaterequal(const Token* other) const { throw; }

Token* Token::operator_substring(const Token* other) const { throw; }

#define OP_MATH_INT(_class, name, op) Token* _class::operator_##name(const Token* other) const {\
		switch (other->get_type()) {\
			case TokenType::NumberInt: {\
				return new TokenNumberInt(data op static_cast<const TokenNumberInt*>(other)->data);\
			} break;\
\
			case TokenType::NumberFloat: {\
				return new TokenNumberFloat(static_cast<double>(data) op static_cast<const TokenNumberFloat*>(other)->data);\
			} break;\
\
			default: {\
				throw;\
			} break;\
		}\
	}

#define OP_MATH_FLOAT(_class, name, op) Token* _class::operator_##name(const Token* other) const {\
		switch (other->get_type()) {\
			case TokenType::NumberFloat: {\
				return new TokenNumberFloat(data op static_cast<const TokenNumberFloat*>(other)->data);\
			} break;\
\
			case TokenType::NumberInt: {\
				return new TokenNumberFloat(data op static_cast<double>(static_cast<const TokenNumberInt*>(other)->data));\
			} break;\
\
			default: {\
				throw;\
			} break;\
		}\
	}

#define OP_CMP_INT(_class, name, op) Token* _class::operator_##name(const Token* other) const {\
		switch (other->get_type()) {\
			case TokenType::NumberInt: {\
				return new TokenBoolean(data op static_cast<const TokenNumberInt*>(other)->data);\
			} break;\
\
			case TokenType::NumberFloat: {\
				return new TokenBoolean(static_cast<double>(data) op static_cast<const TokenNumberFloat*>(other)->data);\
			} break;\
\
			default: {\
				return new TokenBoolean(false);\
			} break;\
		}\
	}

#define OP_CMP_FLOAT(_class, name, op) Token* _class::operator_##name(const Token* other) const {\
		switch (other->get_type()) {\
			case TokenType::NumberFloat: {\
				return new TokenBoolean(data op static_cast<const TokenNumberFloat*>(other)->data);\
			} break;\
\
			case TokenType::NumberInt: {\
				return new TokenBoolean(data op static_cast<double>(static_cast<const TokenNumberInt*>(other)->data));\
			} break;\
\
			default: {\
				return new TokenBoolean(false);\
			} break;\
		}\
	}

#define OP_INC_PRE(_class) Token* _class::operator_inc_pre() {\
		++data;\
		return this;\
	}

#define OP_INC_POST(_class) Token* _class::operator_inc_post() {\
		Token* temp = new _class(data);\
		data++;\
		return temp;\
	}

#define OP_DEC_PRE(_class) Token* _class::operator_dec_pre() {\
		--data;\
		return this;\
	}

#define OP_DEC_POST(_class) Token* _class::operator_dec_post() {\
		Token* temp = new _class(data);\
		data--;\
		return temp;\
	}

///////////////////////////////////////////////////////////////////////////////////////////////////

Token* TokenBoolean::operator_not() const {
	return new TokenBoolean(!data);
}

Token* TokenBoolean::operator_equal(const Token* other) const {
	if (other->get_type() == TokenType::Boolean) {
		return new TokenBoolean(data == static_cast<const TokenBoolean*>(other)->data);
	} else {
		throw;
	}
}

Token* TokenBoolean::operator_notequal(const Token* other) const {
	if (other->get_type() == TokenType::Boolean) {
		return new TokenBoolean(data != static_cast<const TokenBoolean*>(other)->data);
	} else {
		throw;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

OP_MATH_INT(TokenNumberInt, plus, +);
OP_MATH_INT(TokenNumberInt, minus, -);
OP_MATH_INT(TokenNumberInt, multiply, *);
OP_MATH_INT(TokenNumberInt, divide, /);

Token* TokenNumberInt::operator_mod(const Token* other) const {
	switch (other->get_type()) {
		case TokenType::NumberInt: {
			return new TokenNumberInt(data % static_cast<const TokenNumberInt*>(other)->data);
		} break;

		case TokenType::NumberFloat: {
			return new TokenNumberFloat(std::fmod(static_cast<double>(data), static_cast<const TokenNumberFloat*>(other)->data));
		} break;

		default: {
			throw;
		} break;
	}
}

OP_INC_PRE(TokenNumberInt);
OP_INC_POST(TokenNumberInt);
OP_DEC_PRE(TokenNumberInt);
OP_DEC_POST(TokenNumberInt);

Token* TokenNumberInt::operator_negative() const {
	return new TokenNumberInt(-data);
}

OP_CMP_INT(TokenNumberInt, equal, ==);
OP_CMP_INT(TokenNumberInt, notequal, !=);
OP_CMP_INT(TokenNumberInt, less, <);
OP_CMP_INT(TokenNumberInt, greater, >);
OP_CMP_INT(TokenNumberInt, lessequal, <=);
OP_CMP_INT(TokenNumberInt, greaterequal, >=);

///////////////////////////////////////////////////////////////////////////////////////////////////

OP_MATH_FLOAT(TokenNumberFloat, plus, +);
OP_MATH_FLOAT(TokenNumberFloat, minus, -);
OP_MATH_FLOAT(TokenNumberFloat, multiply, *);
OP_MATH_FLOAT(TokenNumberFloat, divide, /);

Token* TokenNumberFloat::operator_mod(const Token* other) const {
	switch (other->get_type()) {
		case TokenType::NumberInt: {
			return new TokenNumberFloat(std::fmod(data, static_cast<double>(static_cast<const TokenNumberInt*>(other)->data)));
		} break;

		case TokenType::NumberFloat: {
			return new TokenNumberFloat(std::fmod(data, static_cast<const TokenNumberFloat*>(other)->data));
		} break;

		default: {
			throw;
		} break;
	}
}

OP_INC_PRE(TokenNumberFloat);
OP_INC_POST(TokenNumberFloat);
OP_DEC_PRE(TokenNumberFloat);
OP_DEC_POST(TokenNumberFloat);

Token* TokenNumberFloat::operator_negative() const {
	return new TokenNumberFloat(-data);
}

OP_CMP_FLOAT(TokenNumberFloat, equal, ==);
OP_CMP_FLOAT(TokenNumberFloat, notequal, !=);
OP_CMP_FLOAT(TokenNumberFloat, less, <);
OP_CMP_FLOAT(TokenNumberFloat, greater, >);
OP_CMP_FLOAT(TokenNumberFloat, lessequal, <=);
OP_CMP_FLOAT(TokenNumberFloat, greaterequal, >=);

///////////////////////////////////////////////////////////////////////////////////////////////////

Token* TokenStringLiteral::operator_plus(const Token* other) const {
	if (other->get_type() == TokenType::StringLiteral) {
		return new TokenStringLiteral(data + static_cast<const TokenStringLiteral*>(other)->data);
	} else {
		throw;
	}
}

Token* TokenStringLiteral::operator_substring(const Token* other) const {
	if (other->get_type() == TokenType::StringLiteral) {
		return new TokenBoolean(data.contains(static_cast<const TokenStringLiteral*>(other)->data));
	} else {
		throw;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// ~ temp x = 5 + 7
// Keyword(Temp) Variable("x") Operator(=) Number(5) Operator(+) Number(7)
std::vector<Token*> ExpressionParser::tokenize_expression(const std::string& expression) {
	std::vector<Token*> result;
	result.reserve(128);

	std::string current_word;
	current_word.reserve(32);

	bool in_quotes = false;
	std::string current_string;
	current_string.reserve(32);

	std::size_t index = 0;
	while (index < expression.length()) {
		char this_char = expression[index];
		if (!in_quotes) {
			switch (this_char) {
				case '+': {
					if (next_char(expression, index) == '+') {
						result.push_back(new TokenOperator(TokenOperator::Type::Increment, true));
						++index;
					} else {
						result.push_back(new TokenOperator(TokenOperator::Type::Plus, false));
					}
				} break;

				case '-': {
					if (next_char(expression, index) == '-') {
						result.push_back(new TokenOperator(TokenOperator::Type::Decrement, true));
						++index;
					} else {
						result.push_back(new TokenOperator(TokenOperator::Type::Minus, false));
					}
				} break;

				case '=': {
					if (next_char(expression, index) == '=') {
						result.push_back(new TokenOperator(TokenOperator::Type::Equal, false));
						++index;
					} else {
						result.push_back(new TokenOperator(TokenOperator::Type::Assign, false));
					}
				} break;

				case '<': {
					if (next_char(expression, index) == '=') {
						result.push_back(new TokenOperator(TokenOperator::Type::LessEqual, false));
						++index;
					} else {
						result.push_back(new TokenOperator(TokenOperator::Type::Less, false));
					}
				} break;
				
				case '>': {
					if (next_char(expression, index) == '=') {
						result.push_back(new TokenOperator(TokenOperator::Type::GreaterEqual, false));
						++index;
					} else {
						result.push_back(new TokenOperator(TokenOperator::Type::Greater, false));
					}
				} break;

				case '*': {
					result.push_back(new TokenOperator(TokenOperator::Type::Multiply, false));
				} break;

				case '/': {
					result.push_back(new TokenOperator(TokenOperator::Type::Divide, false));
				} break;

				case '%': {
					result.push_back(new TokenOperator(TokenOperator::Type::Modulus, false));
				} break;

				case '?': {
					result.push_back(new TokenOperator(TokenOperator::Type::Substring, false));
				} break;

				case '!': {
					if (next_char(expression, index) == '=') {
						result.push_back(new TokenOperator(TokenOperator::Type::NotEqual, false));
						++index;
					} else {
						result.push_back(new TokenOperator(TokenOperator::Type::Not, false));
					}
				} break;

				case '"': {
					if (!in_quotes) {
						in_quotes = true;
					}
					/*} else {
						result.push_back(new TokenStringLiteral(current_string));
						current_string.clear();
						in_quotes = false;
					}*/
				} break;

				default: {
					if (this_char > 32) {
						current_word.push_back(this_char);
					} else if (!current_word.empty()) {
						if (auto keyword = Keywords.find(current_word); keyword != Keywords.end()) {
							result.push_back(new TokenKeyword(keyword->second));
						} else {
							if (current_word.contains(".")) {
								try {
									double word_float = std::stod(current_word);
									result.push_back(new TokenNumberFloat(word_float));
								} catch (...) {
									result.push_back(new TokenVariable(current_word));
								}
							} else {
								try {
									std::int64_t word_int = std::stoll(current_word);
									result.push_back(new TokenNumberInt(word_int));
								} catch (...) {
									result.push_back(new TokenVariable(current_word));
								}
							}
						}
						
						current_word.clear();
					}
				} break;
			}
		} else {
			if (this_char != '"') {
				current_string.push_back(this_char);
			} else {
				result.push_back(new TokenStringLiteral(current_string));
				current_string.clear();
				in_quotes = false;
			}
		}

		++index;
	}

	if (!current_word.empty()) {
		// TODO: dry it
		if (auto keyword = Keywords.find(current_word); keyword != Keywords.end()) {
			result.push_back(new TokenKeyword(keyword->second));
		} else {
			if (current_word.contains(".")) {
				try {
					double word_float = std::stod(current_word);
					result.push_back(new TokenNumberFloat(word_float));
				} catch (...) {
					result.push_back(new TokenVariable(current_word));
				}
			} else {
				try {
					std::int64_t word_int = std::stoll(current_word);
					result.push_back(new TokenNumberInt(word_int));
				} catch (...) {
					result.push_back(new TokenVariable(current_word));
				}
			}
		}
		
		current_word.clear();
	}

	return result;
}

std::vector<Token*> ExpressionParser::shunt(const std::vector<Token*>& infix) {
	std::vector<Token*> postfix;
	std::stack<Token*> stack;

	std::size_t index = 0;
	while (index < infix.size()) {
		Token* this_token = infix[index];

		switch (this_token->get_type()) {
			case TokenType::NumberInt:
			case TokenType::NumberFloat:
			case TokenType::StringLiteral:
			case TokenType::Variable: {
				postfix.push_back(this_token);
			} break;

			case TokenType::Operator: {
				bool unary = static_cast<TokenOperator*>(this_token)->data.unary;
				if (unary) {
					if (Token* next = next_token(infix, index); next && next->get_type() == TokenType::Variable) {
						stack.push(this_token);
					} else if (index > 0 && infix[index - 1]->get_type() == TokenType::Variable) {
						postfix.push_back(this_token);
					} else {
						throw;
					}
				} else {
					while (!stack.empty()) {
						Token* next_op = stack.top();

						bool higher_precedence = false;
						if (next_op->get_type() == TokenType::Function) {
							higher_precedence = true;
						} else {
							TokenOperator::Type this_type = static_cast<TokenOperator*>(this_token)->data.type;
							TokenOperator::Type that_type = static_cast<TokenOperator*>(next_op)->data.type;
							higher_precedence = OperatorPrecedences.at(that_type) <= OperatorPrecedences.at(this_type);
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

			case TokenType::Function: {
				stack.push(this_token);
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

	return postfix;
}

#define OP_BIN(type, func) case Type::type: {\
		Token* right = stack.top();\
		stack.pop();\
		Token* left = stack.top();\
		stack.pop();\
\
		Token* result = left->operator_##func(right);\
		tokens_to_dealloc.insert(result);\
		stack.push(result);\
	} break;

Token* ExpressionParser::execute_expression_tokens(const std::vector<Token*>& expression_tokens, TokenMap& variables) {
	std::stack<Token*> stack;
	std::unordered_set<Token*> tokens_to_dealloc;

	std::size_t index = 0;
	while (index < expression_tokens.size()) {
		Token* this_token = expression_tokens[index];

		switch (this_token->get_type()) {
			case TokenType::NumberInt:
			case TokenType::NumberFloat:
			case TokenType::StringLiteral:
			case TokenType::Variable: {
				stack.push(this_token);
			} break;

			case TokenType::Operator: {
				auto* op = static_cast<TokenOperator*>(this_token);
				using Type = TokenOperator::Type;
				switch (op->data.type) {
					OP_BIN(Plus, plus);
					OP_BIN(Minus, minus);
					OP_BIN(Multiply, multiply);
					OP_BIN(Divide, divide);
					OP_BIN(Modulus, mod);

					OP_BIN(Equal, equal);
					OP_BIN(NotEqual, notequal);
					OP_BIN(Less, less);
					OP_BIN(Greater, greater);
					OP_BIN(LessEqual, lessequal);
					OP_BIN(GreaterEqual, greaterequal);

					case Type::Assign: {
						Token* value = stack.top();
						stack.pop();
						Token* var = stack.top();
						stack.pop();
						if (var->get_type() != TokenType::Variable) {
							throw;
						}

						variables[static_cast<TokenVariable*>(var)->data] = value;
						tokens_to_dealloc.erase(value);
					} break;

					default: {

					} break;
				}
			} break;

			default: {

			} break;
		}

		++index;
	}

	Token* result = nullptr;
	if (!stack.empty()) {
		result = stack.top();
		tokens_to_dealloc.erase(result);
	}

	for (Token* token : tokens_to_dealloc) {
		delete token;
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

PackedToken ExpressionParser::execute_expression(const std::string& expression) {
	std::vector<Token*> tokenized = tokenize_expression(expression);
	std::vector<Token*> shunted = shunt(tokenized);

	TokenMap no_vars;
	Token* result = execute_expression_tokens(shunted, no_vars);
	return PackedToken(result);
}

PackedToken ExpressionParser::execute_expression(const std::string& expression, TokenMap& variables) {
	std::vector<Token*> tokenized = tokenize_expression(expression);
	std::vector<Token*> shunted = shunt(tokenized);

	Token* result = execute_expression_tokens(shunted, variables);
	return PackedToken(result);
}
