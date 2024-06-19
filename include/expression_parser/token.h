#pragma once

#include "types/ink_list.h"
#include "serialization.h"

#include <variant>
#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace ExpressionParserV2 {

enum {
	Variant_Bool,
	Variant_Int,
	Variant_Float,
	Variant_String,
	Variant_List,
};

#define i64 std::int64_t
using VariantValue = std::variant<bool, i64, double, std::string, InkList>;

class Variant {
private:
	VariantValue value;
	bool _has_value;

public:
	Variant();

	Variant(bool val);
	Variant(signed char val);
	Variant(unsigned char val);
	Variant(signed short val);
	Variant(unsigned short val);
	Variant(signed int val);
	Variant(unsigned int val);
	Variant(signed long val);
	Variant(unsigned long val);
	Variant(signed long long val);
	Variant(unsigned long long val);
	Variant(float val);
	Variant(double val);

	Variant(const std::string& val);
	//Variant(const VariantValue& val);
	
	Variant(const Variant& from);
	Variant& operator=(const Variant& from);

	inline bool has_value() const { return _has_value; }
	inline std::size_t index() const { return value.index(); }
	std::string to_printable_string() const;

	Variant operator+(const Variant& rhs) const;
	Variant operator-(const Variant& rhs) const;
	Variant operator*(const Variant& rhs) const;
	Variant operator/(const Variant& rhs) const;
	Variant operator%(const Variant& rhs) const;

	Variant operator==(const Variant& rhs) const;
	Variant operator!=(const Variant& rhs) const;
	Variant operator<(const Variant& rhs) const;
	Variant operator>(const Variant& rhs) const;
	Variant operator<=(const Variant& rhs) const;
	Variant operator>=(const Variant& rhs) const;

	Variant operator&&(const Variant& rhs) const;
	Variant operator||(const Variant& rhs) const;

	Variant operator-() const;
	Variant operator!() const;

	Variant& operator++();
	Variant operator++(int);
	Variant& operator--();
	Variant operator--(int);

	Variant operator_contains(const Variant& rhs) const;
	Variant operator_intersect(const Variant& rhs) const;

	operator bool() const;
	operator i64() const;
	operator double() const;
	operator std::string() const;
};

//using VariableObserverFunc = std::function<void(const std::string&, const Variant&)>;
using InkFunction = std::function<Variant(const std::vector<Variant>& args)>;
typedef void (*VariableObserverFunc)(const std::string&, const Variant&);

struct StoryVariableInfo {
	std::unordered_map<std::string, Variant> variables;
	std::unordered_map<std::string, Variant> constants;
	std::unordered_map<std::string, std::string> redirects;

	// HACK: find some better way to store these+argument counts
	std::unordered_map<std::string, std::pair<InkFunction, std::uint8_t>> builtin_functions;
	std::unordered_map<std::string, InkFunction> external_functions;

	std::unordered_map<std::string, std::vector<VariableObserverFunc>> observers;

	std::optional<Variant> get_variable_value(const std::string& variable) const;
	void set_variable_value(const std::string& variable, const Variant& value);

	void observe_variable(const std::string& variable_name, VariableObserverFunc callback);

	void unobserve_variable(const std::string& variable_name);

	void unobserve_variable(VariableObserverFunc observer);

	void unobserve_variable(const std::string& variable_name, VariableObserverFunc observer);

	inline void flag_variable_changed(const std::string& variable) {
		execute_variable_observers(variable, variables[variable]);
	}

private:
	void execute_variable_observers(const std::string& variable, const Variant& new_value);
};

enum class TokenType {
	Nil,
	Keyword,
	LiteralBool,
	LiteralNumberInt,
	LiteralNumberFloat,
	LiteralString,
	LiteralKnotName,
	Operator,
	ParenComma,
	Function,
	Variable,
};

enum class KeywordType {
	Temp,
	Return,

	True,
	False,

	And,
	Or,
	Not,
	Mod,
};

enum class OperatorType {
	Plus,
	Minus,
	Multiply,
	Divide,
	Modulus,
	Substring,
	Intersect,

	Increment,
	Decrement,
	Negative,

	Assign,

	Equal,
	NotEqual,
	Less,
	Greater,
	LessEqual,
	GreaterEqual,

	And,
	Or,
	Xor,
	Not,

	BitAnd,
	BitOr,
	BitXor,
	BitNot,
	ShiftLeft,
	ShiftRight,
};

enum class OperatorUnaryType {
	NotUnary,
	Prefix,
	Postfix,
};

enum class ParenCommaType {
	LeftParen,
	RightParen,
	Comma,
};

enum class FunctionFetchType {
	Builtin,
	External,
	StoryKnot,
};

//using TokenValue = std::variant<KeywordType, i64, double, std::string, OperatorType, ParenCommaType>;

struct Token {
	TokenType type;
	Variant value = (std::int64_t)0;

	bool knot_name_has_arrow = false;

	KeywordType keyword_type = KeywordType::Temp;
	OperatorType operator_type = OperatorType::And;
	OperatorUnaryType operator_unary_type = OperatorUnaryType::NotUnary;
	ParenCommaType paren_comma_type = ParenCommaType::LeftParen;

	InkFunction function;
	FunctionFetchType function_fetch_type = FunctionFetchType::StoryKnot;
	std::uint8_t function_argument_count = 0;

	std::string variable_name;

	/*Token(const Variant& from_variant) : value(from_variant) {
		switch (from_variant.index()) {
			case Variant_Int:
				type = TokenType::LiteralNumberInt;
				break;
			case Variant_Float:
				type = TokenType::LiteralNumberFloat;
				break;
			case Variant_String:
				type = TokenType::LiteralString;
				break;
			default: break;
		}
	}*/

	static Token from_variant(const Variant& var) {
		Token result = {.value = var};
		switch (var.index()) {
			case Variant_Bool:
				result.type = TokenType::LiteralBool;
				break;
			case Variant_Int:
				result.type = TokenType::LiteralNumberInt;
				break;
			case Variant_Float:
				result.type = TokenType::LiteralNumberFloat;
				break;
			case Variant_String:
				result.type = TokenType::LiteralString;
				break;
			default: break;
		}

		return result;
	}

	static Token nil() {
		return {.type = TokenType::Nil, .value = (std::int64_t)0};
	}

	static Token keyword(KeywordType keyword_type) {
		return {.type = TokenType::Keyword, .keyword_type = keyword_type};
	}

	static Token literal_bool(bool val) {
		return {.type = TokenType::LiteralBool, .value = val};
	}

	static Token literal_int(i64 val) {
		return {.type = TokenType::LiteralNumberInt, .value = val};
	}

	static Token literal_float(double val) {
		return {.type = TokenType::LiteralNumberFloat, .value = val};
	}

	static Token literal_string(const std::string& val) {
		return {.type = TokenType::LiteralString, .value = val};
	}

	static Token literal_knotname(const std::string& val, bool has_arrow) {
		return {.type = TokenType::LiteralKnotName, .value = val, .knot_name_has_arrow = has_arrow};
	}

	static Token operat(OperatorType op_type, OperatorUnaryType unary_type) {
		return {.type = TokenType::Operator, .operator_type = op_type, .operator_unary_type = unary_type};
	}

	static Token paren_comma(ParenCommaType paren_comma_type) {
		return {.type = TokenType::ParenComma, .paren_comma_type = paren_comma_type};
	}

	static Token function_builtin(const std::string& function_name, InkFunction function, std::uint8_t arg_count = 0) {
		return {.type = TokenType::Function, .value = function_name, .function = function, .function_fetch_type = FunctionFetchType::Builtin, .function_argument_count = arg_count};
	}

	static Token function_external(const std::string& function_name, std::uint8_t arg_count = 0) {
		return {.type = TokenType::Function, .value = function_name, .function_fetch_type = FunctionFetchType::External, .function_argument_count = arg_count};
	}

	static Token function_story_knot(const std::string& function_name, std::uint8_t arg_count = 0) {
		return {.type = TokenType::Function, .value = function_name, .function_fetch_type = FunctionFetchType::StoryKnot, .function_argument_count = arg_count};
	}

	static Token variable(const std::string& var_name) {
		return {.type = TokenType::Variable, .variable_name = var_name};
	}

	void fetch_variable_value(const StoryVariableInfo& story_vars);
	void store_variable_value(StoryVariableInfo& story_vars);
	void fetch_function_value(const StoryVariableInfo& story_vars);

	Variant increment(bool post, StoryVariableInfo& story_vars);
	Variant decrement(bool post, StoryVariableInfo& story_vars);

	void assign_variable(const Token& other, StoryVariableInfo& story_vars);

	Variant call_function(const std::vector<Variant>& arguments, const StoryVariableInfo& story_variable_info);
};

}

template <>
struct Serializer<ExpressionParserV2::Variant> {
	ByteVec operator()(const ExpressionParserV2::Variant& token);
};

template <>
struct Deserializer<ExpressionParserV2::Variant> {
	ExpressionParserV2::Variant operator()(const ByteVec& bytes, std::size_t& index);
};

template <>
struct Serializer<ExpressionParserV2::Token> {
	ByteVec operator()(const ExpressionParserV2::Token& token);
};

template <>
struct Deserializer<ExpressionParserV2::Token> {
	ExpressionParserV2::Token operator()(const ByteVec& bytes, std::size_t& index);
};

#undef i64
