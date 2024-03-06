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

using PackToken = std::variant<bool, std::int64_t, double, std::string, const TokenFunction*>;

std::string token_to_printable_string(const PackToken& token);

/*class WrappedPackToken {
private:
	PackToken value;

public:
	WrappedPackToken(const PackToken& from) : value{from} {}
	WrappedPackToken(PackToken&& from) : value{from} {}

	bool as_bool() const { return std::get<bool>(value); }
	std::int64_t as_int() const { return std::get<std::int64_t>(value); }
	double as_float() const { return std::get<double>(value); }
	const std::string& as_string() const { return std::get<std::string>(value); }
	const TokenFunction* const as_function() const { return std::get<const TokenFunction* const>(value); }
};*/

using TokenStack = Stack<Token*>;

using PtrTokenFunc = std::function<Token*(TokenStack&)>;
typedef std::unordered_map<std::string, Token*> TokenMap;
typedef std::unordered_map<std::string, PackToken> VariableMap;
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

	virtual Token* get_value(const VariableMap& variables) { return this; }
	virtual std::optional<PackToken> get_variant_value(const VariableMap& variables) const { return {}; }

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

//typedef Token* (*PtrTokenFunc)(std::stack<Token*>&);

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
	virtual std::optional<PackToken> get_variant_value(const VariableMap& variables) const override { return data; }

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
	virtual std::optional<PackToken> get_variant_value(const VariableMap& variables) const override { return data; }

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

	virtual Token* operator_equal(const Token* other) const override;
	virtual Token* operator_notequal(const Token* other) const override;
	virtual Token* operator_less(const Token* other) const override;
	virtual Token* operator_greater(const Token* other) const override;
	virtual Token* operator_lessequal(const Token* other) const override;
	virtual Token* operator_greaterequal(const Token* other) const override;
};

struct TokenNumberFloat : public Token {
	double data;

	TokenNumberFloat(double value) : data{value} {}

	virtual TokenType get_type() const override { return TokenType::NumberFloat; }
	virtual std::optional<PackToken> get_variant_value(const VariableMap& variables) const override { return data; }

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
};

struct TokenStringLiteral : public Token {
	std::string data;

	TokenStringLiteral(const std::string& data) : data{data} {}

	virtual Token* copy() const override { return new TokenStringLiteral(data); }

	virtual bool as_bool() const override;
	virtual const std::string& as_string() const override;

	virtual std::string to_printable_string() const override;

	virtual TokenType get_type() const override { return TokenType::StringLiteral; }
	virtual std::optional<PackToken> get_variant_value(const VariableMap& variables) const override { return data; }

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
	virtual std::optional<PackToken> get_variant_value(const VariableMap& variables) const override { return data.knot; }
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

	Token* call(TokenStack& stack, const FunctionMap& all_functions);
};

struct TokenVariable : public Token {
	std::string data;

	TokenVariable(const std::string& name) : data{name} {}

	virtual Token* copy() const override { return new TokenVariable(data); }

	virtual TokenType get_type() const override { return TokenType::Variable; }

	virtual Token* get_value(const VariableMap& variables) override;
	virtual std::optional<PackToken> get_variant_value(const VariableMap& variables) const override;
};

struct InternalPackedToken {
	Token* token;
	bool owner;

	InternalPackedToken() : token{nullptr}, owner{false} {}
	explicit InternalPackedToken(Token* token, bool owner) : token{token}, owner{owner} {}
	/*InternalPackedToken(bool from) : token{new TokenBoolean(from)}, owner{true} {}
	InternalPackedToken(std::int64_t from) : token{new TokenNumberInt(from)}, owner{true} {}
	InternalPackedToken(int from) : token{new TokenNumberInt(static_cast<std::int64_t>(from))}, owner{true} {}
	InternalPackedToken(double from) : token{new TokenNumberFloat(from)}, owner{true} {}
	InternalPackedToken(const std::string& from) : token{new TokenStringLiteral(from)}, owner{true} {}
	InternalPackedToken(std::string&& from) : token{new TokenStringLiteral(from)}, owner{true} {}*/

	~InternalPackedToken() {
		//if (owner) {
		//	delete token;
		//}
	}

	static InternalPackedToken from_bool(bool from) { return InternalPackedToken(new TokenBoolean(from), true); }
	static InternalPackedToken from_int(std::int64_t from) { return InternalPackedToken(new TokenNumberInt(from), true); }
	static InternalPackedToken from_float(double from) { return InternalPackedToken(new TokenNumberFloat(from), true); }
	static InternalPackedToken from_string(const std::string& from) { return InternalPackedToken(new TokenStringLiteral(from), true); }
	static InternalPackedToken from_string(std::string&& from) { return InternalPackedToken(new TokenStringLiteral(from), true); }

	static InternalPackedToken from_other(InternalPackedToken& from, bool transfer_ownership) {
		if (transfer_ownership) {
			from.owner = false;
		}

		return InternalPackedToken(from.token, transfer_ownership);
	}

	/*explicit InternalPackedToken(InternalPackedToken& from, bool transfer_ownership) : token{from.token}, owner{transfer_ownership} {
		if (transfer_ownership) {
			from.owner = false;
		}
	}*/

	InternalPackedToken(InternalPackedToken&& from) : token{from.token}, owner{from.owner} {
		from.token = nullptr;
		from.owner = false;
	}

	InternalPackedToken& operator=(InternalPackedToken&& other) {
		if (this != &other) {
			token = other.token;
			owner = other.owner;
			other.token = nullptr;
			other.owner = false;
		}

		return *this;
	}

	InternalPackedToken(const InternalPackedToken& from) : token{from.token}, owner{false} {}
	//InternalPackedToken(InternalPackedToken&& from) = delete;
	InternalPackedToken& operator=(const InternalPackedToken& other) {
		if (this != &other) {
			token = other.token;
			owner = false;
		}

		return *this;
	}
	//InternalPackedToken& operator=(InternalPackedToken&& from) = delete;
	
	/*InternalPackedToken(const InternalPackedToken& from) : token{from.token->copy()} {}

	InternalPackedToken(InternalPackedToken&& from) {
		if (token) {
			delete token;
		}

		token = from.token;
		from.token = nullptr;
	}

	InternalPackedToken& operator=(const InternalPackedToken& other) {
		if (this != &other) {
			token = other.token->copy();
		}

		return *this;
	}

	InternalPackedToken& operator=(InternalPackedToken&& other) {
		if (this != &other) {
			if (token) {
				delete token;
			}
			
			token = other.token;
			other.token = nullptr;
		}

		return *this;
	}*/

	bool operator==(const InternalPackedToken& other) const {
		return token->operator_equal(other.token);
	}

	bool operator!=(const InternalPackedToken& other) const {
		return token->operator_notequal(other.token);
	}

	bool as_bool() const {
		return token->as_bool();
	}

	std::int64_t as_int() const {
		return token->as_int();
	}

	double as_float() const {
		return token->as_float();
	}

	const std::string& as_string() const {
		return token->as_string();
	}

	std::string to_printable_string() const {
		return token->to_printable_string();
	}
};

std::vector<Token*> tokenize_expression(const std::string& expression, const FunctionMap& all_functions, const std::unordered_set<std::string>& deferred_functions);

std::vector<Token*> shunt(const std::vector<Token*>& infix, std::unordered_set<Token*>& tokens_shunted);

std::optional<PackToken> execute_expression_tokens(const std::vector<Token*>& tokens, VariableMap& variables, const FunctionMap& all_functions);

std::optional<PackToken> execute_expression(const std::string& expression, const FunctionMap& functions = {}, const std::unordered_set<std::string>& deferred_functions = {});
std::optional<PackToken> execute_expression(const std::string& expression, VariableMap& variables, const FunctionMap& functions = {}, const std::unordered_set<std::string>& deferred_functions = {});

std::vector<Token*> tokenize_and_shunt_expression(const std::string& expression, const FunctionMap& functions, const std::unordered_set<std::string>& deferred_functions);

}
