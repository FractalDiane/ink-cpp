#pragma once

#include "ink_enums.h"
#include "runtime/ink_story.h"
#include "runtime/ink_story_data.h"

#include "objects/ink_object_choice.h"

#include <string>
#include <vector>
#include <format>

class InkLexer {
public:
	struct Token {
		InkToken token;
		std::string text_contents;
		std::uint8_t count;

		Token() : token{InkToken::INVALID}, text_contents{}, count{1} {}
		Token(InkToken token, const std::string& text) : token{token}, text_contents{text}, count{1} {}
		Token(InkToken token, const std::string& text, std::uint8_t count) : token{token}, text_contents{text}, count{count} {}

		std::string to_string() const {
			if (token == InkToken::Text) {
				return text_contents;
			} else {
				return std::format("{}({})", static_cast<int>(token), count);
			}
		}
	};

	InkLexer() = default;
	~InkLexer() = default;

	std::vector<Token> lex_script(const std::string& script_text);
};

class InkCompiler {
private:
	/*struct Stitch {
		std::string name;
		std::size_t index;
	};

	struct Knot {
		std::string name;
		std::vector<InkObject*> objects;
		std::vector<Stitch> stitches;
	};*/

	std::size_t token_index = 0;
	InkObject* last_token_object = nullptr;

	std::size_t brace_level = 0;
	bool in_choice_line = false;
	bool at_line_start = true;
	bool past_choice_initial_braces = false;
	bool in_parens = false;

	std::size_t choice_level = 0;
	std::vector<InkChoiceEntry> choice_stack;

	std::size_t current_sequence_index = 0;

	std::uint32_t current_uuid = 0;
	
public:
	InkStory compile_script(const std::string& script);
	InkStory compile_file(const std::string& file_path);

	void save_data_to_file(InkStoryData* story_data, const std::string& out_file_path);
	void compile_script_to_file(const std::string& script, const std::string& out_file_path);
	void compile_file_to_file(const std::string& in_file_path, const std::string& out_file_path);

private:
	void init_compiler();

	InkStoryData* compile(const std::string& script);
	InkObject* compile_token(const std::vector<InkLexer::Token>& all_tokens, const InkLexer::Token& token, std::vector<Knot>& story_knots);

	static std::vector<InkLexer::Token> remove_comments(const std::vector<InkLexer::Token>& tokens);
	static InkLexer::Token next_token(const std::vector<InkLexer::Token>& tokens, std::size_t index);
	static bool next_token_is(const std::vector<InkLexer::Token>& tokens, std::size_t index, InkToken what);
	static bool next_token_is_sequence(const std::vector<InkLexer::Token>& tokens, std::size_t index, std::vector<InkToken>&& what);
};
