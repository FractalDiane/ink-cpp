#include "ink_compiler.h"

#include "ink_utils.h"

#include "runtime/ink_story.h"
#include "runtime/ink_story_data.h"

#include "objects/ink_object.h"
#include "objects/ink_object_choice.h"
#include "objects/ink_object_choicetextmix.h"
#include "objects/ink_object_conditional.h"
#include "objects/ink_object_divert.h"
#include "objects/ink_object_globalvariable.h"
#include "objects/ink_object_glue.h"
#include "objects/ink_object_interpolation.h"
#include "objects/ink_object_linebreak.h"
#include "objects/ink_object_logic.h"
#include "objects/ink_object_sequence.h"
#include "objects/ink_object_tag.h"
#include "objects/ink_object_text.h"

#include <unordered_map>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <iostream>
#include <format>
#include <stdexcept>

namespace {
	static const std::unordered_map<char, InkToken> TokenChars = {
		{'/', InkToken::Slash},
		{'\\', InkToken::Backslash},

		{'#', InkToken::Hash},

		{'*', InkToken::Asterisk},
		{'+', InkToken::Plus},

		{'(', InkToken::LeftParen},
		{')', InkToken::RightParen},
		{'[', InkToken::LeftBracket},
		{']', InkToken::RightBracket},
		{'{', InkToken::LeftBrace},
		{'}', InkToken::RightBrace},

		{'=', InkToken::Equal},

		{':', InkToken::Colon},
		{'|', InkToken::Pipe},
		{'&', InkToken::Ampersand},
		{'~', InkToken::Tilde},
	};

	char next_char(const std::string& script_text, size_t index) {
		if (index + 1 < script_text.length()) {
			return script_text[index + 1];
		} else {
			return -1;
		}
	}

	const InkLexer::Token last_token(const std::vector<InkLexer::Token>& tokens) {
		return !tokens.empty() ? tokens.back() : InkLexer::Token();
	}

	void try_add_text_token(std::vector<InkLexer::Token>& result, const std::string& text) {
		InkLexer::Token text_token{InkToken::Text, text};
		if (!text_token.text_contents.empty()) {
			result.push_back(text_token);
		}
	}
}

std::vector<InkLexer::Token> InkLexer::lex_script(const std::string& script_text) {
	std::vector<Token> result;

	std::size_t index = 0;
	std::string current_text;
	current_text.reserve(50);
	bool any_tokens_this_line = false;

	while (index < script_text.length()) {
		Token this_token;
		bool end_text = true;
		char chr = script_text[index];

		this_token.text_contents = chr;
		switch (chr) {
			case '\n': {
				if (last_token(result).token != InkToken::NewLine || !current_text.empty()) {
					this_token.token = InkToken::NewLine;
				}
			} break;

			case '*':
			case '+': {
				this_token.token = chr == '*' ? InkToken::Asterisk : InkToken::Plus;
				++index;
				std::size_t whitespace_skipped = 0;
				while (true) {
					char inner_chr = script_text[index];
					if (inner_chr > 32) {
						if (inner_chr == chr) {
							++this_token.count;
						} else {
							break;
						}
					} else {
						++whitespace_skipped;
					}

					++index;
				}

				--index;
				if (this_token.count == 1) {
					index -= whitespace_skipped;
				}
			} break;

			case '-': {
				if (next_char(script_text, index) == '>') {
					this_token.token = InkToken::Arrow;
					this_token.text_contents = "->";
					++index;
				} else {
					this_token.token = InkToken::Dash;
					++index;
					while (true) {
						char inner_chr = script_text[index];
						if (inner_chr > 32) {
							if (inner_chr == chr) {
								++this_token.count;
							} else {
								break;
							}
						}

						++index;
					}

					--index;
				}
			} break;

			case '<': {
				if (next_char(script_text, index) == '>') {
					this_token.token = InkToken::Diamond;
					++index;
				} else if (next_char(script_text, index) == '-') {
					this_token.token = InkToken::BackArrow;
					++index;
				} else {
					current_text += '<';
					end_text = false;
				}
			} break;

			case '!': {
				if (last_token(result).token == InkToken::LeftBrace) {
				this_token.token = InkToken::Bang;
				} else {
					current_text += '!';
					end_text = false;
				}
			} break;

			default: {
				if (auto token_char = TokenChars.find(chr); token_char != TokenChars.end()) {
					this_token.token = token_char->second;
				} else if (any_tokens_this_line || !current_text.empty() || chr > 32) {
					current_text += chr;
					end_text = false;
				}
			} break;
		}

		if (strip_string_edges(current_text, true, true, true) == "VAR") {
			this_token.token = InkToken::KeywordVar;
			current_text.clear();
		}

		if (end_text) {
			try_add_text_token(result, current_text);
			current_text.clear();
		}

		if (this_token.token != InkToken::INVALID) {
			result.push_back(this_token);
			any_tokens_this_line = this_token.token != InkToken::NewLine;
		}

		++index;
	}

	try_add_text_token(result, current_text);

	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

InkStory InkCompiler::compile_script(const std::string& script) {
	InkStoryData* story_data = compile(script);
	return InkStory(story_data);
}

InkStory InkCompiler::compile_file(const std::string& file_path)
{
	std::ifstream infile{file_path};
	std::stringstream buffer;
	buffer << infile.rdbuf();
	std::string file_text = buffer.str();
	infile.close();

	InkStoryData* story_data = compile(file_text);
	return InkStory(story_data);
}

void InkCompiler::save_data_to_file(InkStoryData* story_data, const std::string& out_file_path) {
	std::ofstream outfile{out_file_path, std::ios::binary};
	std::vector<std::uint8_t> bytes = story_data->get_serialized_bytes();
	for (std::uint8_t byte : bytes) {
		outfile << byte;
	}

	outfile.close();
}

void InkCompiler::compile_script_to_file(const std::string& script, const std::string& out_file_path) {
	InkStoryData* story_data = compile(script);
	save_data_to_file(story_data, out_file_path);
	delete story_data;
}

void InkCompiler::compile_file_to_file(const std::string& in_file_path, const std::string& out_file_path) {
	std::ifstream infile{in_file_path};
	std::stringstream buffer;
	buffer << infile.rdbuf();
	std::string file_text = buffer.str();
	infile.close();

	InkStoryData* story_data = compile(file_text);
	save_data_to_file(story_data, out_file_path);
	delete story_data;
}

void InkCompiler::init_compiler() {
	token_index = 0;
	last_token_object = nullptr;

	brace_level = 0;
	in_choice_line = false;
	at_line_start = true;
	past_choice_initial_braces = false;

	choice_level = 0;
	choice_stack.clear();

	current_sequence_index = 0;
}

InkStoryData *InkCompiler::compile(const std::string &script)
{
	init_compiler();

	InkLexer lexer;
	std::vector<InkLexer::Token> token_stream = lexer.lex_script(script);
	token_stream = remove_comments(token_stream);

	std::vector<Knot> result_knots = {{"_S", {}, {}}};

	token_index = 0;
	while (token_index < token_stream.size()) {
		const InkLexer::Token& this_token = token_stream[token_index];
		if (InkObject* this_token_object = compile_token(token_stream, this_token, result_knots)) {
			bool add_this_object = true;
			if (!this_token_object->has_any_contents(false)) {
				add_this_object = false;
			} else if (this_token_object->get_id() == ObjectId::LineBreak && result_knots.back().objects.empty()) {
				add_this_object = false;
			}

			if (add_this_object) {
				if (this_token_object->get_id() == ObjectId::Text && last_token_object && last_token_object->get_id() == ObjectId::Text) {
					static_cast<InkObjectText*>(result_knots.back().objects.back())->append_text(static_cast<InkObjectText*>(this_token_object)->get_text_contents());
					delete this_token_object;
					this_token_object = nullptr;
				} else if (!(this_token_object->get_id() == ObjectId::LineBreak && (result_knots.back().objects.empty() || (last_token_object && last_token_object->get_id() == ObjectId::LineBreak)))) {
					result_knots.back().objects.push_back(this_token_object);
				} else {
					delete this_token_object;
					this_token_object = nullptr;
				}
			} else {
				delete this_token_object;
				this_token_object = nullptr;
			}

			last_token_object = this_token_object;
		} else {
			last_token_object = nullptr;
		}
		
		++token_index;
	}

	/*for (Knot& knot : result_knots) {
		std::cout << "=== " << knot.name << std::endl;
		for (InkObject* object : knot.objects) {
			std::cout << object->to_string() << std::endl;
			delete object;
		}
	}*/

	return new InkStoryData(result_knots);
}

InkObject* InkCompiler::compile_token(const std::vector<InkLexer::Token>& all_tokens, const InkLexer::Token& token, std::vector<Knot>& story_knots)
{
	bool end_line = false;
	InkObject* result_object = nullptr;

	switch (token.token) {
		case InkToken::NewLine: {
			result_object = new InkObjectLineBreak();
			end_line = true;
		} break;

		case InkToken::Slash: {
			result_object = new InkObjectText("/");
		} break;

		case InkToken::Hash: {
			std::string tag_contents;

			++token_index;
			while (true) {
				if (all_tokens[token_index].token == InkToken::Hash) {
					--token_index;
					break;
				} else if (all_tokens[token_index].token == InkToken::NewLine) {
					break;
				}

				tag_contents += all_tokens[token_index].text_contents;
				++token_index;
			}

			result_object = new InkObjectTag(strip_string_edges(tag_contents, true, true, true));
		} break; 

		case InkToken::Equal: {
			if (at_line_start) {
				if (next_token_is_sequence(all_tokens, token_index, {InkToken::Equal, InkToken::Equal})) {
					if (next_token_is(all_tokens, token_index + 2, InkToken::Text)) {
						std::string new_knot_name = strip_string_edges(all_tokens[token_index + 3].text_contents, true, true, true);
						story_knots.push_back({new_knot_name, {}});

						while (all_tokens[token_index].token != InkToken::NewLine) {
							++token_index;
						}

						end_line = true;
					} else {
						throw std::runtime_error("Expected knot name");
					}
				} else if (next_token_is(all_tokens, token_index, InkToken::Text)) {
					std::string new_stitch_name = strip_string_edges(all_tokens[token_index + 1].text_contents, true, true, true);
					
					std::vector<Stitch>& stitches = story_knots.back().stitches;
					stitches.push_back({new_stitch_name, static_cast<std::uint16_t>(story_knots.back().objects.size())});
					//std::sort(stitches.begin(), stitches.end(), [](const Stitch& a, const Stitch& b) { return a.index < b.index; });

					while (all_tokens[token_index].token != InkToken::NewLine) {
						++token_index;
					}

					end_line = true;
				} else {
					throw std::runtime_error("Expected stitch name");
				}
			} else {
				result_object = new InkObjectText("=");
			}
		} break;

		case InkToken::Asterisk:
		case InkToken::Plus: {
			if (at_line_start) {
				++choice_level;
				std::vector<InkChoiceEntry> choice_options;
				bool has_gather = false;
				bool current_choice_sticky = token.token == InkToken::Plus;

				while (token_index < all_tokens.size()) {
					if (all_tokens[token_index].token == InkToken::NewLine) {
						if (next_token_is(all_tokens, token_index, InkToken::Equal)) {
							end_line = true;
							break;
						} else if (next_token_is(all_tokens, token_index, InkToken::Dash)) { // TODO: nested choices
							has_gather = true;
							end_line = true;
							break;
						} else if ((next_token_is(all_tokens, token_index, InkToken::Asterisk) || next_token_is(all_tokens, token_index, InkToken::Plus)) && next_token(all_tokens, token_index).count < choice_level) { // TODO: nested choices
							end_line = true;
							break;
						}
					}

					bool in_result = false;
					in_choice_line = true;
					past_choice_initial_braces = false;
					++token_index;

					choice_stack.push_back({.sticky = current_choice_sticky});
					InkChoiceEntry& choice_entry = choice_stack.back();

					while (token_index < all_tokens.size()) {
						const InkLexer::Token& in_choice_token = all_tokens[token_index];
						if (in_choice_token.token == InkToken::NewLine) {
							if (next_token_is(all_tokens, token_index, InkToken::Dash) || next_token_is(all_tokens, token_index, InkToken::Equal)) {
								break;
							} else if (next_token_is(all_tokens, token_index, InkToken::Asterisk) || next_token_is(all_tokens, token_index, InkToken::Plus)) {
								++token_index;
								current_choice_sticky = all_tokens[token_index].token == InkToken::Plus;
								break;
							}
						}

						if (InkObject* in_choice_object = compile_token(all_tokens, in_choice_token, story_knots)) {
							if (!in_result && (in_choice_object->get_id() == ObjectId::LineBreak || in_choice_object->get_id() == ObjectId::Divert)) {
								in_result = true;
								in_choice_line = false;
								if (in_choice_object->get_id() == ObjectId::Divert) {
									if (in_choice_object->has_any_contents(false)) {
										--token_index;
										choice_entry.immediately_continue_to_result = !choice_entry.text.empty();
									} else {
										const std::vector<InkObject*>& text_contents = choice_entry.text;
										bool no_text = text_contents.empty() || (text_contents.size() == 1 && !text_contents[0]->has_any_contents(true));
										choice_entry.fallback = no_text;
										++token_index;
										choice_entry.immediately_continue_to_result = true;
									}
								}

								delete in_choice_object;
								continue;
							} else if (in_result && in_choice_object->get_id() == ObjectId::LineBreak && choice_entry.result.objects.empty()) {
								delete in_choice_object;
								++token_index;
								continue;
							}

							std::vector<InkObject*>& target_array = !in_result ? choice_entry.text : choice_entry.result.objects;
							if (in_choice_object->has_any_contents(false)) {
								target_array.push_back(in_choice_object);
							}
						}
						
						++token_index;

						if (in_choice_token.token != InkToken::LeftBrace && (in_choice_token.token != InkToken::Text || !strip_string_edges(in_choice_token.text_contents, true, true, true).empty())) {
							past_choice_initial_braces = true;
						} 
					}

					choice_options.push_back(choice_entry);
					choice_stack.pop_back();
				}

				result_object = new InkObjectChoice(choice_options, has_gather);
				--choice_level;
			} else {
				result_object = new InkObjectText(token.token == InkToken::Plus ? "+" : "*");
			}
		} break;

		case InkToken::LeftBracket:
		case InkToken::RightBracket: {
			bool right = token.token == InkToken::RightBracket;
			if (in_choice_line) {
				result_object = new InkObjectChoiceTextMix(right);
			} else {
				result_object = new InkObjectText(right ? "]" : "[");
			}
		} break;

		case InkToken::LeftBrace: {
			++brace_level;

			InkSequenceType sequence_type = InkSequenceType::Sequence;
			switch (next_token(all_tokens, token_index).token) {
				case InkToken::Ampersand: {
					sequence_type = InkSequenceType::Cycle;
					++token_index;
				} break;
				case InkToken::Bang: {
					sequence_type = InkSequenceType::OnceOnly;
					++token_index;
				} break;
				case InkToken::Tilde: {
					sequence_type = InkSequenceType::Shuffle;
					++token_index;
				} break;
				default: break;
			}

			std::vector<std::vector<InkObject*>> items = {{}};
			std::vector<InkObject*> items_if;
			std::vector<InkObject*> items_else;
			std::vector<std::string> text_items;

			bool is_conditional = false;
			bool in_else = false;
			bool found_pipe = false;

			++token_index;
			while (all_tokens[token_index].token != InkToken::RightBrace) {
				text_items.push_back(all_tokens[token_index].text_contents);
				if (in_choice_line && !past_choice_initial_braces) {
					items_if.push_back(compile_token(all_tokens, all_tokens[token_index], story_knots));
				} else {
					switch (all_tokens[token_index].token) {
						case InkToken::Colon: {
							is_conditional = true;
						} break;

						case InkToken::Pipe: {
							found_pipe = true;
							if (is_conditional) {
								in_else = true;
							} else {
								items.push_back({});
							}
						} break;

						default: {
							InkObject* compiled_object = compile_token(all_tokens, all_tokens[token_index], story_knots);
							if (is_conditional) {
								std::vector<InkObject*>& target_array = in_else ? items_else : items_if;
								target_array.push_back(compiled_object);
							} else if (!items.empty()) {
								items.back().push_back(compiled_object);
							} else {
								delete compiled_object;
							}
						} break;
					}
				}

				++token_index;
			}

			--brace_level;

			if (in_choice_line && !past_choice_initial_braces) {
				std::string condition;
				condition.reserve(items_if.size() * 10);
				for (InkObject* object : items_if) {
					condition += object->to_string();
					delete object;
				}

				choice_stack.back().conditions.push_back(condition);
			} else {
				if (is_conditional) {
					std::string condition;
					condition.reserve(50);
					for (InkObject* object : items[0]) {
						condition += object->to_string();
						delete object;
					}

					result_object = new InkObjectConditional(strip_string_edges(condition, true, true, true), items_if, items_else);
				} else if (found_pipe || sequence_type != InkSequenceType::Sequence) {
					result_object = new InkObjectSequence(sequence_type, items);
				} else if (!text_items.empty()) {
					std::string all_text = join_string_vector(text_items, std::string());
					result_object = new InkObjectInterpolation(all_text);
				}
			}
		} break;

		case InkToken::Colon: {
			if (brace_level == 0) {
				result_object = new InkObjectText(":");
			}
		} break;

		case InkToken::Arrow: {
			if (next_token_is(all_tokens, token_index, InkToken::Text)) {
				const std::string& target = strip_string_edges(all_tokens[token_index + 1].text_contents, true, true, true);
				if (!in_parens) {
					result_object = new InkObjectDivert(target);
					++token_index;
				} else {
					result_object = new InkObjectText("->");
				}
			} else if (in_choice_line) {
				result_object = new InkObjectDivert();
			} else {
				throw std::runtime_error("Expected divert target");
			}
		} break;

		case InkToken::Diamond: {
			result_object = new InkObjectGlue();
		} break;

		case InkToken::Dash: {
			if (!at_line_start) {
				result_object = new InkObjectText("-");
			}
		} break;

		case InkToken::Tilde: {
			if (at_line_start) {
				++token_index;

				std::string expression;
				expression.reserve(50);
				while (all_tokens[token_index].token != InkToken::NewLine) {
					expression += all_tokens[token_index].text_contents;
					++token_index;
				}

				result_object = new InkObjectLogic(expression);
			} else {
				result_object = new InkObjectText("~");
			}
		} break;

		case InkToken::LeftParen: {
			in_parens = true;
			if (false) {

			} else {
				result_object = new InkObjectText("(");
			}
		} break;

		case InkToken::RightParen: {
			in_parens = false;
			if (false) {

			} else {
				result_object = new InkObjectText(")");
			}
		} break;

		case InkToken::KeywordVar: {
			if (at_line_start) {
				if (next_token_is_sequence(all_tokens, token_index, {InkToken::Text, InkToken::Equal})) {
					const std::string& identifier = strip_string_edges(all_tokens[token_index + 1].text_contents, true, true, true);
					
					token_index += 3;
					std::string expression;
					expression.reserve(50);
					while (all_tokens[token_index].token != InkToken::NewLine) {
						expression += all_tokens[token_index].text_contents;
						++token_index;
					}

					result_object = new InkObjectGlobalVariable(identifier, strip_string_edges(expression, true, true, true));
				} else {
					throw std::runtime_error("Malformed VAR statement");
				}
			} else {
				result_object = new InkObjectText("VAR");
			}
		} break;

		case InkToken::Text: {
			std::string text_stripped = strip_string_edges(token.text_contents, true, true, true);
			std::string text_notabs = strip_string_edges(token.text_contents);
			if (!text_notabs.empty() && (!text_stripped.empty() || !in_choice_line || !at_line_start)) {
				result_object = new InkObjectText(token.text_contents);
			}
		} break;

		default: break;
	}

	at_line_start = end_line;
	// result_object->end_text = end_text;

	return result_object;
}

std::vector<InkLexer::Token> InkCompiler::remove_comments(const std::vector<InkLexer::Token>& tokens) {
	std::vector<InkLexer::Token> result;

	std::size_t index = 0;
	while (index < tokens.size()) {
		if (tokens[index].token == InkToken::Slash) {
			if (next_token_is(tokens, index, InkToken::Slash)) {
				++index;
				do {
					++index;
				} while (tokens[index].token != InkToken::NewLine);
			} else if (next_token_is(tokens, index, InkToken::Asterisk)) {
				++index;
				do {
					++index;
				} while (tokens[index].token != InkToken::Asterisk || !next_token_is(tokens, index, InkToken::Slash));

				index += 2;
			}
		}

		result.push_back(tokens[index]);
		++index;
	}

	return result;
}

InkLexer::Token InkCompiler::next_token(const std::vector<InkLexer::Token>& tokens, std::size_t index) {
	if (index + 1 < tokens.size()) {
		return tokens[index + 1];
	} else {
		return InkLexer::Token();
	}
}

bool InkCompiler::next_token_is(const std::vector<InkLexer::Token>& tokens, std::size_t index, InkToken what) {
	return next_token(tokens, index).token == what;
}

bool InkCompiler::next_token_is_sequence(const std::vector<InkLexer::Token>& tokens, std::size_t index, std::vector<InkToken>&& what) {
	for (std::size_t i = 0; i < what.size(); ++i) {
		if (!next_token_is(tokens, index + i, what[i])) {
			return false;
		}
	}

	return true;
}
