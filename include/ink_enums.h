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

enum class InkObjectField : char {
	Type,
	Contents,
	Tags,

	Root_Knots,

	Knot_Stitches,

	Choice_Text,
	Choice_Result,
	Choice_Sticky,
	Choice_Conditions,
	Choice_Fallback,

	Conditional_Condition,
	Conditional_If,
	Conditional_Else,

	Alternate_Items,
	Alternate_Id,

	Variable_Name,
	Variable_Value,
};

enum class InkObjectType : char {
	INVALID,

	Text,
	Choice,

	LineBreak,
	Glue,
	Divert,
	ChoiceTextMixStart,
	ChoiceTextMixEnd,

	Tag,

	Interpolation,
	Conditional,
	Sequence,
	Cycle,
	OnceOnly,
	Shuffle,
	ShuffleOnce,
	ShuffleStop,

	Logic,
	GlobalVariable,
};
