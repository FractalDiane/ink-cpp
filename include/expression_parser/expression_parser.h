#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <stack>

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

struct Token {
	virtual ~Token() = default;

	virtual TokenType get_type() const = 0;

	virtual bool as_bool() const;
	virtual std::int64_t as_int() const;
	virtual double as_float() const;
	virtual const std::string& as_string() const;

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

typedef Token* (*PtrTokenFunc)(std::stack<Token*>&);

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
};

struct TokenBoolean : public Token {
	bool data;

	TokenBoolean(bool value) : data{value} {}

	virtual TokenType get_type() const override { return TokenType::Boolean; }

	virtual bool as_bool() const override;
	virtual std::int64_t as_int() const override;
	virtual double as_float() const override;

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

	virtual bool as_bool() const override;
	virtual std::int64_t as_int() const override;
	virtual double as_float() const override;

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

	virtual bool as_bool() const override;
	virtual std::int64_t as_int() const override;
	virtual double as_float() const override;

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

	virtual bool as_bool() const override;
	virtual const std::string& as_string() const override;

	virtual TokenType get_type() const override { return TokenType::StringLiteral; }

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

	virtual TokenType get_type() const override { return TokenType::KnotName; }
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

	virtual TokenType get_type() const override { return TokenType::Operator; }
};

struct TokenParenComma : public Token {
	enum class Type {
		LeftParen,
		RightParen,
		Comma,
	} data;

	TokenParenComma(Type type) : data{type} {}

	virtual TokenType get_type() const override { return TokenType::ParenComma; }
};

struct TokenFunction : public Token {
	/*struct Data {
		std::string function;
		std::vector<Token*> arguments;
	} data;*/
	PtrTokenFunc data;

	TokenFunction(PtrTokenFunc function) : data{function} {}

	virtual TokenType get_type() const override { return TokenType::Function; }
};

struct TokenVariable : public Token {
	std::string data;

	TokenVariable(const std::string& name) : data{name} {}

	virtual TokenType get_type() const override { return TokenType::Variable; }
};

struct PackedToken {
	Token* token;

	PackedToken() : token{nullptr} {}
	PackedToken(Token* token) : token{token} {}
	PackedToken(bool from) : token{new TokenBoolean(from)} {}
	PackedToken(std::int64_t from) : token{new TokenNumberInt(from)} {}
	PackedToken(double from) : token{new TokenNumberFloat(from)} {}
	PackedToken(const std::string& from) : token{new TokenStringLiteral(from)} {}
	PackedToken(std::string&& from) : token{new TokenStringLiteral(from)} {}

	~PackedToken() { delete token; }
	
	PackedToken(PackedToken& from) = delete;
	PackedToken(PackedToken&& from) {
		if (token) {
			delete token;
		}

		token = from.token;
		from.token = nullptr;
	}

	PackedToken& operator=(PackedToken& other) = delete;
	PackedToken& operator=(PackedToken&& other) {
		if (this != &other) {
			if (token) {
				delete token;
			}
			
			token = other.token;
			other.token = nullptr;
		}

		return *this;
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
};

typedef std::unordered_map<std::string, PackedToken> TokenMap;
typedef std::unordered_map<std::string, PtrTokenFunc> FunctionMap;

std::vector<Token*> tokenize_expression(const std::string& expression, const FunctionMap& all_functions);

std::vector<Token*> shunt(const std::vector<Token*>& infix);

Token* execute_expression_tokens(const std::vector<Token*>& tokens, TokenMap& variables);

PackedToken execute_expression(const std::string& expression, const FunctionMap& functions = {});
PackedToken execute_expression(const std::string& expression, TokenMap& variables, const FunctionMap& functions = {});

}
