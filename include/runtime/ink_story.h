#pragma once

#include "runtime/ink_story_data.h"
#include "runtime/ink_story_state.h"

#include "expression_parser/expression_parser.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <string_view>

class InkStory {
private:
	InkStoryData* story_data;
	InkStoryState story_state;

	static ExpressionParser::FunctionMap builtin_functions;
	static bool builtins_initialized;

private:
	void init_story();
	void bind_ink_functions();

public:
	explicit InkStory(InkStoryData* data) : story_data{data} { init_story(); }
	explicit InkStory(const std::string& inkb_file);
	~InkStory() { delete story_data; }

	InkStoryData* get_story_data() const { return story_data; }
	void print_info() const;

	bool can_continue();
	std::string continue_story();
	std::string continue_story_maximally();

	const std::vector<std::string>& get_current_choices() const;
	const std::vector<std::string>& get_current_tags() const;

	void choose_choice_index(std::size_t index);

	ExpressionParser::PackedToken get_variable(const std::string& name) const;
	void set_variable(const std::string& name, const ExpressionParser::PackedToken& value);
};
