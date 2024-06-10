#pragma once

#include "runtime/ink_story_data.h"
#include "runtime/ink_story_state.h"

#include "expression_parser/expression_parser.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <string_view>
#include <functional>

class InkStory {
private:
	InkStoryData* story_data;
	InkStoryState story_state;

private:
	void init_story();
	void bind_ink_functions();

	//std::optional<ExpressionParser::Variant> divert_to_function_knot(const std::string& knot);
	//InkStoryEvalResult run_thread(const GetContentResult& target);

public:
	explicit InkStory() : story_data{nullptr} {}
	explicit InkStory(InkStoryData* data) : story_data{data} { init_story(); }
	explicit InkStory(const std::string& inkb_file);
	~InkStory() { delete story_data; }

	InkStory(const InkStory& from) = delete;
	InkStory& operator=(const InkStory& other) = delete;

	InkStory(InkStory&& from) : story_data{from.story_data} {
		from.story_data = nullptr;
		init_story();
	}

	InkStory& operator=(InkStory&& other) {
		if (this != &other) {
			story_data = other.story_data;
			other.story_data = nullptr;
			init_story();
		}

		return *this;
	}

	InkStoryData* get_story_data() const { return story_data; }
	void print_info() const;

	bool can_continue();
	std::string continue_story();
	std::string continue_story_maximally();

	std::vector<std::string> get_current_choices() const;
	const std::vector<std::string>& get_current_tags() const;

	void choose_choice_index(std::size_t index);

	std::optional<ExpressionParser::Variant> get_variable(const std::string& name) const;
	void set_variable(const std::string& name, ExpressionParser::Variant&& value);

	void observe_variable(const std::string& variable_name, ExpressionParser::VariableObserverFunc callback);
	void unobserve_variable(const std::string& variable_name);
	void unobserve_variable(ExpressionParser::VariableObserverFunc observer);
	void unobserve_variable(const std::string& variable_name, ExpressionParser::VariableObserverFunc observer);
};
