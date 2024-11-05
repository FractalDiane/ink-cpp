#pragma once

enum class InkToken : char {
	INVALID,

	Text,
	NewLine,

	Slash,
	Backslash,

	Hash,

	Asterisk,
	Plus,
	LeftBracket,
	RightBracket,

	Equal,
	Arrow,
	BackArrow,
	Dash,
	Diamond,

	LeftBrace,
	RightBrace,
	Colon,
	Pipe,
	Ampersand,
	Bang,
	Tilde,
	Comma,

	LeftParen,
	RightParen,

	KeywordVar,
	KeywordConst,
	KeywordFunction,
	KeywordInclude,
	KeywordList,
	KeywordExternal,
};
