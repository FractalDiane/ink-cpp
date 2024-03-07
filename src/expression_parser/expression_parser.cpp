#include "expression_parser/expression_parser.h"

#include <cctype>
#include <unordered_set>
#include <cmath>
#include <stack>

using namespace ExpressionParser;

#define O TokenOperator::Type
#define C(i) static_cast<std::uint8_t>(i)

namespace {
	static const std::unordered_map<std::string, TokenOperator::Data> OperatorKeywords = {
		//{"temp", TokenKeyword::Type::Temp},
		//{"true", TokenKeyword::Type::True},
		//{"false", TokenKeyword::Type::False},
		{"and", {TokenOperator::Type::And, TokenOperator::UnaryType::NotUnary}},
		{"or", {TokenOperator::Type::Or, TokenOperator::UnaryType::NotUnary}},
		{"not", {TokenOperator::Type::Not, TokenOperator::UnaryType::Prefix}},
		{"mod", {TokenOperator::Type::Modulus, TokenOperator::UnaryType::NotUnary}},
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

	///////////////////////////////////////////////////////////////////////////////////////////////

	Token* builtin_pow(TokenStack& stack, VariableMap& variables, const VariableMap& constants) {
		Variant expo = stack.top(0)->get_variant_value(variables, constants).value();
		Variant base = stack.top(1)->get_variant_value(variables, constants).value();

		Token* result = new TokenNumberFloat(std::pow(as_float(base), as_float(expo)));

		stack.pop();
		stack.pop();
		return result;
	}

	Token* builtin_int(TokenStack& stack, VariableMap& variables, const VariableMap& constants) {
		Variant what = stack.top()->get_variant_value(variables, constants).value();
		Token* result = new TokenNumberInt(as_int(what));

		stack.pop();
		return result;
	}

	Token* builtin_float(TokenStack& stack, VariableMap& variables, const VariableMap& constants) {
		Variant what = stack.top()->get_variant_value(variables, constants).value();
		Token* result = new TokenNumberFloat(as_float(what));

		stack.pop();
		return result;
	}

	Token* builtin_floor(TokenStack& stack, VariableMap& variables, const VariableMap& constants) {
		Variant what = stack.top()->get_variant_value(variables, constants).value();
		Token* result = new TokenNumberInt(static_cast<std::int64_t>(std::floor(as_float(what))));
		
		stack.pop();
		return result;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////

	static const std::unordered_map<std::string, PtrTokenFunc> BuiltinFunctions = {
		{"POW", builtin_pow},
		{"INT", builtin_int},
		{"FLOAT", builtin_float},
		{"FLOOR", builtin_floor},
	};

	///////////////////////////////////////////////////////////////////////////////////////////////

	char next_char(const std::string& string, std::size_t index) {
		return index + 1 < string.length() ? string[index + 1] : 0;
	}
}


#undef O
#undef C

bool ExpressionParser::as_bool(const Variant& variant) {
	switch (variant.index()) {
		case Variant_Bool: {
			return std::get<bool>(variant);
		} break;

		case Variant_Int: {
			return std::get<std::int64_t>(variant) != 0;
		} break;

		case Variant_Float: {
			return std::get<double>(variant) != 0.0;
		} break;

		default: {
			throw;
		}
	}
}

std::int64_t ExpressionParser::as_int(const Variant& variant) {
	switch (variant.index()) {
		case Variant_Bool: {
			return static_cast<std::int64_t>(std::get<bool>(variant));
		} break;

		case Variant_Int: {
			return std::get<std::int64_t>(variant);
		} break;

		case Variant_Float: {
			return static_cast<std::int64_t>(std::get<double>(variant));
		} break;

		case Variant_String: {
			return std::stoll(std::get<std::string>(variant));
		} break;

		default: {
			throw;
		}
	}
}

double ExpressionParser::as_float(const Variant& variant) {
	switch (variant.index()) {
		case Variant_Bool: {
			return static_cast<double>(std::get<bool>(variant));
		} break;

		case Variant_Int: {
			return static_cast<double>(std::get<std::int64_t>(variant));
		} break;

		case Variant_Float: {
			return std::get<double>(variant);
		} break;

		case Variant_String: {
			return std::stod(std::get<std::string>(variant));
		} break;

		default: {
			throw;
		}
	}
}

std::string ExpressionParser::as_string(const Variant& variant) {
	switch (variant.index()) {
		case Variant_String: {
			return std::get<std::string>(variant);
		} break;

		default: {
			throw;
		}
	}
}

std::string ExpressionParser::to_printable_string(const Variant& variant) {
	switch (variant.index()) {
		case Variant_Bool: {
			return std::get<bool>(variant) ? "true" : "false";
		} break;

		case Variant_Int: {
			return std::to_string(std::get<std::int64_t>(variant));
		} break;

		case Variant_Float: {
			double value = std::get<double>(variant);
			if (std::rint(value) == value) {
				return std::to_string(static_cast<std::int64_t>(value));
			} else {
				return std::to_string(value);
			}
		} break;

		case Variant_String: {
			return std::get<std::string>(variant);
		} break;

		default: {
			throw;
		} break;
	}
}

bool Token::as_bool() const { throw; }
std::int64_t Token::as_int() const { throw; }
double Token::as_float() const { throw; }
const std::string& Token::as_string() const { throw; }

std::string Token::to_printable_string() const { throw; }

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
Token* Token::operator_bitnot() const { throw; }
Token* Token::operator_negative() const { throw; }

Token* Token::operator_equal(const Token* other) const { throw; }
Token* Token::operator_notequal(const Token* other) const { throw; }
Token* Token::operator_less(const Token* other) const { throw; }
Token* Token::operator_greater(const Token* other) const { throw; }
Token* Token::operator_lessequal(const Token* other) const { throw; }
Token* Token::operator_greaterequal(const Token* other) const { throw; }

Token* Token::operator_and(const Token* other) const { throw; }
Token* Token::operator_or(const Token* other) const { throw; }

Token* Token::operator_bitand(const Token* other) const { throw; }
Token* Token::operator_bitor(const Token* other) const { throw; }
Token* Token::operator_bitxor(const Token* other) const { throw; }
Token* Token::operator_shiftleft(const Token* other) const { throw; }
Token* Token::operator_shiftright(const Token* other) const { throw; }

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

bool TokenBoolean::as_bool() const {
	return data;
}

std::int64_t TokenBoolean::as_int() const {
	return data ? 1 : 0;
}

double TokenBoolean::as_float() const {
	return data ? 1.0 : 0.0;
}

Token* TokenBoolean::operator_not() const {
	return new TokenBoolean(!data);
}

std::string TokenBoolean::to_printable_string() const {
	return data ? "true" : "false";
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

Token* TokenBoolean::operator_and(const Token* other) const {
	if (other->get_type() == TokenType::Boolean) {
		return new TokenBoolean(data && static_cast<const TokenBoolean*>(other)->data);
	} else if (other->get_type() == TokenType::NumberInt) {
		return new TokenBoolean(data && static_cast<bool>(static_cast<const TokenNumberInt*>(other)->data));
	} else if (other->get_type() == TokenType::NumberFloat) {
		return new TokenBoolean(data && static_cast<bool>(static_cast<const TokenNumberFloat*>(other)->data));
	} else {
		throw;
	}
}

Token* TokenBoolean::operator_or(const Token* other) const {
	if (other->get_type() == TokenType::Boolean) {
		return new TokenBoolean(data || static_cast<const TokenBoolean*>(other)->data);
	} else if (other->get_type() == TokenType::NumberInt) {
		return new TokenBoolean(data || static_cast<bool>(static_cast<const TokenNumberInt*>(other)->data));
	} else if (other->get_type() == TokenType::NumberFloat) {
		return new TokenBoolean(data || static_cast<bool>(static_cast<const TokenNumberFloat*>(other)->data));
	} else {
		throw;
	}
}

Token* TokenBoolean::operator_bitand(const Token* other) const {
	if (other->get_type() == TokenType::Boolean) {
		return new TokenBoolean(data & static_cast<const TokenBoolean*>(other)->data);
	} else if (other->get_type() == TokenType::NumberInt) {
		return new TokenBoolean(data & static_cast<bool>(static_cast<const TokenNumberInt*>(other)->data));
	} else if (other->get_type() == TokenType::NumberFloat) {
		return new TokenBoolean(data & static_cast<bool>(static_cast<const TokenNumberFloat*>(other)->data));
	} else {
		throw;
	}
}

Token* TokenBoolean::operator_bitor(const Token* other) const {
	if (other->get_type() == TokenType::Boolean) {
		return new TokenBoolean(data | static_cast<const TokenBoolean*>(other)->data);
	} else if (other->get_type() == TokenType::NumberInt) {
		return new TokenBoolean(data | static_cast<bool>(static_cast<const TokenNumberInt*>(other)->data));
	} else if (other->get_type() == TokenType::NumberFloat) {
		return new TokenBoolean(data | static_cast<bool>(static_cast<const TokenNumberFloat*>(other)->data));
	} else {
		throw;
	}
}

Token* TokenBoolean::operator_bitxor(const Token* other) const {
	if (other->get_type() == TokenType::Boolean) {
		return new TokenBoolean(data ^ static_cast<const TokenBoolean*>(other)->data);
	} else if (other->get_type() == TokenType::NumberInt) {
		return new TokenBoolean(data ^ static_cast<bool>(static_cast<const TokenNumberInt*>(other)->data));
	} else if (other->get_type() == TokenType::NumberFloat) {
		return new TokenBoolean(data ^ static_cast<bool>(static_cast<const TokenNumberFloat*>(other)->data));
	} else {
		throw;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool TokenNumberInt::as_bool() const {
	return data != 0;
}

std::int64_t TokenNumberInt::as_int() const {
	return data;
}

double TokenNumberInt::as_float() const {
	return static_cast<double>(data);
}

std::string TokenNumberInt::to_printable_string() const {
	return std::to_string(data);
}

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

Token* TokenNumberInt::operator_not() const {
	return new TokenNumberInt(data == 0 ? 1 : 0);
}

OP_CMP_INT(TokenNumberInt, equal, ==);
OP_CMP_INT(TokenNumberInt, notequal, !=);
OP_CMP_INT(TokenNumberInt, less, <);
OP_CMP_INT(TokenNumberInt, greater, >);
OP_CMP_INT(TokenNumberInt, lessequal, <=);
OP_CMP_INT(TokenNumberInt, greaterequal, >=);

Token* TokenNumberInt::operator_and(const Token* other) const {
	if (other->get_type() == TokenType::Boolean) {
		return new TokenNumberInt(data && static_cast<std::int64_t>(static_cast<const TokenBoolean*>(other)->data));
	} else if (other->get_type() == TokenType::NumberInt) {
		return new TokenNumberInt(data && static_cast<const TokenNumberInt*>(other)->data);
	} else if (other->get_type() == TokenType::NumberFloat) {
		return new TokenNumberInt(data && static_cast<std::int64_t>(static_cast<const TokenNumberFloat*>(other)->data));
	} else {
		throw;
	}
}

Token* TokenNumberInt::operator_or(const Token* other) const {
	if (other->get_type() == TokenType::Boolean) {
		return new TokenNumberInt(data || static_cast<std::int64_t>(static_cast<const TokenBoolean*>(other)->data));
	} else if (other->get_type() == TokenType::NumberInt) {
		return new TokenNumberInt(data || static_cast<const TokenNumberInt*>(other)->data);
	} else if (other->get_type() == TokenType::NumberFloat) {
		return new TokenNumberInt(data || static_cast<std::int64_t>(static_cast<const TokenNumberFloat*>(other)->data));
	} else {
		throw;
	}
}

Token* TokenNumberInt::operator_bitand(const Token* other) const {
	if (other->get_type() == TokenType::Boolean) {
		return new TokenNumberInt(data & static_cast<std::int64_t>(static_cast<const TokenBoolean*>(other)->data));
	} else if (other->get_type() == TokenType::NumberInt) {
		return new TokenNumberInt(data & static_cast<const TokenNumberInt*>(other)->data);
	} else if (other->get_type() == TokenType::NumberFloat) {
		return new TokenNumberInt(data & static_cast<std::int64_t>(static_cast<const TokenNumberFloat*>(other)->data));
	} else {
		throw;
	}
}

Token* TokenNumberInt::operator_bitor(const Token* other) const {
	if (other->get_type() == TokenType::Boolean) {
		return new TokenNumberInt(data | static_cast<std::int64_t>(static_cast<const TokenBoolean*>(other)->data));
	} else if (other->get_type() == TokenType::NumberInt) {
		return new TokenNumberInt(data | static_cast<const TokenNumberInt*>(other)->data);
	} else if (other->get_type() == TokenType::NumberFloat) {
		return new TokenNumberInt(data | static_cast<std::int64_t>(static_cast<const TokenNumberFloat*>(other)->data));
	} else {
		throw;
	}
}

Token* TokenNumberInt::operator_bitxor(const Token* other) const {
	if (other->get_type() == TokenType::Boolean) {
		return new TokenNumberInt(data ^ static_cast<std::int64_t>(static_cast<const TokenBoolean*>(other)->data));
	} else if (other->get_type() == TokenType::NumberInt) {
		return new TokenNumberInt(data ^ static_cast<const TokenNumberInt*>(other)->data);
	} else if (other->get_type() == TokenType::NumberFloat) {
		return new TokenNumberInt(data ^ static_cast<std::int64_t>(static_cast<const TokenNumberFloat*>(other)->data));
	} else {
		throw;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool TokenNumberFloat::as_bool() const {
	return data != 0.0;
}

std::int64_t TokenNumberFloat::as_int() const {
	return static_cast<std::int64_t>(data);
}

double TokenNumberFloat::as_float() const {
	return data;
}

std::string TokenNumberFloat::to_printable_string() const {
	return std::to_string(data);
}

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

Token* TokenNumberFloat::operator_not() const {
	return new TokenNumberFloat(data == 0.0 ? 1.0 : 0.0);
}

OP_CMP_FLOAT(TokenNumberFloat, equal, ==);
OP_CMP_FLOAT(TokenNumberFloat, notequal, !=);
OP_CMP_FLOAT(TokenNumberFloat, less, <);
OP_CMP_FLOAT(TokenNumberFloat, greater, >);
OP_CMP_FLOAT(TokenNumberFloat, lessequal, <=);
OP_CMP_FLOAT(TokenNumberFloat, greaterequal, >=);

Token* TokenNumberFloat::operator_and(const Token* other) const {
	if (other->get_type() == TokenType::Boolean) {
		return new TokenNumberFloat(data && static_cast<double>(static_cast<const TokenBoolean*>(other)->data));
	} else if (other->get_type() == TokenType::NumberInt) {
		return new TokenNumberFloat(data && static_cast<double>(static_cast<const TokenNumberInt*>(other)->data));
	} else if (other->get_type() == TokenType::NumberFloat) {
		return new TokenNumberFloat(data && static_cast<const TokenNumberFloat*>(other)->data);
	} else {
		throw;
	}
}

Token* TokenNumberFloat::operator_or(const Token* other) const {
	if (other->get_type() == TokenType::Boolean) {
		return new TokenNumberFloat(data || static_cast<double>(static_cast<const TokenBoolean*>(other)->data));
	} else if (other->get_type() == TokenType::NumberInt) {
		return new TokenNumberFloat(data || static_cast<double>(static_cast<const TokenNumberInt*>(other)->data));
	} else if (other->get_type() == TokenType::NumberFloat) {
		return new TokenNumberFloat(data || static_cast<const TokenNumberFloat*>(other)->data);
	} else {
		throw;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool TokenStringLiteral::as_bool() const {
	return !data.empty();
}

const std::string& TokenStringLiteral::as_string() const {
	return data;
}

std::string TokenStringLiteral::to_printable_string() const {
	return data;
}

Token* TokenStringLiteral::operator_plus(const Token* other) const {
	if (other->get_type() == TokenType::StringLiteral) {
		return new TokenStringLiteral(data + static_cast<const TokenStringLiteral*>(other)->data);
	} else {
		throw;
	}
}

Token* TokenStringLiteral::operator_equal(const Token* other) const {
	if (other->get_type() == TokenType::StringLiteral) {
		return new TokenBoolean(data == static_cast<const TokenStringLiteral*>(other)->data);
	} else {
		throw;
	}
}

Token* TokenStringLiteral::operator_notequal(const Token* other) const {
	if (other->get_type() == TokenType::StringLiteral) {
		return new TokenBoolean(data != static_cast<const TokenStringLiteral*>(other)->data);
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

const std::string& TokenKnotName::as_string() const {
	return data.knot;
}

std::string TokenKnotName::to_printable_string() const {
	return data.knot;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Token::ValueResult TokenVariable::get_value(const VariableMap& variables, const VariableMap& constants) {
	if (auto var_value = variables.find(data); var_value != variables.end()) {
		switch (var_value->second.index()) {
			case Variant_Bool: {
				return {new TokenBoolean(std::get<bool>(var_value->second)), true};
			} break;

			case Variant_Int: {
				return {new TokenNumberInt(std::get<std::int64_t>(var_value->second)), true};
			} break;

			case Variant_Float: {
				return {new TokenNumberFloat(std::get<double>(var_value->second)), true};
			} break;

			case Variant_String: {
				return {new TokenStringLiteral(std::get<std::string>(var_value->second)), true};
			} break;

			default: {
				throw;
			} break;
		}
	} else if (auto const_value = constants.find(data); const_value != constants.end()) {
		// TODO: dry it
		switch (const_value->second.index()) {
			case Variant_Bool: {
				return {new TokenBoolean(std::get<bool>(const_value->second)), true};
			} break;

			case Variant_Int: {
				return {new TokenNumberInt(std::get<std::int64_t>(const_value->second)), true};
			} break;

			case Variant_Float: {
				return {new TokenNumberFloat(std::get<double>(const_value->second)), true};
			} break;

			case Variant_String: {
				return {new TokenStringLiteral(std::get<std::string>(const_value->second)), true};
			} break;

			default: {
				throw;
			} break;
		}
	} else {
		return {nullptr, false};
	}
}

std::optional<Variant> TokenVariable::get_variant_value(const VariableMap& variables, const VariableMap& constants) const {
	if (auto var_value = variables.find(data); var_value != variables.end()) {
		return var_value->second;
	} else if (auto const_value = constants.find(data); const_value != constants.end()) {
		return const_value->second;
	} else {
		return {};
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Token* TokenFunction::call(TokenStack& stack, const FunctionMap& all_functions, VariableMap& variables, const VariableMap& constants) {
	if (!data.defer_fetch) {
		return (data.function)(stack, variables, constants);
	} else {
		if (auto function = all_functions.find(data.name); function != all_functions.end()) {
			return (function->second)(stack, variables, constants);
		} else {
			throw;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void try_add_word(std::vector<Token*>& result, std::string& word, const FunctionMap& all_functions, bool in_knot_name, const std::unordered_set<std::string>& deferred_functions) {
	if (!word.empty()) {
		bool found_result = false;
		if (auto keyword = OperatorKeywords.find(word); keyword != OperatorKeywords.end()) {
			result.push_back(new TokenOperator(keyword->second.type, keyword->second.unary_type));
			found_result = true;
		} else if (word == "true") {
			result.push_back(new TokenBoolean(true));
			found_result = true;
		} else if (word == "false") {
			result.push_back(new TokenBoolean(false));
			found_result = true;
		} else if (auto func = all_functions.find(word); func != all_functions.end()) {
			result.push_back(new TokenFunction(func->first, func->second, false));
			found_result = true;
		} else if (deferred_functions.contains(word)) {
			result.push_back(new TokenFunction(word, nullptr, true));
			found_result = true;
		} else {
			if (word.contains(".")) {
				try {
					double word_float = std::stod(word);
					result.push_back(new TokenNumberFloat(word_float));
					found_result = true;
				} catch (...) {
					//result.push_back(new TokenVariable(word));
				}
			} else {
				try {
					std::int64_t word_int = std::stoll(word);
					result.push_back(new TokenNumberInt(word_int));
					found_result = true;
				} catch (...) {
					//result.push_back(new TokenVariable(word));
				}
			}
		}

		if (!found_result) {
			if (in_knot_name) {
				result.push_back(new TokenKnotName(word, true));
			} else {
				result.push_back(new TokenVariable(word));
			}
		}

		word.clear();
	}
}

std::vector<Token*> ExpressionParser::tokenize_expression(const std::string& expression, const FunctionMap& all_functions, const std::unordered_set<std::string>& deferred_functions) {
	std::vector<Token*> result;
	result.reserve(128);

	std::string current_word;
	current_word.reserve(32);

	bool in_quotes = false;
	std::string current_string;
	current_string.reserve(32);

	bool in_knot_name = false;

	std::size_t index = 0;
	while (index < expression.length()) {
		using UnaryType = TokenOperator::UnaryType;
		char this_char = expression[index];
		if (!in_quotes) {
			switch (this_char) {
				case '+': {
					if (next_char(expression, index) == '+') {
						if (next_char(expression, index + 1) <= 32) {
							try_add_word(result, current_word, all_functions, in_knot_name, deferred_functions);
							result.push_back(new TokenOperator(TokenOperator::Type::Increment, UnaryType::Postfix));
						} else {
							result.push_back(new TokenOperator(TokenOperator::Type::Increment, UnaryType::Prefix));
						}
						
						++index;
					} else {
						result.push_back(new TokenOperator(TokenOperator::Type::Plus, UnaryType::NotUnary));
					}
				} break;

				case '-': {
					if (next_char(expression, index) == '-') {
						if (next_char(expression, index + 1) <= 32) {
							try_add_word(result, current_word, all_functions, in_knot_name, deferred_functions);
							result.push_back(new TokenOperator(TokenOperator::Type::Decrement, UnaryType::Postfix));
						} else {
							result.push_back(new TokenOperator(TokenOperator::Type::Decrement, UnaryType::Prefix));
						}
						
						++index;
					} else if (next_char(expression, index) == '>') {
						in_knot_name = true;
						++index;
					} else {
						if (next_char(expression, index) > 32 && !result.empty() && result.back()->get_type() == TokenType::Operator) {
							result.push_back(new TokenOperator(TokenOperator::Type::Negative, UnaryType::Prefix));
						} else {
							result.push_back(new TokenOperator(TokenOperator::Type::Minus, UnaryType::NotUnary));
						}
					}
				} break;

				case '=': {
					if (next_char(expression, index) == '=') {
						result.push_back(new TokenOperator(TokenOperator::Type::Equal, UnaryType::NotUnary));
						++index;
					} else {
						result.push_back(new TokenOperator(TokenOperator::Type::Assign, UnaryType::NotUnary));
					}
				} break;

				case '<': {
					if (next_char(expression, index) == '=') {
						result.push_back(new TokenOperator(TokenOperator::Type::LessEqual, UnaryType::NotUnary));
						++index;
					} else if (next_char(expression, index) == '<') {
						result.push_back(new TokenOperator(TokenOperator::Type::ShiftLeft, UnaryType::NotUnary));
					} else {
						result.push_back(new TokenOperator(TokenOperator::Type::Less, UnaryType::NotUnary));
					}
				} break;
				
				case '>': {
					if (next_char(expression, index) == '=') {
						result.push_back(new TokenOperator(TokenOperator::Type::GreaterEqual, UnaryType::NotUnary));
						++index;
					} else if (next_char(expression, index) == '>') {
						result.push_back(new TokenOperator(TokenOperator::Type::ShiftRight, UnaryType::NotUnary));
					} else {
						result.push_back(new TokenOperator(TokenOperator::Type::Greater, UnaryType::NotUnary));
					}
				} break;

				case '*': {
					result.push_back(new TokenOperator(TokenOperator::Type::Multiply, UnaryType::NotUnary));
				} break;

				case '/': {
					result.push_back(new TokenOperator(TokenOperator::Type::Divide, UnaryType::NotUnary));
				} break;

				case '%': {
					result.push_back(new TokenOperator(TokenOperator::Type::Modulus, UnaryType::NotUnary));
				} break;

				case '?': {
					result.push_back(new TokenOperator(TokenOperator::Type::Substring, UnaryType::NotUnary));
				} break;

				case '!': {
					if (next_char(expression, index) == '=') {
						result.push_back(new TokenOperator(TokenOperator::Type::NotEqual, UnaryType::NotUnary));
						++index;
					} else if (next_char(expression, index) > 32) {
						result.push_back(new TokenOperator(TokenOperator::Type::Not, UnaryType::Prefix));
					}
				} break;

				case '&': {
					if (next_char(expression, index) == '&') {
						result.push_back(new TokenOperator(TokenOperator::Type::And, UnaryType::NotUnary));
						++index;
					} else {
						result.push_back(new TokenOperator(TokenOperator::Type::BitAnd, UnaryType::NotUnary));
					}
				} break;

				case '|': {
					if (next_char(expression, index) == '|') {
						result.push_back(new TokenOperator(TokenOperator::Type::Or, UnaryType::NotUnary));
						++index;
					} else {
						result.push_back(new TokenOperator(TokenOperator::Type::BitOr, UnaryType::NotUnary));
					}
				} break;

				case '^': {
					result.push_back(new TokenOperator(TokenOperator::Type::BitXor, UnaryType::NotUnary));
				} break;

				case '~': {
					if (next_char(expression, index) > 32) {
						result.push_back(new TokenOperator(TokenOperator::Type::BitNot, UnaryType::Prefix));
					}
				} break;

				case '(': {
					try_add_word(result, current_word, all_functions, in_knot_name, deferred_functions);
					result.push_back(new TokenParenComma(TokenParenComma::Type::LeftParen));
				} break;

				case ')': {
					try_add_word(result, current_word, all_functions, in_knot_name, deferred_functions);
					result.push_back(new TokenParenComma(TokenParenComma::Type::RightParen));
				} break;

				case ',': {
					try_add_word(result, current_word, all_functions, in_knot_name, deferred_functions);
					result.push_back(new TokenParenComma(TokenParenComma::Type::Comma));
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
						try_add_word(result, current_word, all_functions, in_knot_name, deferred_functions);
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

	try_add_word(result, current_word, all_functions, in_knot_name, deferred_functions);

	return result;
}

std::vector<Token*> ExpressionParser::shunt(const std::vector<Token*>& infix, std::unordered_set<Token*>& tokens_shunted) {
	std::vector<Token*> postfix;
	std::stack<Token*> stack;

	std::size_t index = 0;
	while (index < infix.size()) {
		Token* this_token = infix[index];

		switch (this_token->get_type()) {
			case TokenType::Boolean:
			case TokenType::NumberInt:
			case TokenType::NumberFloat:
			case TokenType::StringLiteral:
			case TokenType::Variable:
			case TokenType::KnotName: {
				postfix.push_back(this_token);
				tokens_shunted.insert(this_token);
			} break;

			case TokenType::Operator: {
				TokenOperator::UnaryType unary_type = static_cast<TokenOperator*>(this_token)->data.unary_type;
				if (unary_type != TokenOperator::UnaryType::NotUnary) {
					if (unary_type == TokenOperator::UnaryType::Postfix) {
						postfix.push_back(this_token);
						tokens_shunted.insert(this_token);
					} else {
						stack.push(this_token);
					}
				} else {
					while (!stack.empty()) {
						Token* next_op = stack.top();

						bool higher_precedence = false;
						if (next_op->get_type() == TokenType::Operator) {
							TokenOperator::Type this_type = static_cast<TokenOperator*>(this_token)->data.type;
							TokenOperator::Type that_type = static_cast<TokenOperator*>(next_op)->data.type;
							higher_precedence = OperatorPrecedences.at(that_type) <= OperatorPrecedences.at(this_type);
						} else {
							break;
						}

						if (higher_precedence) {
							postfix.push_back(next_op);
							tokens_shunted.insert(next_op);
							stack.pop();
						} else {
							break;
						}
					}

					stack.push(this_token);
				}
			} break;

			case TokenType::ParenComma: {
				auto* paren_comma = static_cast<TokenParenComma*>(this_token);
				switch (paren_comma->data) {
					case TokenParenComma::Type::LeftParen: {
						stack.push(this_token);
					} break;

					case TokenParenComma::Type::RightParen: {
						while (!stack.empty()) {
							Token* next = stack.top();
							if (next->get_type() == TokenType::ParenComma && static_cast<TokenParenComma*>(next)->data == TokenParenComma::Type::LeftParen) {
								break;
							} else {
								stack.pop();
								postfix.push_back(next);
								tokens_shunted.insert(next);
							}
						}

						if (stack.empty()) {
							throw;
						}

						stack.pop();
						if (!stack.empty()) {
							Token* new_top = stack.top();
							if (new_top->get_type() == TokenType::Function) {
								postfix.push_back(new_top);
								tokens_shunted.insert(new_top);
								stack.pop();
							}
						}
					} break;

					case TokenParenComma::Type::Comma:
					default: {
						while (!stack.empty()) {
							Token* next = stack.top();
							if (next->get_type() == TokenType::ParenComma) {
								if (static_cast<TokenParenComma*>(next)->data != TokenParenComma::Type::LeftParen) {
									postfix.push_back(next);
									tokens_shunted.insert(next);
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

			default: {

			} break;
		}

		++index;
	}

	while (!stack.empty()) {
		postfix.push_back(stack.top());
		tokens_shunted.insert(stack.top());
		stack.pop();
	}

	return postfix;
}

#define OP_UN_PRE(type, func) case Type::type: {\
		Token::ValueResult operand = stack.top()->get_value(variables, constants);\
		stack.pop();\
\
		Token* result = operand.token->operator_##func();\
		tokens_to_dealloc.insert(result);\
\
		if (operand.from_variable) {\
			delete operand.token;\
		}\
\
		stack.push(result);\
	} break;

#define OP_BIN(type, func) case Type::type: {\
		Token::ValueResult right = stack.top()->get_value(variables, constants);\
		stack.pop();\
		Token::ValueResult left = stack.top()->get_value(variables, constants);\
		stack.pop();\
\
		Token* result = left.token->operator_##func(right.token);\
		tokens_to_dealloc.insert(result);\
\
		if (left.from_variable) {\
			delete left.token;\
		}\
\
		if (right.from_variable) {\
			delete right.token;\
		}\
\
		stack.push(result);\
	} break;

std::optional<Variant> ExpressionParser::execute_expression_tokens(const std::vector<Token*>& expression_tokens, VariableMap& variables, const VariableMap& constants, const FunctionMap& all_functions) {
	TokenStack stack;
	std::unordered_set<Token*> tokens_to_dealloc;

	std::size_t index = 0;
	while (index < expression_tokens.size()) {
		Token* this_token = expression_tokens[index];

		switch (this_token->get_type()) {
			case TokenType::Boolean:
			case TokenType::NumberInt:
			case TokenType::NumberFloat:
			case TokenType::StringLiteral:
			case TokenType::Variable:
			case TokenType::KnotName: {
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

					OP_BIN(And, and);
					OP_BIN(Or, or);

					OP_BIN(BitAnd, bitand);
					OP_BIN(BitOr, bitor);
					OP_BIN(BitXor, bitxor);
					OP_BIN(ShiftLeft, shiftleft);
					OP_BIN(ShiftRight, shiftright);

					OP_BIN(Substring, substring);

					OP_UN_PRE(Negative, negative);
					OP_UN_PRE(Not, not);
					OP_UN_PRE(BitNot, bitnot);

					case Type::Increment:
					case Type::Decrement: {
						Token::ValueResult operand = stack.top()->get_value(variables, constants);

						Token* result = nullptr;
						if (op->data.unary_type == TokenOperator::UnaryType::Prefix) {
							result = op->data.type == Type::Increment ? operand.token->operator_inc_pre() : operand.token->operator_dec_pre();
						} else {
							result = op->data.type == Type::Increment ? operand.token->operator_inc_post() : operand.token->operator_dec_post();
						}

						if (operand.from_variable && op->data.unary_type == TokenOperator::UnaryType::Postfix) {
							TokenNumberInt add{op->data.type == Type::Increment ? 1 : -1};
							Token* new_result = result->operator_plus(&add);
							variables[static_cast<TokenVariable*>(stack.top())->data] = new_result->get_variant_value(variables, constants).value();
							delete operand.token;
							delete new_result;
						}

						stack.pop();
						stack.push(result);

						if (op->data.unary_type == TokenOperator::UnaryType::Postfix) {
							tokens_to_dealloc.insert(result);
						}
					} break;

					case Type::Assign: {
						Token* value = stack.top();
						stack.pop();
						Token* var = stack.top();
						stack.pop();
						if (var->get_type() != TokenType::Variable) {
							throw;
						}

						std::string var_name = static_cast<TokenVariable*>(var)->data;
						if (std::optional<Variant> var_value = value->get_variant_value(variables, constants); var_value.has_value()) {
							variables[var_name] = var_value.value();
						} else {
							throw;
						}

						//tokens_to_dealloc.erase(value);
					} break;

					default: {

					} break;
				}
			} break;

			case TokenType::Function: {
				if (Token* result = static_cast<TokenFunction*>(this_token)->call(stack, all_functions, variables, constants)) {
					stack.push(result);
					tokens_to_dealloc.insert(result);
				}
			} break;

			default: {

			} break;
		}

		++index;
	}

	std::optional<Variant> result{};
	if (!stack.empty()) {
		result = stack.top()->get_variant_value(variables, constants);
	}

	for (Token* token : tokens_to_dealloc) {
		delete token;
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::optional<Variant> ExpressionParser::execute_expression(const std::string& expression, const FunctionMap& functions, const std::unordered_set<std::string>& deferred_functions) {
	FunctionMap all_functions = BuiltinFunctions;
	if (!functions.empty()) {
		all_functions.insert(functions.begin(), functions.end());
	}
	
	std::unordered_set<Token*> dummy;
	std::vector<Token*> tokenized = tokenize_expression(expression, all_functions, deferred_functions);
	std::vector<Token*> shunted = shunt(tokenized, dummy);

	VariableMap no_vars;
	VariableMap no_consts;
	std::optional<Variant> result = execute_expression_tokens(shunted, no_vars, no_consts, all_functions);

	for (Token* token : tokenized) {
		delete token;
	}

	return result;
}

std::optional<Variant> ExpressionParser::execute_expression(const std::string& expression, VariableMap& variables, const VariableMap& constants, const FunctionMap& functions, const std::unordered_set<std::string>& deferred_functions) {
	FunctionMap all_functions = BuiltinFunctions;
	if (!functions.empty()) {
		all_functions.insert(functions.begin(), functions.end());
	}

	std::unordered_set<Token*> dummy;
	std::vector<Token*> tokenized = tokenize_expression(expression, all_functions, deferred_functions);
	std::vector<Token*> shunted = shunt(tokenized, dummy);

	std::optional<Variant> result = execute_expression_tokens(shunted, variables, constants, all_functions);

	for (Token* token : tokenized) {
		delete token;
	}

	//return InternalPackedToken(result);
	return result;
}

std::vector<Token*> ExpressionParser::tokenize_and_shunt_expression(const std::string& expression, const FunctionMap& functions, const std::unordered_set<std::string>& deferred_functions) {
	FunctionMap all_functions = BuiltinFunctions;
	if (!functions.empty()) {
		all_functions.insert(functions.begin(), functions.end());
	}

	std::unordered_set<Token*> tokens_shunted;
	std::vector<Token*> tokenized = tokenize_expression(expression, all_functions, deferred_functions);
	std::vector<Token*> shunted = shunt(tokenized, tokens_shunted);

	for (Token* token : tokenized) {
		if (!tokens_shunted.contains(token)) {
			delete token;
		}
	}

	return shunted;
}
