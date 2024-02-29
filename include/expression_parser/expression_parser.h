#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>

namespace ExpressionParser {

/*enum class TokenType {
	Operator_Plus,
	Operator_Minus,
	Operator_Multiply,
	Operator_Divide,
	Operator_Modulus,

	Operator_Prefix_Negative,
	Operator_Prefix_Increment,
	Operator_Prefix_Decrement,

	Operator_Postfix_Increment,
	Operator_Postfix_Decrement,

	Operator_Assign,

	Operator_Equal,
	Operator_NotEqual,
	Operator_Less,
	Operator_Greater,
	Operator_LessEqual,
	Operator_GreaterEqual,

	Operator_And,
	Operator_Or,
	Operator_Xor,
	Operator_Not,

	LeftParen,
	RightParen,
	Comma,

	KeywordTemp,

	VariableName,
	Boolean,
	Integer,
	Float,
	String,
	Function,
};*/

enum class TokenType {
	Keyword,
	Boolean,
	NumberInt,
	NumberFloat,
	StringLiteral,
	KnotName,
	Operator,
	Function,
	Variable,
};

struct Token {
	virtual ~Token() = default;

	virtual TokenType get_type() const = 0;

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
	virtual Token* operator_negative() const;

	virtual Token* operator_equal(const Token* other) const;
	virtual Token* operator_notequal(const Token* other) const;
	virtual Token* operator_less(const Token* other) const;
	virtual Token* operator_greater(const Token* other) const;
	virtual Token* operator_lessequal(const Token* other) const;
	virtual Token* operator_greaterequal(const Token* other) const;

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
};

struct TokenBoolean : public Token {
	bool data;

	TokenBoolean(bool value) : data{value} {}

	virtual TokenType get_type() const override { return TokenType::Boolean; }

	virtual Token* operator_not() const override;
	virtual Token* operator_equal(const Token* other) const override;
	virtual Token* operator_notequal(const Token* other) const override;
};

struct TokenNumberInt : public Token {
	std::int64_t data;

	TokenNumberInt(std::int64_t value) : data{value} {}

	virtual TokenType get_type() const override { return TokenType::NumberInt; }

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

	virtual TokenType get_type() const override { return TokenType::StringLiteral; }

	virtual Token* operator_plus(const Token* other) const override;
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

		LeftParen,
		RightParen,
		Comma,

		And,
		Or,
		Xor,
		Not,

		BitAnd,
		BitOr,
		BitXor,
		ShiftLeft,
		ShiftRight,
	};

	struct Data {
		Type type;
		bool unary;
	} data;

	TokenOperator(Type type, bool unary) : data{type, unary} {}

	virtual TokenType get_type() const override { return TokenType::Operator; }
};

struct TokenFunction : public Token {
	struct Data {
		std::string function;
		std::vector<Token*> arguments;
	} data;
	

	TokenFunction(const std::string& function, const std::vector<Token*>& arguments) : data{function, arguments} {}

	virtual ~TokenFunction() override {
		for (Token* token : data.arguments) {
			delete token;
		}
	}

	virtual TokenType get_type() const override { return TokenType::Function; }
};

struct TokenVariable : public Token {
	std::string data;

	TokenVariable(const std::string& name) : data{name} {}

	virtual TokenType get_type() const override { return TokenType::Variable; }
};

class PackedToken {
private:
	Token* token;

public:
	PackedToken(Token* token) : token{token} {}
	~PackedToken() { delete token; }

	bool as_bool() const {
		if (token->get_type() == TokenType::Boolean) {
			return static_cast<TokenBoolean*>(token)->data;
		} else {
			throw;
		}
	}

	std::int64_t as_int() const {
		if (token->get_type() == TokenType::NumberInt) {
			return static_cast<TokenNumberInt*>(token)->data;
		} else {
			throw;
		}
	}

	double as_float() const {
		if (token->get_type() == TokenType::NumberFloat) {
			return static_cast<TokenNumberFloat*>(token)->data;
		} else {
			throw;
		}
	}

	const std::string& as_string() const {
		if (token->get_type() == TokenType::StringLiteral) {
			return static_cast<const TokenStringLiteral*>(token)->data;
		} else {
			throw;
		}
	}
};

typedef std::unordered_map<std::string, Token*> TokenMap;

std::vector<Token*> tokenize_expression(const std::string& expression);

std::vector<Token*> shunt(const std::vector<Token*>& infix);

Token* execute_expression_tokens(const std::vector<Token*>& tokens, TokenMap& variables);

PackedToken execute_expression(const std::string& expression);
PackedToken execute_expression(const std::string& expression, TokenMap& variables);

}
