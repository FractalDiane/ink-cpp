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

	/*Token* builtin_pow(TokenStack& stack, VariableMap& variables, const VariableMap& constants, RedirectMap& variable_redirects) {
		Variant expo = stack.top(0)->get_variant_value(variables, constants, variable_redirects).value();
		Variant base = stack.top(1)->get_variant_value(variables, constants, variable_redirects).value();

		Token* result = new TokenNumberFloat(std::pow(as_float(base), as_float(expo)));

		stack.pop();
		stack.pop();
		return result;
	}

	Token* builtin_int(TokenStack& stack, VariableMap& variables, const VariableMap& constants, RedirectMap& variable_redirects) {
		Variant what = stack.top()->get_variant_value(variables, constants, variable_redirects).value();
		Token* result = new TokenNumberInt(as_int(what));

		stack.pop();
		return result;
	}

	Token* builtin_float(TokenStack& stack, VariableMap& variables, const VariableMap& constants, RedirectMap& variable_redirects) {
		Variant what = stack.top()->get_variant_value(variables, constants, variable_redirects).value();
		Token* result = new TokenNumberFloat(as_float(what));

		stack.pop();
		return result;
	}

	Token* builtin_floor(TokenStack& stack, VariableMap& variables, const VariableMap& constants, RedirectMap& variable_redirects) {
		Variant what = stack.top()->get_variant_value(variables, constants, variable_redirects).value();
		Token* result = new TokenNumberFloat(std::floor(as_float(what)));
		
		stack.pop();
		return result;
	}

	Token* builtin_ceil(TokenStack& stack, VariableMap& variables, const VariableMap& constants, RedirectMap& variable_redirects) {
		Variant what = stack.top()->get_variant_value(variables, constants, variable_redirects).value();
		Token* result = new TokenNumberFloat(std::ceil(as_float(what)));
		
		stack.pop();
		return result;
	}*/

	Variant builtin_pow(const std::vector<Variant>& args) {
		const Variant& base = args[0];
		const Variant& exponent = args[1];
		return std::pow(base, exponent);
	}

	Variant builtin_int(const std::vector<Variant>& args) {
		return static_cast<std::int64_t>(args[0]);
	}

	Variant builtin_float(const std::vector<Variant>& args) {
		return static_cast<double>(args[0]);
	}

	Variant builtin_floor(const std::vector<Variant>& args) {
		return std::floor(args[0]);
	}

	Variant builtin_ceil(const std::vector<Variant>& args) {
		return std::ceil(args[0]);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////

	static const std::unordered_map<std::string, InkFunction> BuiltinFunctions = {
		{"POW", builtin_pow},
		{"INT", builtin_int},
		{"FLOAT", builtin_float},
		{"FLOOR", builtin_floor},
		{"CEILING", builtin_ceil},
	};

	///////////////////////////////////////////////////////////////////////////////////////////////

	char next_char(const std::string& string, std::size_t index) {
		return index + 1 < string.length() ? string[index + 1] : 0;
	}
}


#undef O
#undef C

/*ByteVec Serializer<Token*>::operator()(const Token* token) {
	ByteVec result = {static_cast<std::uint8_t>(token->get_type())};
	ByteVec result2 = token->to_serialized_bytes();
	result.insert(result.end(), result2.begin(), result2.end());

	return result;
}

Token* Deserializer<Token*>::operator()(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::uint8_t> ds8;
	Deserializer<std::int64_t> dsi64;
	Deserializer<double> dsdb;
	Deserializer<std::string> dsstring;

	Token* result = nullptr;
	TokenType type = static_cast<TokenType>(ds8(bytes, index));
	switch (type) {
		case TokenType::Boolean: {
			bool value = static_cast<bool>(ds8(bytes, index));
			TokenBoolean* result_bool = new TokenBoolean(value);
			result = result_bool;
		} break;

		case TokenType::NumberInt: {
			std::int64_t value = dsi64(bytes, index);
			TokenNumberInt* result_int = new TokenNumberInt(value);
			result = result_int;
		} break;

		case TokenType::NumberFloat: {
			double value = dsdb(bytes, index);
			TokenNumberFloat* result_float = new TokenNumberFloat(value);
			result = result_float;
		} break;

		case TokenType::StringLiteral: {
			std::string value = dsstring(bytes, index);
			TokenStringLiteral* result_string = new TokenStringLiteral(value);
			result = result_string;
		} break;

		case TokenType::Variable: {
			std::string name = dsstring(bytes, index);
			TokenVariable* result_var = new TokenVariable(name);
			result = result_var;
		} break;

		case TokenType::Function: {
			std::string name = dsstring(bytes, index);
			std::uint8_t args = ds8(bytes, index);
			auto fetch_method = static_cast<TokenFunction::FetchMethod>(ds8(bytes, index));
			TokenFunction* result_func = new TokenFunction(name, nullptr, fetch_method);
			result_func->data.argument_count = args;
			result = result_func;
		} break;

		case TokenType::Operator: {
			TokenOperator::Type my_type = static_cast<TokenOperator::Type>(ds8(bytes, index));
			TokenOperator::UnaryType my_unary_type = static_cast<TokenOperator::UnaryType>(ds8(bytes, index));
			TokenOperator* result_op = new TokenOperator(my_type, my_unary_type);
			result = result_op;
		} break;

		case TokenType::ParenComma: {
			TokenParenComma::Type my_type = static_cast<TokenParenComma::Type>(ds8(bytes, index));
			TokenParenComma* result_parencomma = new TokenParenComma(my_type);
			result = result_parencomma;
		} break;

		case TokenType::Keyword: {
			TokenKeyword::Type my_type = static_cast<TokenKeyword::Type>(ds8(bytes, index));
			TokenKeyword* result_keyword = new TokenKeyword(my_type);
			result = result_keyword;
		} break;

		case TokenType::KnotName: {
			std::string knot = dsstring(bytes, index);
			TokenKnotName* result_knotname = new TokenKnotName(knot, true);
			result = result_knotname;
		} break;

		default: {
			throw;
		} break;
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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
				return std::format("{:.7f}", value);
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

Token* ExpressionParser::variant_to_token(const Variant& variant) {
	switch (variant.index()) {
		case Variant_Bool: {
			return new TokenBoolean(std::get<bool>(variant));
		} break;

		case Variant_Int: {
			return new TokenNumberInt(std::get<std::int64_t>(variant));
		} break;

		case Variant_Float: {
			return new TokenNumberFloat(std::get<double>(variant));
		} break;

		case Variant_String: {
			return new TokenStringLiteral(std::get<std::string>(variant));
		} break;

		default: {
			throw;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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

ByteVec Token::to_serialized_bytes() const {
	return {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ByteVec TokenKeyword::to_serialized_bytes() const {
	Serializer<std::uint8_t> s8;
	return s8(static_cast<std::uint8_t>(data));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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

ByteVec TokenBoolean::to_serialized_bytes() const {
	Serializer<std::uint8_t> s;
	return s(static_cast<std::uint8_t>(data));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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

ByteVec TokenNumberInt::to_serialized_bytes() const {
	Serializer<std::int64_t> s;
	return s(data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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

ByteVec TokenNumberFloat::to_serialized_bytes() const {
	Serializer<double> s;
	return s(data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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

ByteVec TokenStringLiteral::to_serialized_bytes() const {
	Serializer<std::string> s;
	return s(data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string& TokenKnotName::as_string() const {
	return data.knot;
}

std::string TokenKnotName::to_printable_string() const {
	return data.knot;
}

ByteVec TokenKnotName::to_serialized_bytes() const {
	Serializer<std::string> s;
	return s(data.knot);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Token::ValueResult TokenVariable::get_value(const VariableMap& variables, const VariableMap& constants, RedirectMap& variable_redirects) {
	const Variant* value = nullptr;

	if (auto var_value = variables.find(data); var_value != variables.end()) {
		value = &var_value->second;
	} else if (auto const_value = constants.find(data); const_value != constants.end()) {
		value = &const_value->second;
	} else {
		return {nullptr, false};
	}

	switch (value->index()) {
		case Variant_Bool: {
			return {new TokenBoolean(std::get<bool>(*value)), true};
		} break;

		case Variant_Int: {
			return {new TokenNumberInt(std::get<std::int64_t>(*value)), true};
		} break;

		case Variant_Float: {
			return {new TokenNumberFloat(std::get<double>(*value)), true};
		} break;

		case Variant_String: {
			return {new TokenStringLiteral(std::get<std::string>(*value)), true};
		} break;

		default: {
			throw;
		} break;
	}
}

std::optional<Variant> TokenVariable::get_variant_value(const VariableMap& variables, const VariableMap& constants, RedirectMap& variable_redirects) const {
	if (auto var_value = variables.find(data); var_value != variables.end()) {
		return var_value->second;
	} else if (auto const_value = constants.find(data); const_value != constants.end()) {
		return const_value->second;
	} else {
		return {};
	}
}

std::string TokenVariable::to_printable_string() const {
	return data;
}

ByteVec TokenVariable::to_serialized_bytes() const {
	Serializer<std::string> s;
	return s(data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Token* TokenFunction::call(TokenStack& stack, const FunctionMap& functions, VariableMap& variables, const VariableMap& constants, RedirectMap& variable_redirects) {
	switch (data.fetch_method) {
		case TokenFunction::FetchMethod::Immediate: {
			return (data.function)(stack, variables, constants, variable_redirects);
		} break;

		case TokenFunction::FetchMethod::Defer: {
			if (auto builtin = BuiltinFunctions.find(data.name); builtin != BuiltinFunctions.end()) {
			return (builtin->second)(stack, variables, constants, variable_redirects);
			} else if (auto function = functions.find(data.name); function != functions.end()) {
				return (function->second)(stack, variables, constants, variable_redirects);
			} else {
				throw std::runtime_error("Tried calling an unknown function");
			}
		} break;

		case TokenFunction::FetchMethod::StoryKnot:
		default: {
			throw std::runtime_error("Story knot function was not prepared before evaluating expression");
		} break;
	}
}

ByteVec TokenFunction::to_serialized_bytes() const {
	Serializer<std::string> s;
	Serializer<std::uint8_t> s8;
	ByteVec result = s(data.name);
	ByteVec result2 = s8(data.argument_count);
	ByteVec result3 = s8(static_cast<std::uint8_t>(data.fetch_method));
	result.push_back(result2[0]);
	return result2;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ByteVec TokenOperator::to_serialized_bytes() const {
	Serializer<std::uint8_t> s;
	ByteVec result = s(static_cast<std::uint8_t>(data.type));
	ByteVec result2 = s(static_cast<std::uint8_t>(data.unary_type));
	result.insert(result.end(), result2.begin(), result2.end());

	return result;
}

ByteVec TokenParenComma::to_serialized_bytes() const {
	Serializer<std::uint8_t> s;
	return s(static_cast<std::uint8_t>(data));
}*/

////////////////////////////////////////////////////////////////////////////////////////////////////

void try_add_word(const std::string& expression, std::size_t index, std::vector<Token>& result, std::string& word, const StoryVariableInfo& story_var_info, bool in_knot_name) {
	if (!word.empty()) {
		bool found_result = false;
		if (auto keyword = OperatorKeywords.find(word); keyword != OperatorKeywords.end()) {
			result.push_back(Token::operat(keyword->second.first, keyword->second.second));
			found_result = true;
		} else if (word == "true") {
			result.push_back(Token::literal_int(1));
			found_result = true;
		} else if (word == "false") {
			result.push_back(Token::literal_int(0));
			found_result = true;
		} else if (auto builtin_func = BuiltinFunctions.find(word); builtin_func != BuiltinFunctions.end()) {
			result.push_back(Token::function_immediate(word, builtin_func->second));
			found_result = true;
		} else if (auto func = story_var_info.immediate_functions.find(word); func != story_var_info.immediate_functions.end()) {
			result.push_back(Token::function_immediate(word, func->second));
			found_result = true;
		} else if (story_var_info.deferred_functions.contains(word)) {
			result.push_back(Token::function_deferred(word));
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
						result.push_back(Token::literal_float(word_float));
						found_result = true;
					} catch (...) {
						
					}
				} else {
					try {
						std::int64_t word_int = std::stoll(word);
						result.push_back(Token::literal_int(word_int));
						found_result = true;
					} catch (...) {
						
					}
				}
			}
		}

		if (!found_result) {
			if (index < expression.length() - 1 && expression[index] == '(') {
				result.push_back(Token::function_story_knot(word));
				found_result = true;
			}
		}
		

		if (!found_result) {
			if (in_knot_name) {
				result.push_back(Token::literal_knotname(word, true));
			} else {
				result.push_back(Token::variable(word));
			}
		}

		word.clear();
	}
}

std::vector<Token> tokenize_expression(const std::string& expression, ExpressionParserV2::StoryVariableInfo& story_variable_info) {
	std::vector<Token> result;
	result.reserve(128);

	std::string current_word;
	current_word.reserve(32);

	bool in_quotes = false;
	std::string current_string;
	current_string.reserve(32);

	bool in_knot_name = false;

	//#define TRY_ADD_WORD() try_add_word(expression, index, result, current_word, functions, in_knot_name, deferred_functions)
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
							//result.push_back(new TokenOperator(TokenOperator::Type::Increment, UnaryType::Postfix));
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
					//} else if (next_char(expression, index) == '<') {
					//	result.push_back(new TokenOperator(TokenOperator::Type::ShiftLeft, UnaryType::NotUnary));
					} else {
						result.push_back(Token::operat(OperatorType::Less, UnaryType::NotUnary));
					}
				} break;
				
				case '>': {
					TRY_ADD_WORD();
					if (next_char(expression, index) == '=') {
						result.push_back(Token::operat(OperatorType::GreaterEqual, UnaryType::NotUnary));
						++index;
					//} else if (next_char(expression, index) == '>') {
					//	result.push_back(new TokenOperator(TokenOperator::Type::ShiftRight, UnaryType::NotUnary));
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
					//} else {
					//	result.push_back(new TokenOperator(TokenOperator::Type::BitAnd, UnaryType::NotUnary));
					//}
				} break;

				case '|': {
					TRY_ADD_WORD();
					if (next_char(expression, index) == '|') {
						result.push_back(Token::operat(OperatorType::Or, UnaryType::NotUnary));
						++index;
					}
					//} else {
					//	result.push_back(new TokenOperator(TokenOperator::Type::BitOr, UnaryType::NotUnary));
					//}
				} break;

				case '^': {
					TRY_ADD_WORD();
					result.push_back(Token::operat(OperatorType::Intersect, UnaryType::NotUnary));
				} break;

				/*case '~': {
					if (next_char(expression, index) > 32) {
						result.push_back(new TokenOperator(TokenOperator::Type::BitNot, UnaryType::Prefix));
					}
				} break;*/

				case '(': {
					TRY_ADD_WORD();
					result.push_back(Token::paren_comma(ParenCommaType::LeftParen));
				} break;

				case ')': {
					TRY_ADD_WORD();
					result.push_back(Token::paren_comma(ParenCommaType::RightParen));
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
				result.push_back(Token::literal_string(current_string));
				current_string.clear();
				in_quotes = false;
			}
		}

		++index;
	}

	TRY_ADD_WORD();

	std::vector<std::pair<Token, std::uint8_t>> arg_count_stack;
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
							arg_count_stack.back().first.function_argument_count = arg_count_stack.back().second;
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

std::vector<Token> shunt(const std::vector<Token>& infix) {
	//std::vector<const Token&> postfix;
	//std::stack<const Token&> stack;
	std::vector<std::reference_wrapper<const Token>> postfix;
	std::stack<std::reference_wrapper<const Token>> stack;

	std::size_t index = 0;
	while (index < infix.size()) {
		const Token& this_token = infix[index];

		switch (this_token.type) {
			case TokenType::LiteralNumberInt:
			case TokenType::LiteralNumberFloat:
			case TokenType::LiteralString:
			case TokenType::LiteralKnotName:
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
				//auto* paren_comma = static_cast<TokenParenComma*>(this_token);
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

/*#define OP_UN_PRE(type, func) case Type::type: {\
		Token::ValueResult operand = stack.top()->get_value(variables, constants, variable_redirects);\
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
		Token::ValueResult right = stack.top()->get_value(variables, constants, variable_redirects);\
		stack.pop();\
		Token::ValueResult left = stack.top()->get_value(variables, constants, variable_redirects);\
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
	} break;*/

#define OP_BIN(type, op) case OperatorType::type: {\
	const Token& right = stack.back();\
	const Token& left = stack[stack.size() - 2];\
	stack.push_back(Token::from_variant(left.value op right.value));\
	stack.pop_back();\
	stack.pop_back();\
} break;

#define OP_UN_PRE(type, op) case OperatorType::type: {\
	const Token& operand = stack.back();\
	stack.push_back(Token::from_variant(op##operand.value));\
	stack.pop_back();\
} break;

ExecuteResult execute_expression_tokens(std::vector<Token>& expression_tokens, StoryVariableInfo& story_variable_info) {
	//TokenStack stack;
	std::vector<Token> stack;
	//std::unordered_set<Token*> tokens_to_dealloc;

	std::size_t index = 0;
	while (index < expression_tokens.size()) {
		Token& this_token = expression_tokens[index];

		switch (this_token.type) {
			case TokenType::LiteralNumberInt:
			case TokenType::LiteralNumberFloat:
			case TokenType::LiteralString:
			case TokenType::LiteralKnotName:
			case TokenType::Variable: {
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

					OP_BIN(And, &&);
					OP_BIN(Or, ||);

					OP_UN_PRE(Negative, -);
					OP_UN_PRE(Not, !);

					case OperatorType::Increment:
					case OperatorType::Decrement: {
						//Token::ValueResult operand = stack.top()->get_value(variables, constants, variable_redirects);
						Token& operand = stack.back();
						/*if (operand.operator_unary_type == OperatorUnaryType::Prefix) {
							stack.push_back(Token::from_variant(operand.operator_type == OperatorType::Increment ? operand.increment(false, story_variable_info) : operand.decrement(false, story_variable_info)));
						} else {
							stack.push_back(Token::from_variant(operand.operator_type == OperatorType::Increment ? operand.increment(true, story_variable_info) : operand.decrement(false, story_variable_info)));
						}*/

						bool postfix = operand.operator_unary_type == OperatorUnaryType::Prefix;
						stack.push_back(Token::from_variant(operand.operator_type == OperatorType::Increment ? operand.increment(postfix, story_variable_info) : operand.decrement(postfix, story_variable_info)));

						/*Token* result = nullptr;
						if (op->data.unary_type == TokenOperator::UnaryType::Prefix) {
							result = op->data.type == Type::Increment ? operand.token->operator_inc_pre() : operand.token->operator_dec_pre();
						} else {
							result = op->data.type == Type::Increment ? operand.token->operator_inc_post() : operand.token->operator_dec_post();
						}*/

						/*if (operand.from_variable && op->data.unary_type == TokenOperator::UnaryType::Postfix) {
							TokenNumberInt add{op->data.type == Type::Increment ? 1 : -1};
							Token* new_result = result->operator_plus(&add);

							const std::string& var_name = static_cast<TokenVariable*>(stack.top())->data;
							variables[var_name] = new_result->get_variant_value(variables, constants, variable_redirects).value();
							variables.flag_variable_changed(var_name);
							delete operand.token;
							delete new_result;
						}*/

						//stack.pop();
						//stack.push(result);
						stack.erase(stack.end() - 2);

						/*if (op->data.unary_type == TokenOperator::UnaryType::Postfix) {
							tokens_to_dealloc.insert(result);
						}*/
					} break;

					case OperatorType::Assign: {
						Token& value = stack.back();
						//stack.pop();
						Token& var = stack[stack.size() - 2];
						//stack.pop();
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

						/*if (value->get_type() == TokenType::Variable) {
							std::string value_name = static_cast<TokenVariable*>(value)->data;
							if (var_name == value_name) {
								break;
							}
						}

						// HACK: do this a way better way
						while (true) {
							bool found_any = false;
							for (auto& entry : variable_redirects) {
								while (entry.second.contains(var_name)) {
									var_name = entry.second.at(var_name);
									found_any = true;
								}
							}

							if (!found_any) {
								break;
							}
						}

						if (std::optional<Variant> var_value = value->get_variant_value(variables, constants, variable_redirects); var_value.has_value()) {
							bool changed = var_value != variables[var_name];
							variables[var_name] = *var_value;

							if (changed) {
								variables.flag_variable_changed(var_name);
							}
						} else {
							throw std::runtime_error("Variable being assigned has no value");
						}*/
					} break;

					default: {

					} break;
				}
			} break;

			case TokenType::Function: {
				/*auto* token_func = static_cast<TokenFunction*>(this_token);
				if (token_func->data.fetch_method == TokenFunction::FetchMethod::StoryKnot) {
					std::vector<Token*> func_args;
					for (std::uint8_t i = 0; i < token_func->data.argument_count; ++i) {
						func_args.push_back(stack.top(token_func->data.argument_count - i - 1));
					}

					return std::unexpected(NulloptResult(NulloptResult::Reason::FoundKnotFunction, token_func, index, func_args, tokens_to_dealloc));
				}

				if (Token* result = token_func->call(stack, functions, variables, constants, variable_redirects)) {
					stack.push(result);
					tokens_to_dealloc.insert(result);
				}*/

				std::vector<Token> func_args;
				for (std::uint8_t i = 0; i < this_token.function_argument_count; ++i) {
					func_args.push_back(stack[stack.size() - this_token.function_argument_count - i - 2]);
				}

				if (this_token.function_fetch_type == FunctionFetchType::StoryKnot) {
					return std::unexpected(NulloptResult(NulloptResult::Reason::FoundKnotFunction, this_token, index, func_args));
				}

				std::vector<Variant> arg_values;
				for (const Token& token : func_args) {
					arg_values.push_back(token.value);
				}

				if (Variant result = this_token.call_function(arg_values); result.has_value()) {
					stack.push_back(Token::from_variant(result));
				}
			} break;

			default: {

			} break;
		}

		++index;
	}

	/*std::optional<Variant> result{};
	if (!stack.empty()) {
		result = stack.top()->get_variant_value(variables, constants, variable_redirects);
	}*/

	if (!stack.empty() && stack.back().value.has_value()) {
		return stack.back().value;
	}

	return std::unexpected(NulloptResult(NulloptResult::Reason::NoReturnValue));

	/*for (Token* token : tokens_to_dealloc) {
		delete token;
	}*/

	/*if (result.has_value()) {
		return *result;
	} else {
		return std::unexpected(NulloptResult(NulloptResult::Reason::NoReturnValue));
	}*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ExecuteResult execute_expression(const std::string& expression, StoryVariableInfo& story_variable_info) {
	//std::unordered_set<Token*> dummy;
	std::vector<Token> tokenized = tokenize_expression(expression, story_variable_info);
	std::vector<Token> shunted = shunt(tokenized);

	ExecuteResult result = execute_expression_tokens(shunted, story_variable_info);

	return result;
}

/*ExecuteResult execute_expression(const std::string& expression, VariableMap& variables, const VariableMap& constants, RedirectMap& variable_redirects, const FunctionMap& functions, const std::unordered_set<std::string>& deferred_functions) {
	std::unordered_set<Token*> dummy;
	std::vector<Token*> tokenized = tokenize_expression(expression, functions, deferred_functions);
	std::vector<Token*> shunted = shunt(tokenized, dummy);

	ExecuteResult result = execute_expression_tokens(shunted, variables, constants, variable_redirects, functions);

	for (Token* token : tokenized) {
		delete token;
	}

	return result;
}*/

ShuntedExpression tokenize_and_shunt_expression(const std::string& expression, StoryVariableInfo& story_variable_info) {
	std::vector<Token> tokenized = tokenize_expression(expression, story_variable_info);
	std::vector<Token> shunted = shunt(tokenized);

	return ShuntedExpression(shunted);
}
