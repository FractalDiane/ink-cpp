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

namespace ExpressionParser {

enum class TokenType {
	Keyword,
	Boolean,
	NumberInt,
	NumberFloat,
	StringLiteral,
	KnotName,
	Operator,
	ParenComma,
	Function,
	Variable,
};

enum {
	Variant_Bool,
	Variant_Int,
	Variant_Float,
	Variant_String,
};

struct Token;
struct TokenFunction;
struct InternalPackedToken;

template <typename T>
class Stack {
private:
	std::deque<T> deque;

public:
	void push(const T& value) { deque.push_back(value); }
	void push(T&& value) { deque.push_back(value); }
	T& top(std::size_t index = 0) noexcept { return deque[deque.size() - index - 1]; }
	void pop() { deque.pop_back(); }

	std::size_t size() const { return deque.size(); }
	bool empty() const { return deque.empty(); }
};

using Variant = std::variant<bool, std::int64_t, double, std::string, const TokenFunction*>;

bool as_bool(const Variant& variant);
std::int64_t as_int(const Variant& variant);
double as_float(const Variant& variant);
std::string as_string(const Variant& variant);

std::string to_printable_string(const Variant& variant);

using TokenStack = Stack<Token*>;

typedef std::unordered_map<std::string, Variant> VariableMap;
using PtrTokenFunc = std::function<Token*(TokenStack&, VariableMap&, const VariableMap&)>;
typedef std::unordered_map<std::string, PtrTokenFunc> FunctionMap;

struct Token {
	virtual ~Token() = default;

	virtual TokenType get_type() const = 0;
	virtual Token* copy() const = 0;

	virtual bool as_bool() const;
	virtual std::int64_t as_int() const;
	virtual double as_float() const;
	virtual const std::string& as_string() const;

	virtual std::string to_printable_string() const;

	struct ValueResult {
		Token* token;
		bool from_variable;
	};

	virtual ValueResult get_value(const VariableMap& variables, const VariableMap& constants) { return {this, false}; }
	virtual std::optional<Variant> get_variant_value(const VariableMap& variables, const VariableMap& constants) const { return {}; }

	virtual Token* operator_plus(const Token* other) const;
	virtual Token* operator_minus(const Token* other) const;
	virtual Token* operator_multiply(const Token* other) const;
	virtual Token* operator_divide(const Token* other) const;
	virtual Token* operator_mod(const Token* other) const;

	virtual Token* operator_inc_pre();
	virtual Token* operator_inc_post();
	virtual Token* operator_dec_pre();
	virtual Token* operator_dec_post();
	virtual Token* operator_not() const;
	virtual Token* operator_bitnot() const;
	virtual Token* operator_negative() const;

	virtual Token* operator_equal(const Token* other) const;
	virtual Token* operator_notequal(const Token* other) const;
	virtual Token* operator_less(const Token* other) const;
	virtual Token* operator_greater(const Token* other) const;
	virtual Token* operator_lessequal(const Token* other) const;
	virtual Token* operator_greaterequal(const Token* other) const;

	virtual Token* operator_and(const Token* other) const;
	virtual Token* operator_or(const Token* other) const;
	
	virtual Token* operator_bitand(const Token* other) const;
	virtual Token* operator_bitor(const Token* other) const;
	virtual Token* operator_bitxor(const Token* other) const;
	virtual Token* operator_shiftleft(const Token* other) const;
	virtual Token* operator_shiftright(const Token* other) const;
	
	virtual Token* operator_substring(const Token* other) const;
};

struct TokenKeyword : public Token {
	enum class Type {
		Temp,

		True,
		False,

		And,
		Or,
		Not,
		Mod,
	} data;

	TokenKeyword(Type type) : data{type} {}

	virtual TokenType get_type() const override { return TokenType::Keyword; }

	virtual Token* copy() const override { return new TokenKeyword(data); }
};

struct TokenBoolean : public Token {
	bool data;

	TokenBoolean(bool value) : data{value} {}

	virtual TokenType get_type() const override { return TokenType::Boolean; }
	virtual std::optional<Variant> get_variant_value(const VariableMap& variables, const VariableMap& constants) const override { return data; }

	virtual Token* copy() const override { return new TokenBoolean(data); }

	virtual bool as_bool() const override;
	virtual std::int64_t as_int() const override;
	virtual double as_float() const override;

	virtual std::string to_printable_string() const override;

	virtual Token* operator_not() const override;
	virtual Token* operator_equal(const Token* other) const override;
	virtual Token* operator_notequal(const Token* other) const override;

	virtual Token* operator_and(const Token* other) const override;
	virtual Token* operator_or(const Token* other) const override;
	virtual Token* operator_bitand(const Token* other) const override;
	virtual Token* operator_bitor(const Token* other) const override;
	virtual Token* operator_bitxor(const Token* other) const override;
};

struct TokenNumberInt : public Token {
	std::int64_t data;

	TokenNumberInt(std::int64_t value) : data{value} {}

	virtual TokenType get_type() const override { return TokenType::NumberInt; }
	virtual std::optional<Variant> get_variant_value(const VariableMap& variables, const VariableMap& constants) const override { return data; }

	virtual Token* copy() const override { return new TokenNumberInt(data); }

	virtual bool as_bool() const override;
	virtual std::int64_t as_int() const override;
	virtual double as_float() const override;

	virtual std::string to_printable_string() const override;

	virtual Token* operator_plus(const Token* other) const override;
	virtual Token* operator_minus(const Token* other) const override;
	virtual Token* operator_multiply(const Token* other) const override;
	virtual Token* operator_divide(const Token* other) const override;
	virtual Token* operator_mod(const Token* other) const override;

	virtual Token* operator_inc_pre() override;
	virtual Token* operator_inc_post() override;
	virtual Token* operator_dec_pre() override;
	virtual Token* operator_dec_post() override;
	virtual Token* operator_negative() const override;
	virtual Token* operator_not() const override;

	virtual Token* operator_equal(const Token* other) const override;
	virtual Token* operator_notequal(const Token* other) const override;
	virtual Token* operator_less(const Token* other) const override;
	virtual Token* operator_greater(const Token* other) const override;
	virtual Token* operator_lessequal(const Token* other) const override;
	virtual Token* operator_greaterequal(const Token* other) const override;

	virtual Token* operator_and(const Token* other) const override;
	virtual Token* operator_or(const Token* other) const override;
	virtual Token* operator_bitand(const Token* other) const override;
	virtual Token* operator_bitor(const Token* other) const override;
	virtual Token* operator_bitxor(const Token* other) const override;
};

struct TokenNumberFloat : public Token {
	double data;

	TokenNumberFloat(double value) : data{value} {}

	virtual TokenType get_type() const override { return TokenType::NumberFloat; }
	virtual std::optional<Variant> get_variant_value(const VariableMap& variables, const VariableMap& constants) const override { return data; }

	virtual Token* copy() const override { return new TokenNumberFloat(data); }

	virtual bool as_bool() const override;
	virtual std::int64_t as_int() const override;
	virtual double as_float() const override;

	virtual std::string to_printable_string() const override;

	virtual Token* operator_plus(const Token* other) const override;
	virtual Token* operator_minus(const Token* other) const override;
	virtual Token* operator_multiply(const Token* other) const override;
	virtual Token* operator_divide(const Token* other) const override;
	virtual Token* operator_mod(const Token* other) const override;
	virtual Token* operator_not() const override;

	virtual Token* operator_inc_pre() override;
	virtual Token* operator_inc_post() override;
	virtual Token* operator_dec_pre() override;
	virtual Token* operator_dec_post() override;
	virtual Token* operator_negative() const override;

	virtual Token* operator_equal(const Token* other) const override;
	virtual Token* operator_notequal(const Token* other) const override;
	virtual Token* operator_less(const Token* other) const override;
	virtual Token* operator_greater(const Token* other) const override;
	virtual Token* operator_lessequal(const Token* other) const override;
	virtual Token* operator_greaterequal(const Token* other) const override;

	virtual Token* operator_and(const Token* other) const override;
	virtual Token* operator_or(const Token* other) const override;
};

struct TokenStringLiteral : public Token {
	std::string data;

	TokenStringLiteral(const std::string& data) : data{data} {}

	virtual Token* copy() const override { return new TokenStringLiteral(data); }

	virtual bool as_bool() const override;
	virtual const std::string& as_string() const override;

	virtual std::string to_printable_string() const override;

	virtual TokenType get_type() const override { return TokenType::StringLiteral; }
	virtual std::optional<Variant> get_variant_value(const VariableMap& variables, const VariableMap& constants) const override { return data; }

	virtual Token* operator_plus(const Token* other) const override;

	virtual Token* operator_equal(const Token* other) const override;
	virtual Token* operator_notequal(const Token* other) const override;

	virtual Token* operator_substring(const Token* other) const override;
};

struct TokenKnotName : public Token {
	struct Data {
		std::string knot;
		bool has_arrow;
	} data;

	TokenKnotName(const std::string& knot, bool has_arrow) : data{knot, has_arrow} {}

	virtual Token* copy() const override { return new TokenKnotName(data.knot, data.has_arrow); }

	virtual const std::string& as_string() const override;

	virtual std::string to_printable_string() const override;

	virtual TokenType get_type() const override { return TokenType::KnotName; }
	virtual std::optional<Variant> get_variant_value(const VariableMap& variables, const VariableMap& constants) const override { return data.knot; }
};

struct TokenOperator : public Token {
	enum class Type {
		Plus,
		Minus,
		Multiply,
		Divide,
		Modulus,
		Substring,

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

	enum class UnaryType {
		NotUnary,
		Prefix,
		Postfix,
	};

	struct Data {
		Type type;
		UnaryType unary_type;
	} data;

	TokenOperator(Type type, UnaryType unary_type) : data{type, unary_type} {}

	virtual Token* copy() const override { return new TokenOperator(data.type, data.unary_type); }

	virtual TokenType get_type() const override { return TokenType::Operator; }
};

struct TokenParenComma : public Token {
	enum class Type {
		LeftParen,
		RightParen,
		Comma,
	} data;

	TokenParenComma(Type type) : data{type} {}

	virtual Token* copy() const override { return new TokenParenComma(data); }

	virtual TokenType get_type() const override { return TokenType::ParenComma; }
};

struct TokenFunction : public Token {
	struct Data {
		std::string name;
		PtrTokenFunc function;
		bool defer_fetch;
	} data;

	TokenFunction(const std::string& name, PtrTokenFunc function, bool defer_fetch) : data{name, function, defer_fetch} {}

	virtual Token* copy() const override { return new TokenFunction(data.name, data.function, data.defer_fetch); }

	virtual TokenType get_type() const override { return TokenType::Function; }

	Token* call(TokenStack& stack, const FunctionMap& all_functions, VariableMap& variables, const VariableMap& constants);
};

struct TokenVariable : public Token {
	std::string data;

	TokenVariable(const std::string& name) : data{name} {}

	virtual Token* copy() const override { return new TokenVariable(data); }

	virtual TokenType get_type() const override { return TokenType::Variable; }

	virtual ValueResult get_value(const VariableMap& variables, const VariableMap& constants) override;
	virtual std::optional<Variant> get_variant_value(const VariableMap& variables, const VariableMap& constants) const override;
};

std::vector<Token*> tokenize_expression(const std::string& expression, const FunctionMap& all_functions, const std::unordered_set<std::string>& deferred_functions);

std::vector<Token*> shunt(const std::vector<Token*>& infix, std::unordered_set<Token*>& tokens_shunted);

std::optional<Variant> execute_expression_tokens(const std::vector<Token*>& tokens, VariableMap& variables, const VariableMap& constants, const FunctionMap& all_functions);

std::optional<Variant> execute_expression(const std::string& expression, const FunctionMap& functions = {}, const std::unordered_set<std::string>& deferred_functions = {});
std::optional<Variant> execute_expression(const std::string& expression, VariableMap& variables, const VariableMap& constants, const FunctionMap& functions = {}, const std::unordered_set<std::string>& deferred_functions = {});

std::vector<Token*> tokenize_and_shunt_expression(const std::string& expression, const FunctionMap& functions, const std::unordered_set<std::string>& deferred_functions);

}
