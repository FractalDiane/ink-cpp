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
#include "objects/ink_object_list.h"
#include "objects/ink_object_logic.h"
#include "objects/ink_object_sequence.h"
#include "objects/ink_object_tag.h"
#include "objects/ink_object_text.h"

#include "expression_parser/expression_parser.h"

#include <unordered_map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <list>

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
		{',', InkToken::Comma},
	};

	struct KeywordEntry {
		InkToken token;
		bool must_be_at_line_start;
	};

	static const std::unordered_map<std::string, KeywordEntry> TokenKeywords = {
		{"VAR", {InkToken::KeywordVar, true}},
		{"CONST", {InkToken::KeywordConst, true}},
		{"function", {InkToken::KeywordFunction, false}},
		{"INCLUDE", {InkToken::KeywordInclude, true}},
		{"LIST", {InkToken::KeywordList, true}},
		{"EXTERNAL", {InkToken::KeywordExternal, true}},
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

	void try_add_text_token(std::vector<InkLexer::Token>& result, const std::string& text, bool& currently_escaped) {
		InkLexer::Token text_token{InkToken::Text, text};
		text_token.escaped = currently_escaped;
		if (!text_token.text_contents.empty()) {
			result.push_back(text_token);
			currently_escaped = false;
		}
	}
}

std::vector<InkLexer::Token> InkLexer::lex_script(const std::string& script_text) {
	std::vector<Token> result;

	std::size_t index = 0;
	std::string current_text;
	current_text.reserve(50);
	bool current_text_escaped = false;
	bool any_tokens_this_line = false;
	bool at_line_start = true;

	while (index < script_text.length()) {
		Token this_token;
		bool end_text = true;
		char chr = script_text[index];

		this_token.text_contents = chr;
		switch (chr) {
			case '\n': {
				if (last_token(result).token != InkToken::NewLine || !current_text.empty()) {
					this_token.token = InkToken::NewLine;
					at_line_start = true;
				}
			} break;

			case '\\': {
				current_text += next_char(script_text, index);
				current_text_escaped = true;
				++index;
			} break;

			case '*':
			case '+': {
				if (at_line_start) {
					this_token.token = chr == '*' ? InkToken::Asterisk : InkToken::Plus;
					++index;
					std::size_t whitespace_skipped = 0;
					while (true) {
						char inner_chr = script_text[index];
						if (inner_chr > 32 || inner_chr == '\n') {
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
				} else {
					this_token.token = chr == '*' ? InkToken::Asterisk : InkToken::Plus;
				}
			} break;

			case '-': {
				if (next_char(script_text, index) == '>') {
					this_token.token = InkToken::Arrow;
					this_token.text_contents = "->";
					++index;
				} else if (at_line_start) {
					this_token.token = InkToken::Dash;
					++index;
					while (true) {
						char inner_chr = script_text[index];
						if (inner_chr > 32 || inner_chr == '\n') {
							if (inner_chr == chr) {
								++this_token.count;
							} else {
								break;
							}
						}

						++index;
					}

					// for weird gather + divert cases like - -> END
					if (next_char(script_text, index - 1) == '>') {
						--this_token.count;
						--index;
					}

					--index;
				} else {
					this_token.token = InkToken::Dash;
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
					at_line_start = false;
				}
			} break;
		}

		if (auto keyword_token = TokenKeywords.find(strip_string_edges(current_text, true, true, true)); keyword_token != TokenKeywords.end()) {
			if (!any_tokens_this_line || !keyword_token->second.must_be_at_line_start) {
				this_token.token = keyword_token->second.token;
				current_text.clear();
			}
		}

		if (end_text) {
			try_add_text_token(result, current_text, current_text_escaped);
			current_text.clear();
		}

		if (this_token.token != InkToken::INVALID) {
			result.push_back(this_token);
			any_tokens_this_line = this_token.token != InkToken::NewLine;
			at_line_start = this_token.token == InkToken::NewLine;;
		}

		++index;
	}

	try_add_text_token(result, current_text, current_text_escaped);

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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
	last_object = nullptr;

	brace_level = 0;
	in_choice_line = false;
	at_line_start = true;
	past_choice_initial_braces = false;

	choice_level = 0;
	choice_stack.clear();

	current_sequence_index = 0;
}

InkStoryData* InkCompiler::compile(const std::string& script)
{
	init_compiler();

	InkLexer lexer;
	std::vector<InkLexer::Token> token_stream = lexer.lex_script(script);
	token_stream = remove_comments(token_stream);

	std::vector<Knot> result_knots;// = {{"_S", {}, {}}};
	Knot start_knot;
	start_knot.name = "_S";
	start_knot.uuid = Uuid(current_uuid++);
	result_knots.push_back(start_knot);
	current_knot_index = 0;

	story_variable_info = ExpressionParserV2::StoryVariableInfo();
	story_variable_info.builtin_functions = {
		{"RANDOM", {nullptr, (std::uint8_t)2}},
		{"SEED_RANDOM", {nullptr, (std::uint8_t)1}},
		{"CHOICE_COUNT", {nullptr, (std::uint8_t)0}},
		{"TURNS", {nullptr, (std::uint8_t)0}},
		{"TURNS_SINCE", {nullptr, (std::uint8_t)1}},

		{"LIST_VALUE", {nullptr, (std::uint8_t)1}},
		{"LIST_COUNT", {nullptr, (std::uint8_t)1}},
		{"LIST_MIN", {nullptr, (std::uint8_t)1}},
		{"LIST_MAX", {nullptr, (std::uint8_t)1}},
		{"LIST_RANDOM", {nullptr, (std::uint8_t)1}},
		{"LIST_ALL", {nullptr, (std::uint8_t)1}},
		{"LIST_INVERT", {nullptr, (std::uint8_t)1}},
		{"LIST_RANGE", {nullptr, (std::uint8_t)3}},
	};

	token_index = 0;
	while (token_index < token_stream.size()) {
		const InkLexer::Token& this_token = token_stream[token_index];
		if (InkObject* this_token_object = compile_token(token_stream, this_token, result_knots)) {
			Knot& current_knot = result_knots[current_knot_index];
			bool add_this_object = true;
			if (!this_token_object->has_any_contents(false)) {
				add_this_object = false;
			} else if (this_token_object->get_id() == ObjectId::LineBreak && current_knot.objects.empty()) {
				add_this_object = false;
			}

			bool appended_text = false;
			if (add_this_object) {
				if (this_token_object->get_id() == ObjectId::Text && last_token_object && last_token_object->get_id() == ObjectId::Text) {
					auto* last_text = static_cast<InkObjectText*>(current_knot.objects.back());
					last_text->append_text(static_cast<InkObjectText*>(this_token_object)->get_text_contents());
					if (this_token_object == last_object) {
						last_object = last_text;
					}

					delete this_token_object;
					this_token_object = nullptr;
					appended_text = true;
				} else {
					current_knot.objects.push_back(this_token_object);
					if (!current_knot.has_content && this_token_object->contributes_content_to_knot()) {
						current_knot.has_content = true;
					}
				}
			} else {
				delete this_token_object;
				this_token_object = nullptr;
			}

			if (!appended_text) {
				last_token_object = this_token_object;
			}
		} else {
			last_token_object = nullptr;
		}
		
		++token_index;
	}

	InkStoryData* result = new InkStoryData(result_knots, std::move(story_variable_info));
	return result;
}

InkObject* InkCompiler::compile_token(std::vector<InkLexer::Token>& all_tokens, const InkLexer::Token& token, std::vector<Knot>& story_knots)
{
	bool end_line = false;
	InkObject* result_object = nullptr;

	switch (token.token) {
		case InkToken::NewLine: {
			if (last_object) {
				switch (last_object->get_id()) {
					case ObjectId::Logic:
						for (const ExpressionParserV2::Token& logic_token : static_cast<InkObjectLogic*>(last_object)->contents_shunted_tokens.tokens) {
							if (logic_token.type == ExpressionParserV2::TokenType::Function) {
								for (const Knot& knot : story_knots) {
									if (knot.is_function && knot.has_content && knot.name == static_cast<std::string>(logic_token.value)) {
										goto add_newline;
									}
								}
							}
						}

						break;
					case ObjectId::ChoiceTextMix:
					case ObjectId::Text:
					case ObjectId::Interpolation:
					case ObjectId::Conditional:
					case ObjectId::Sequence:
					case ObjectId::Glue:
					case ObjectId::Divert:
					add_newline:
						result_object = new InkObjectLineBreak();
						break;
					default:
						break;
				}
			}

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

				tag_contents += all_tokens[token_index].get_text_contents();
				++token_index;
			}

			result_object = new InkObjectTag(strip_string_edges(tag_contents, true, true, true));
		} break; 

		case InkToken::Equal: {
			if (at_line_start) {
				if (next_token_is(all_tokens, token_index, InkToken::Equal)) {
					++token_index;
					if (next_token_is(all_tokens, token_index, InkToken::Equal)) {
						++token_index;
					}

					if (next_token_is(all_tokens, token_index, InkToken::Text) || next_token_is(all_tokens, token_index, InkToken::KeywordFunction)) {
						Knot new_knot;
						if (all_tokens[token_index + 1].token == InkToken::KeywordFunction) {
							new_knot.is_function = true;
							++token_index;
						}

						std::string new_knot_name = strip_string_edges(all_tokens[token_index + 1].text_contents, true, true, true);

						bool is_new_knot = true;
						std::size_t existing_index = 0;
						for (std::size_t i = 0; i < story_knots.size(); ++i)
						for (const Knot& knot : story_knots) {
							if (knot.name == new_knot_name) {
								is_new_knot = false;
								break;
							}

							++existing_index;
						}

						new_knot.name = new_knot_name;
						new_knot.uuid = Uuid(current_uuid++);
						new_knot.type = WeaveContentType::Knot;

						if (next_token_is(all_tokens, token_index + 1, InkToken::LeftParen)) {
							token_index += 3;

							std::string all_params;
							all_params.reserve(50);
							bool found_arrow = false;
							while (all_tokens[token_index].token != InkToken::RightParen) {
								if (all_tokens[token_index].token != InkToken::Arrow) {
									if (found_arrow) {
										all_params += strip_string_edges(all_tokens[token_index].text_contents, true, false, true);
										found_arrow = false;
									} else {
										all_params += all_tokens[token_index].text_contents;
									}
								} else {
									found_arrow = true;
								}
								
								++token_index;
							}

							--token_index;

							std::vector<std::string> split = split_string(all_params, ',', true);
							std::vector<InkWeaveContent::Parameter> params;
							for (const std::string& param : split) {
								// TODO: handle multiple spaces
								if (param.starts_with("ref ")) {
									params.push_back({param.substr(4), true});
								} else {
									params.push_back({param, false});
								}
							}

							new_knot.parameters = params;
						}

						// line break objects at the end of a knot are redundant and only cause problems
						//if (!story_knots.back().objects.empty() && story_knots.back().objects.back()->get_id() == ObjectId::LineBreak) {
						//	story_knots.back().objects.pop_back();
						//}

						// TODO: don't force it to parse all the parameters if the knot already exists
						if (is_new_knot) {
							story_knots.push_back(new_knot);
							++current_knot_index;
						} else {
							current_knot_index = existing_index;
						}

						while (all_tokens[token_index].token != InkToken::NewLine) {
							++token_index;
						}

						end_line = true;
					} else {
						throw std::runtime_error("Expected knot name");
					}
				} else if (next_token_is(all_tokens, token_index, InkToken::Text)) {
					std::string new_stitch_name = strip_string_edges(all_tokens[token_index + 1].text_contents, true, true, true);
					
					std::vector<Stitch>& stitches = story_knots[current_knot_index].stitches;
					Stitch new_stitch;
					new_stitch.name = new_stitch_name;
					new_stitch.uuid = Uuid(current_uuid++);
					new_stitch.type = WeaveContentType::Stitch;
					new_stitch.index = static_cast<std::uint16_t>(story_knots[current_knot_index].objects.size());

					// TODO: dry it
					if (next_token_is(all_tokens, token_index + 1, InkToken::LeftParen)) {
						token_index += 3;

						std::string all_params;
						all_params.reserve(50);
						while (all_tokens[token_index].token != InkToken::RightParen) {
							all_params += all_tokens[token_index].text_contents;
							++token_index;
						}

						--token_index;

						std::vector<std::string> split = split_string(all_params, ',', true);
						std::vector<InkWeaveContent::Parameter> params;
						for (const std::string& param : split) {
							// TODO: handle multiple spaces
							if (param.starts_with("ref ")) {
								params.push_back({param.substr(4), true});
							} else {
								params.push_back({param, false});
							}
						}
							
						new_stitch.parameters = params;
					}

					stitches.push_back(new_stitch);

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
			if (at_line_start && token.count > choice_level) {
				++choice_level;
				std::list<InkChoiceEntry> choice_options;
				bool current_choice_sticky = token.token == InkToken::Plus;

				while (token_index < all_tokens.size()) {
					if (all_tokens[token_index].token == InkToken::NewLine) {
						if (next_token_is(all_tokens, token_index, InkToken::Equal)) {
							end_line = true;
							break;
						} else if (next_token_is(all_tokens, token_index, InkToken::RightBrace)) {
							end_line = true;
							break;
						} else if (next_token_is(all_tokens, token_index, InkToken::Dash)) {
							end_line = true;
							break;
						} else if ((next_token_is(all_tokens, token_index, InkToken::Asterisk) || next_token_is(all_tokens, token_index, InkToken::Plus)) && next_token(all_tokens, token_index).count <= choice_level) {
							end_line = true;
							break;
						}
					}

					bool in_result = false;
					in_choice_line = true;
					past_choice_initial_braces = false;
					++token_index;

					choice_stack.push_back(InkChoiceEntry(current_choice_sticky));
					anonymous_knot_stack.push_back(&choice_stack.back().result);

					while (all_tokens[token_index].token == InkToken::Text && !all_tokens[token_index].escaped && strip_string_edges(all_tokens[token_index].get_text_contents(), true, true, true).empty()) {
						++token_index;
					}

					if (all_tokens[token_index].token == InkToken::LeftParen && next_token_is_sequence(all_tokens, token_index, {InkToken::Text, InkToken::RightParen})) {
						GatherPoint label;
						label.name = all_tokens[token_index + 1].text_contents;
						label.uuid = Uuid(current_uuid++);
						label.type = WeaveContentType::GatherPoint;
						//label.index = static_cast<std::uint16_t>(story_knots[current_knot_index].objects.size());
						label.in_choice = true;
						label.choice_index = static_cast<std::uint16_t>(choice_options.size());
						choice_stack.back().label = label;

						Knot& gather_point_knot = anonymous_knot_stack.size() > 1 ? *anonymous_knot_stack[anonymous_knot_stack.size() - 2] : story_knots[current_knot_index];
						std::vector<GatherPoint>& gather_points = 
						!gather_point_knot.stitches.empty() && gather_point_knot.objects.size() >= gather_point_knot.stitches[0].index
						? gather_point_knot.stitches.back().gather_points
						: gather_point_knot.gather_points;

						label.index = static_cast<std::uint16_t>(gather_point_knot.objects.size());
						gather_points.push_back(label);
						
						token_index += 3;
					}

					while (token_index < all_tokens.size()) {
						const InkLexer::Token& in_choice_token = all_tokens[token_index];
						if (in_choice_token.token == InkToken::NewLine) {
							InkLexer::Token next = next_token(all_tokens, token_index);
							if (next.token == InkToken::Equal || next.token == InkToken::RightBrace) {
								break;
							} else if (next.token == InkToken::Dash && next.count <= choice_level) {
								break;
							} else if (next.token == InkToken::Asterisk || next.token == InkToken::Plus) {
								if (next.count == choice_level) {
									++token_index;
									current_choice_sticky = all_tokens[token_index].token == InkToken::Plus;
									break;
								} else if (next.count < choice_level) {
									break;
								}
							}
						}

						if (InkObject* in_choice_object = compile_token(all_tokens, in_choice_token, story_knots)) {
							InkChoiceEntry& choice_entry = choice_stack.back();
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

								if (last_object == in_choice_object) {
									last_object = nullptr;
								}

								delete in_choice_object;
								continue;
							} else if (in_result && in_choice_object->get_id() == ObjectId::LineBreak && choice_entry.result.objects.empty()) {
								if (last_object == in_choice_object) {
									last_object = nullptr;
								}
								
								delete in_choice_object;
								++token_index;
								continue;
							}

							std::vector<InkObject*>& target_array = !in_result ? choice_entry.text : choice_entry.result.objects;
							if (in_choice_object->has_any_contents(false)) {
								target_array.push_back(in_choice_object);
							}

							if (in_choice_object->get_id() == ObjectId::Choice) {
								--token_index;
							}
						}
						
						++token_index;

						if (!past_choice_initial_braces && in_choice_token.token != InkToken::LeftBrace && (in_choice_token.escaped || in_choice_token.token != InkToken::Text || !strip_string_edges(in_choice_token.text_contents, true, true, true).empty())) {
							past_choice_initial_braces = true;
						}
					}

					in_choice_line = false;
					choice_options.push_back(choice_stack.back());
					choice_stack.pop_back();
					anonymous_knot_stack.pop_back();
				}

				in_choice_line = false;

				std::vector<InkChoiceEntry> choice_options_vec{
					std::make_move_iterator(std::begin(choice_options)),
					std::make_move_iterator(std::end(choice_options)),
				};

				result_object = new InkObjectChoice(choice_options_vec);
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

			bool is_explicit_alternative = false;

			if (next_token_is_sequence(all_tokens, token_index, {InkToken::Text, InkToken::Colon})) {
				std::string text = strip_string_edges(all_tokens[token_index + 1].text_contents, true, true, true);
				if (text == "stopping") {
					token_index += 2;
					is_explicit_alternative = true;
				} else if (text == "shuffle") {
					sequence_type = InkSequenceType::Shuffle;
					token_index += 2;
					is_explicit_alternative = true;
				} else if (text == "cycle") {
					sequence_type = InkSequenceType::Cycle;
					token_index += 2;
					is_explicit_alternative = true;
				} else if (text == "once") {
					sequence_type = InkSequenceType::OnceOnly;
					token_index += 2;
					is_explicit_alternative = true;
				} else if (text == "shuffle once") {
					sequence_type = InkSequenceType::ShuffleOnce;
					token_index += 2;
					is_explicit_alternative = true;
				} else if (text == "shuffle stopping") {
					sequence_type = InkSequenceType::ShuffleStop;
					token_index += 2;
					is_explicit_alternative = true;
				}
			}

			std::vector<Knot> items = {Knot()};
			std::vector<std::pair<ExpressionParserV2::ShuntedExpression, Knot>> items_conditions = {{{}, Knot()}};
			Knot items_else;
			std::vector<std::string> text_items;
			ExpressionParserV2::ShuntedExpression switch_expression;
			switch_expression.uuid = current_uuid++;

			bool is_conditional = false;
			bool will_be_conditional = false;
			bool is_switch = false;
			bool in_else = false;
			bool found_pipe = false;
			bool found_colon = false;
			bool found_dash = false;

			++token_index;

			for (std::size_t i = token_index; i < all_tokens.size() && all_tokens[i].token != InkToken::RightBrace; ++i) {
				if (all_tokens[i].token == InkToken::Colon) {
					will_be_conditional = true;
					break;
				}
			}

			while (token_index < all_tokens.size() && all_tokens[token_index].token != InkToken::RightBrace) {
				if (!all_tokens[token_index].get_text_contents().empty()) {
					text_items.push_back(all_tokens[token_index].get_text_contents());
				}
				
				if (in_choice_line && !past_choice_initial_braces) {
					items_conditions.back().second.objects.push_back(compile_token(all_tokens, all_tokens[token_index], story_knots));
				} else {
					switch (all_tokens[token_index].token) {
						case InkToken::Colon: {
							found_colon = true;
							is_conditional = true;
							if (auto& current_condition = items_conditions.back().first; current_condition.tokens.empty()) {
								std::string condition_string;
								condition_string.reserve(50);
								for (InkObject* object : items[0].objects) {
									condition_string += object->to_string();
									if (object == last_object) {
										last_object = nullptr;
									}

									delete object;
								}

								try {
									current_condition = ExpressionParserV2::tokenize_and_shunt_expression(condition_string, story_variable_info);
								} catch (...) {
									throw std::runtime_error("Malformed condition in conditional");
								}

								items.clear();
							}
						} break;

						case InkToken::Pipe: {
							if (will_be_conditional) {
								if (found_colon) {
									in_else = true;
									break;
								}
							} else {
								found_pipe = true;
								items.push_back({});
								break;
							}
						}

						case InkToken::Dash: {
							if (at_line_start) {
								bool is_condition_entry = !is_conditional;
								if (!is_condition_entry) {
									std::size_t index = token_index;
									while (index < all_tokens.size()) {
										InkToken this_token = all_tokens[index].token;
										if (this_token == InkToken::Colon) {
											is_condition_entry = true;
											break;
										}

										if (this_token == InkToken::NewLine || this_token == InkToken::LeftBrace) {
											break;
										}

										++index;
									}
								}
								
								if (!is_condition_entry) {
									break;
								}

								if (is_explicit_alternative) {
									++token_index;
									if (found_dash) {
										items.push_back({});
									}

									found_dash = true;
								} else {
									if (InkLexer::Token next = next_token(all_tokens, token_index); next.token == InkToken::Text && next.text_contents == "else") {
										in_else = true;
										token_index += 2;

										while (next_token_is(all_tokens, token_index, InkToken::NewLine) || (strip_string_edges(next_token(all_tokens, token_index).get_text_contents(), true, true, true).empty())) {
											++token_index;
										}

										found_dash = true;
										break;
									} else {
										is_conditional = true;
										if (found_colon && !found_dash) {
											is_switch = true;
											switch_expression = items_conditions.back().first;
											for (InkObject* object : items_conditions.back().second.objects) {
												delete object;
											}

											items_conditions.back().first.tokens.clear();
											items_conditions.back().second.objects.clear();
										}
										
										std::string this_condition;
										this_condition.reserve(50);
										++token_index;
										while (token_index < all_tokens.size() && all_tokens[token_index].token != InkToken::Colon) {
											this_condition += all_tokens[token_index].get_text_contents();
											++token_index;
										}

										while (next_token_is(all_tokens, token_index, InkToken::NewLine) || (strip_string_edges(next_token(all_tokens, token_index).get_text_contents(), true, true, true).empty())) {
											++token_index;
										}

										try {
											ExpressionParserV2::ShuntedExpression shunted = ExpressionParserV2::tokenize_and_shunt_expression(this_condition, story_variable_info);
											shunted.uuid = current_uuid++;
											if (items_conditions.back().first.tokens.empty()) {
												items_conditions.back().first = shunted;
											} else {
												items_conditions.push_back({shunted, {}});
											}
										} catch (...) {
											throw std::runtime_error("Malformed condition in conditional");
										}

										found_dash = true;
										break;
									}
								}
							}
						}

						default: {
							if (InkObject* compiled_object = compile_token(all_tokens, all_tokens[token_index], story_knots)) {
								// HACK: get rid of whitespace in switch results if they're text
								if (is_switch && compiled_object->get_id() == ObjectId::Text) {
									const Knot& target_array = in_else ? items_else : items_conditions.back().second;
									if (target_array.objects.empty()) {
										InkObjectText* object_text = static_cast<InkObjectText*>(compiled_object);
										object_text->set_text_contents(strip_string_edges(object_text->get_text_contents(), true, false, true));
									}
								}

								if (compiled_object->has_any_contents(false)) {
									if (is_conditional) {
										Knot& target_array = in_else ? items_else : items_conditions.back().second;
										target_array.objects.push_back(compiled_object);
									} else if (!items.empty()) {
										items.back().objects.push_back(compiled_object);
									} else {
										delete compiled_object;
									}
								} else {
									delete compiled_object;
								}
							}
						} break;
					}
				}

				++token_index;
			}

			--brace_level;

			bool delete_items = true;
			if (in_choice_line && !past_choice_initial_braces) {
				std::string condition;
				condition.reserve(items_conditions.back().second.objects.size() * 10);
				for (InkObject* object : items_conditions.back().second.objects) {
					condition += object->to_string();
					delete object;
				}

				try {
					ExpressionParserV2::ShuntedExpression condition_shunted = ExpressionParserV2::tokenize_and_shunt_expression(condition, story_variable_info);
					condition_shunted.uuid = current_uuid++;
					choice_stack.back().conditions.push_back(condition_shunted);
				} catch (...) {
					throw std::runtime_error("Malformed choice condition");
				}
			} else {
				if (is_conditional) {
					if (!is_switch) {
						result_object = new InkObjectConditional(items_conditions, items_else);
					} else {
						result_object = new InkObjectConditional(switch_expression, items_conditions, items_else);
					}
				} else if (found_pipe || is_explicit_alternative || sequence_type != InkSequenceType::Sequence) {
					result_object = new InkObjectSequence(sequence_type, is_explicit_alternative, items);
					delete_items = false;
				} else if (!text_items.empty()) {
					std::string all_text = join_string_vector(text_items, std::string());
					try {
						ExpressionParserV2::ShuntedExpression shunted = ExpressionParserV2::tokenize_and_shunt_expression(all_text, story_variable_info);
						shunted.uuid = current_uuid++;
						result_object = new InkObjectInterpolation(shunted);
					} catch (...) {
						throw std::runtime_error("Malformed interpolation");
					}
				}
			}

			if (delete_items) {
				for (auto& knot : items) {
					for (InkObject* object : knot.objects) {
						delete object;
					}
				}
			}
		} break;

		case InkToken::Colon: {
			if (brace_level == 0) {
				result_object = new InkObjectText(":");
			}
		} break;

		case InkToken::Arrow:
		case InkToken::BackArrow: {
			if (next_token_is(all_tokens, token_index, InkToken::Text) && !strip_string_edges(all_tokens[token_index + 1].text_contents, true, true, true).empty()) {
				if (!in_parens) {
					std::string target = strip_string_edges(all_tokens[token_index + 1].text_contents, true, true, true);
					ExpressionParserV2::ShuntedExpression target_tokens;
					target_tokens.uuid = current_uuid++;
					try {
						target_tokens = ExpressionParserV2::tokenize_and_shunt_expression(target, story_variable_info);
					} catch (...) {
						throw std::runtime_error("Illegal value in divert target");
					}
		
					std::vector<ExpressionParserV2::ShuntedExpression> arguments;
					if (next_token_is(all_tokens, token_index + 1, InkToken::LeftParen)) {
						token_index += 3;

						std::string all_args;
						all_args.reserve(50);

						std::size_t extra_paren_count = 0;
						while (all_tokens[token_index].token != InkToken::RightParen || extra_paren_count > 0) {
							all_args += all_tokens[token_index].text_contents;
							if (all_tokens[token_index].token == InkToken::LeftParen) {
								++extra_paren_count;
							} else if (all_tokens[token_index].token == InkToken::RightParen) {
								--extra_paren_count;
							}

							++token_index;
						}

						--token_index;

						std::vector<std::string> split = split_string(all_args, ',', true, true);
						for (const std::string& arg : split) {
							try {
								ExpressionParserV2::ShuntedExpression tokenized = ExpressionParserV2::tokenize_and_shunt_expression(arg, story_variable_info);
								tokenized.uuid = current_uuid++;
								arguments.push_back(tokenized);
							} catch (...) {
								throw std::runtime_error("Malformed knot argument");
							}
						}
					}

					if (token.token == InkToken::Arrow) {
						bool to_tunnel = false;

						// HACK: do this a way better way
						std::size_t index = token_index + 2;
						while (index < all_tokens.size() && all_tokens[index].token == InkToken::Text
							&& strip_string_edges(all_tokens[index].text_contents, true, true, true).empty()) {
							++index;
						}

						--index;

						if (next_token_is(all_tokens, index, InkToken::Arrow)) {
							to_tunnel = true;
							just_added_divert_to_tunnel = true;
						}
						
						result_object = new InkObjectDivert(target_tokens, arguments, to_tunnel ? DivertType::ToTunnel : DivertType::ToKnot);
						++token_index;
					} else {
						result_object = new InkObjectDivert(target_tokens, arguments, DivertType::Thread);
						++token_index;
					}
				} else {
					result_object = new InkObjectText("->");
				}
			} else if (next_token_is(all_tokens, token_index, InkToken::Arrow)) {
				ExpressionParserV2::ShuntedExpression target_tokens;
				target_tokens.uuid = current_uuid++;
				if (next_token_is(all_tokens, token_index + 1, InkToken::Text)) {
					std::string target = strip_string_edges(all_tokens[token_index + 2].text_contents, true, true, true);
					try {
						target_tokens = ExpressionParserV2::tokenize_and_shunt_expression(target, story_variable_info);
					} catch (...) {
						throw std::runtime_error("Illegal value in tunnel divert target");
					}
				}

				result_object = new InkObjectDivert(target_tokens, {}, DivertType::FromTunnel);
				++token_index;
			} else if (in_choice_line) {
				result_object = new InkObjectDivert();
			} else if (just_added_divert_to_tunnel) {
				just_added_divert_to_tunnel = false;
			} else {
				throw std::runtime_error("Expected divert target");
			}
		} break;

		case InkToken::Diamond: {
			result_object = new InkObjectGlue();
		} break;

		case InkToken::Dash: {
			if (at_line_start) {
				Knot& gather_point_knot = !anonymous_knot_stack.empty() ? *anonymous_knot_stack.back() : story_knots[current_knot_index];
				std::vector<GatherPoint>& gather_points = 
				!gather_point_knot.stitches.empty() && gather_point_knot.objects.size() >= gather_point_knot.stitches[0].index
				? gather_point_knot.stitches.back().gather_points
				: gather_point_knot.gather_points;
				
				GatherPoint new_gather_point;
				new_gather_point.uuid = Uuid(current_uuid++);
				new_gather_point.type = WeaveContentType::GatherPoint;
				//new_gather_point.index = static_cast<std::uint16_t>(story_knots[current_knot_index].objects.size());
				new_gather_point.index = static_cast<std::uint16_t>(gather_point_knot.objects.size());
				new_gather_point.level = token.count;

				if (next_token_is_sequence(all_tokens, token_index, {InkToken::LeftParen, InkToken::Text, InkToken::RightParen})) {
					new_gather_point.name = all_tokens[token_index + 2].text_contents;
					token_index += 3;
				}

				if (!story_knots[current_knot_index].objects.empty() && story_knots[current_knot_index].objects.back()->get_id() == ObjectId::Choice) {
					while (next_token_is(all_tokens, token_index, InkToken::NewLine)) {
						++token_index;
					}
				}

				gather_points.push_back(new_gather_point);
			} else {
				result_object = new InkObjectText("-");
			}
		} break;

		case InkToken::Tilde: {
			if (at_line_start) {
				++token_index;

				std::string expression;
				expression.reserve(50);
				while (all_tokens[token_index].token != InkToken::NewLine) {
					expression += all_tokens[token_index].get_text_contents();
					++token_index;
				}

				try {
					ExpressionParserV2::ShuntedExpression expression_shunted = ExpressionParserV2::tokenize_and_shunt_expression(expression, story_variable_info);
					expression_shunted.uuid = current_uuid++;
					result_object = new InkObjectLogic(expression_shunted);
				} catch (...) {
					throw std::runtime_error("Malformed logic statement");
				}

				--token_index;
				end_line = true;
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

		case InkToken::Bang: {
			if (false) {

			} else {
				result_object = new InkObjectText("!");
			}
		} break;

		case InkToken::Ampersand: {
			if (false) {

			} else {
				result_object = new InkObjectText("&");
			}
		}

		case InkToken::Pipe: {
			if (false) {

			} else {
				result_object = new InkObjectText("|");
			}
		} break;

		case InkToken::Comma: {
			if (false) {

			} else {
				result_object = new InkObjectText(",");
			}
		} break;
		
		case InkToken::KeywordVar:
		case InkToken::KeywordConst: {
			if (at_line_start) {
				if (next_token_is_sequence(all_tokens, token_index, {InkToken::Text, InkToken::Equal})) {
					const std::string& identifier = strip_string_edges(all_tokens[token_index + 1].text_contents, true, true, true);
					
					token_index += 3;
					std::string expression;
					expression.reserve(50);
					while (token_index < all_tokens.size() && all_tokens[token_index].token != InkToken::NewLine) {
						expression += all_tokens[token_index].get_text_contents();
						++token_index;
					}

					--token_index;

					try {
						ExpressionParserV2::ShuntedExpression expression_shunted = ExpressionParserV2::tokenize_and_shunt_expression(expression, story_variable_info);
						expression_shunted.uuid = current_uuid++;
						result_object = new InkObjectGlobalVariable(identifier, token.token == InkToken::KeywordConst, expression_shunted);
					} catch (...) {
						throw std::runtime_error("Malformed value of VAR/CONST statement");
					}
				} else {
					throw std::runtime_error("Malformed VAR/CONST statement");
				}
			} else {
				result_object = new InkObjectText(token.token == InkToken::KeywordConst ? "CONST" : "VAR");
			}
		} break;

		case InkToken::KeywordList: {
			if (at_line_start) {
				if (next_token_is_sequence(all_tokens, token_index, {InkToken::Text, InkToken::Equal})) {
					const std::string& identifier = strip_string_edges(all_tokens[token_index + 1].text_contents, true, true, true);
					
					token_index += 3;
					std::vector<InkListDefinition::Entry> entries;
					entries.reserve(16);

					std::string this_entry_name;
					std::optional<std::int64_t> this_entry_value = std::nullopt;
					std::int64_t last_entry_value = 0;
					bool include_this_entry = false;
					bool in_list_parens = false;

					auto add_entry = [&]() { 
						InkListDefinition::Entry new_entry;
						new_entry.label = this_entry_name;
						new_entry.value = this_entry_value.has_value() ? *this_entry_value : last_entry_value + 1;
						new_entry.is_included_by_default = include_this_entry;
						entries.push_back(new_entry);

						this_entry_name.clear();
						this_entry_value = std::nullopt;
						last_entry_value = new_entry.value;
						include_this_entry = false;
					};

					while (token_index < all_tokens.size() && all_tokens[token_index].token != InkToken::NewLine) {
						const InkLexer::Token& this_token = all_tokens[token_index];
						switch (this_token.token) {
							case InkToken::LeftParen: {
								if (!in_list_parens) {
									in_list_parens = true;
									include_this_entry = true;
								} else {
									throw std::runtime_error("Malformed LIST definition: mismatched '('");
								}
							} break;

							case InkToken::Text: {
								if (std::string stripped = strip_string_edges(this_token.get_text_contents(), true, true, true); !stripped.empty()) {
									this_entry_name = stripped;
								}
							} break;

							case InkToken::Equal: {
								if (all_tokens[token_index - 1].token == InkToken::Text && next_token_is(all_tokens, token_index, InkToken::Text)) {
									try {
										this_entry_value = std::stoll(all_tokens[token_index + 1].get_text_contents());
										++token_index;
									} catch (...) {
										throw std::runtime_error("Malformed LIST definition: invalid entry value");
									}
								} else {
									throw std::runtime_error("Malformed LIST definition: misplaced '='");
								}
							} break;

							case InkToken::Comma: {
								if (!this_entry_name.empty()) {
									(add_entry)();
								} else {
									throw std::runtime_error("Malformed LIST definition: misplaced ','");
								}
							} break;

							case InkToken::RightParen: {
								if (in_list_parens) {
									in_list_parens = false;
								} else {
									throw std::runtime_error("Malformed LIST definition: mismatched ')'");
								}
							} break;

							default: {
								throw std::runtime_error("Malformed LIST definition");
							} break;
						}

						++token_index;
					}

					--token_index;

					if (!this_entry_name.empty()) {
						(add_entry)();
					}

					Uuid new_list_uuid = story_variable_info.add_list_definition(identifier, entries);
					result_object = new InkObjectList(identifier, new_list_uuid, entries);
				} else {
					throw std::runtime_error("Malformed LIST definition");
				}
			} else {
				result_object = new InkObjectText("LIST");
			}
		} break;

		case InkToken::KeywordInclude: {
			++token_index;
			std::string path;
			path.reserve(255);
			while (token_index < all_tokens.size() && all_tokens[token_index].token != InkToken::NewLine) {
				path += all_tokens[token_index].get_text_contents();
				++token_index;
			}

			InkCompiler include_compiler;
			include_compiler.set_current_uuid(current_uuid);
			InkStory include_story = include_compiler.compile_file(strip_string_edges(path, true, true, true));
			current_uuid = include_compiler.get_current_uuid();
			for (const auto& included_knot : include_story.story_data->knots) {
				bool already_exists = false;
				for (Knot& existing_knot : story_knots) {
					if (existing_knot.name == included_knot.second.name) {
						existing_knot.append_knot(included_knot.second);
						already_exists = true;
						break;
					}
				}

				if (!already_exists) {
					story_knots.push_back(included_knot.second);
				}
			}

			include_story.story_data->knots.clear();
			end_line = true;
		} break;

		case InkToken::Text: {
			std::string text_stripped = strip_string_edges(token.text_contents, true, true, true);
			std::string text_notabs = strip_string_edges(token.text_contents);
			result_object = new InkObjectText(token.text_contents);
		} break;

		default: break;
	}

	at_line_start = end_line;

	last_object = result_object;
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
