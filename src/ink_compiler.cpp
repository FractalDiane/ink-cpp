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

#include "expression_parser/expression_parser.h"

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
		//{',', InkToken::Comma},
	};

	static const std::unordered_map<std::string, InkToken> TokenKeywords = {
		{"VAR", InkToken::KeywordVar},
		{"CONST", InkToken::KeywordConst},
		{"LIST", InkToken::KeywordList},
		{"function", InkToken::KeywordFunction},
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
			this_token.token = keyword_token->second;
			current_text.clear();
		}

		if (end_text) {
			try_add_text_token(result, current_text);
			current_text.clear();
		}

		if (this_token.token != InkToken::INVALID) {
			result.push_back(this_token);
			any_tokens_this_line = this_token.token != InkToken::NewLine;
			at_line_start = this_token.token == InkToken::NewLine;;
		}

		++index;
	}

	try_add_text_token(result, current_text);

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

			bool appended_text = false;
			if (add_this_object) {
				if (this_token_object->get_id() == ObjectId::Text && last_token_object && last_token_object->get_id() == ObjectId::Text) {
					static_cast<InkObjectText*>(result_knots.back().objects.back())->append_text(static_cast<InkObjectText*>(this_token_object)->get_text_contents());
					delete this_token_object;
					this_token_object = nullptr;
					appended_text = true;
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

			if (!appended_text) {
				last_token_object = this_token_object;
			}
		} else {
			last_token_object = nullptr;
		}
		
		++token_index;
	}

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

				tag_contents += all_tokens[token_index].get_text_contents();
				++token_index;
			}

			result_object = new InkObjectTag(strip_string_edges(tag_contents, true, true, true));
		} break; 

		case InkToken::Equal: {
			if (at_line_start) {
				//if (next_token_is_sequence(all_tokens, token_index, {InkToken::Equal, InkToken::Equal})) {
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
						if (new_knot.is_function) {
							declared_functions.insert(new_knot_name);
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

						story_knots.push_back(new_knot);

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
					Stitch new_stitch;
					new_stitch.name = new_stitch_name;
					new_stitch.uuid = Uuid(current_uuid++);
					new_stitch.type = WeaveContentType::Stitch;
					new_stitch.index = static_cast<std::uint16_t>(story_knots.back().objects.size());

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
			if (at_line_start && token.count > choice_level) {
				++choice_level;
				std::vector<InkChoiceEntry> choice_options;
				//bool has_gather = false;
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
							//has_gather = true;
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

					choice_stack.push_back({.sticky = current_choice_sticky});

					if (next_token_is_sequence(all_tokens, token_index, {InkToken::LeftParen, InkToken::Text, InkToken::RightParen})) {
						GatherPoint label;
						label.name = all_tokens[token_index + 2].text_contents;
						label.uuid = Uuid(current_uuid++);
						label.type = WeaveContentType::GatherPoint;
						label.index = static_cast<std::uint16_t>(story_knots.back().objects.size());
						label.in_choice = true;
						label.choice_index = static_cast<std::uint16_t>(choice_options.size());
						choice_stack.back().label = label;

						std::vector<GatherPoint>& gather_points = 
						!story_knots.back().stitches.empty() && story_knots.back().objects.size() >= story_knots.back().stitches[0].index
						? story_knots.back().stitches.back().gather_points
						: story_knots.back().gather_points;

						gather_points.push_back(label);

						token_index += 4;
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

							if (in_choice_object->get_id() == ObjectId::Choice) {
								--token_index;
							}
						}
						
						++token_index;

						if (!past_choice_initial_braces && in_choice_token.token != InkToken::LeftBrace && (in_choice_token.token != InkToken::Text || !strip_string_edges(in_choice_token.text_contents, true, true, true).empty())) {
							past_choice_initial_braces = true;
						}
					}

					in_choice_line = false;
					choice_options.push_back(choice_stack.back());
					choice_stack.pop_back();
				}

				in_choice_line = false;
				result_object = new InkObjectChoice(choice_options);
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

			std::vector<std::vector<InkObject*>> items = {{}};
			std::vector<std::pair<std::vector<ExpressionParser::Token*>, Knot>> items_conditions = {{{}, Knot()}};
			Knot items_else;
			std::vector<std::string> text_items;
			std::vector<ExpressionParser::Token*> switch_expression;

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
							if (auto& current_condition = items_conditions.back().first; current_condition.empty()) {
								std::string condition_string;
								condition_string.reserve(50);
								for (InkObject* object : items[0]) {
									condition_string += object->to_string();
									delete object;
								}

								try {
									current_condition = ExpressionParser::tokenize_and_shunt_expression(condition_string, {}, declared_functions);
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

											items_conditions.back().first.clear();
											items_conditions.back().second.objects.clear();
										}
										
										std::string this_condition;
										this_condition.reserve(50);
										++token_index;
										while (token_index < all_tokens.size() && all_tokens[token_index].token != InkToken::Colon) {
											this_condition += all_tokens[token_index].get_text_contents();
											++token_index;
										}

										try {
											std::vector<ExpressionParser::Token*> shunted = ExpressionParser::tokenize_and_shunt_expression(this_condition, {}, declared_functions);
											if (items_conditions.back().first.empty()) {
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
							InkObject* compiled_object = compile_token(all_tokens, all_tokens[token_index], story_knots);
							if (compiled_object->has_any_contents(true)) {
								if (is_conditional) {
									Knot& target_array = in_else ? items_else : items_conditions.back().second;
									target_array.objects.push_back(compiled_object);
								} else if (!items.empty()) {
									items.back().push_back(compiled_object);
								} else {
									delete compiled_object;
								}
							} else {
								delete compiled_object;
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
					std::vector<ExpressionParser::Token*> condition_shunted = ExpressionParser::tokenize_and_shunt_expression(condition, {}, declared_functions);
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
						std::vector<ExpressionParser::Token*> shunted = ExpressionParser::tokenize_and_shunt_expression(all_text, {}, declared_functions);
						result_object = new InkObjectInterpolation(shunted);
					} catch (...) {
						throw std::runtime_error("Malformed interpolation");
					}
				}
			}

			if (delete_items) {
				for (auto& vec : items) {
					for (InkObject* object : vec) {
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

		case InkToken::Arrow: {
			if (next_token_is(all_tokens, token_index, InkToken::Text) && !strip_string_edges(all_tokens[token_index + 1].text_contents, true, true, true).empty()) {
				if (!in_parens) {
					std::string target = strip_string_edges(all_tokens[token_index + 1].text_contents, true, true, true);
					std::vector<ExpressionParser::Token*> target_tokens;
					try {
						target_tokens = ExpressionParser::tokenize_and_shunt_expression(target, {}, declared_functions);
					} catch (...) {
						throw std::runtime_error("Illegal value in divert target");
					}
		
					std::vector<std::vector<ExpressionParser::Token*>> arguments;
					if (next_token_is(all_tokens, token_index + 1, InkToken::LeftParen)) {
						token_index += 3;

						std::string all_args;
						all_args.reserve(50);
						while (all_tokens[token_index].token != InkToken::RightParen) {
							all_args += all_tokens[token_index].text_contents;
							++token_index;
						}

						--token_index;

						std::vector<std::string> split = split_string(all_args, ',', true);
						for (const std::string& arg : split) {
							try {
								std::vector<ExpressionParser::Token*> tokenized = ExpressionParser::tokenize_and_shunt_expression(arg, {}, declared_functions);
								arguments.push_back(tokenized);
							} catch (...) {
								throw std::runtime_error("Malformed knot argument");
							}
						}
					}

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
					result_object = new InkObjectText("->");
				}
			} else if (next_token_is(all_tokens, token_index, InkToken::Arrow)) {
				std::vector<ExpressionParser::Token*> target_tokens;
				if (next_token_is(all_tokens, token_index + 1, InkToken::Text)) {
					std::string target = strip_string_edges(all_tokens[token_index + 2].text_contents, true, true, true);
					try {
						target_tokens = ExpressionParser::tokenize_and_shunt_expression(target, {}, declared_functions);
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
				std::vector<GatherPoint>& gather_points = 
				!story_knots.back().stitches.empty() && story_knots.back().objects.size() >= story_knots.back().stitches[0].index
				? story_knots.back().stitches.back().gather_points
				: story_knots.back().gather_points;
				
				GatherPoint new_gather_point;
				new_gather_point.uuid = Uuid(current_uuid++);
				new_gather_point.type = WeaveContentType::GatherPoint;
				new_gather_point.index = static_cast<std::uint16_t>(story_knots.back().objects.size());
				new_gather_point.level = token.count;

				if (next_token_is_sequence(all_tokens, token_index, {InkToken::LeftParen, InkToken::Text, InkToken::RightParen})) {
					new_gather_point.name = all_tokens[token_index + 2].text_contents;
					token_index += 3;
				}

				if (!story_knots.back().objects.empty() && story_knots.back().objects.back()->get_id() == ObjectId::Choice) {
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
					std::vector<ExpressionParser::Token*> expression_shunted = ExpressionParser::tokenize_and_shunt_expression(expression, {}, declared_functions);
					result_object = new InkObjectLogic(expression_shunted);
				} catch (...) {
					throw std::runtime_error("Malformed logic statement");
				}

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
		
		case InkToken::KeywordVar:
		case InkToken::KeywordConst: {
			if (at_line_start) {
				if (next_token_is_sequence(all_tokens, token_index, {InkToken::Text, InkToken::Equal})) {
					const std::string& identifier = strip_string_edges(all_tokens[token_index + 1].text_contents, true, true, true);
					
					token_index += 3;
					std::string expression;
					expression.reserve(50);
					while (all_tokens[token_index].token != InkToken::NewLine) {
						expression += all_tokens[token_index].get_text_contents();
						++token_index;
					}

					--token_index;

					try {
						std::vector<ExpressionParser::Token*> expression_shunted = ExpressionParser::tokenize_and_shunt_expression(expression, {}, declared_functions);
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
