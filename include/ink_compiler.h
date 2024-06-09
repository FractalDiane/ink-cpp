#pragma once

#include "ink_enums.h"
#include "runtime/ink_story.h"
#include "runtime/ink_story_data.h"

#include "objects/ink_object_choice.h"

#include <string>
#include <vector>
#include <unordered_set>
#include <format>

class InkLexer {
public:
	struct Token {
		InkToken token;
		std::string text_contents;
		std::uint8_t count;
		bool escaped;

		Token() : token{InkToken::INVALID}, text_contents{}, count{1}, escaped{false} {}
		Token(InkToken token, const std::string& text) : token{token}, text_contents{text}, count{1}, escaped{false} {}
		Token(InkToken token, const std::string& text, std::uint8_t count) : token{token}, text_contents{text}, count{count}, escaped{false} {}

		std::string to_string() const {
			if (token == InkToken::Text) {
				return text_contents;
			} else {
				return std::format("{}({})", static_cast<int>(token), count);
			}
		}

		std::string get_text_contents() const {
			if (token == InkToken::Text) {
				return text_contents;
			} else {
				std::string result;
				result.reserve(text_contents.length() * count);
				for (int i = 0; i < count; ++i) {
					result += text_contents;
				}

				return result;
			}
		}
	};

	InkLexer() = default;
	~InkLexer() = default;

	std::vector<Token> lex_script(const std::string& script_text);
};

class InkCompiler {
private:
	std::size_t token_index = 0;
	InkObject* last_token_object = nullptr;

	std::size_t brace_level = 0;
	bool in_choice_line = false;
	bool at_line_start = true;
	bool past_choice_initial_braces = false;
	bool in_parens = false;
	bool just_added_divert_to_tunnel = false;

	std::size_t choice_level = 0;
	std::vector<InkChoiceEntry> choice_stack;

	std::size_t current_sequence_index = 0;

	UuidValue current_uuid = 0;

	std::unordered_set<std::string> declared_variables;
	std::unordered_set<std::string> declared_functions = {
		"RANDOM", "SEED_RANDOM", "CHOICE_COUNT", "TURNS", "TURNS_SINCE",
	};
	
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
