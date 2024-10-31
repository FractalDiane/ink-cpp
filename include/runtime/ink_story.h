#pragma once

#include "runtime/ink_story_data.h"
#include "runtime/ink_story_state.h"

#include "expression_parser/token.h"
#include "expression_parser/expression_parser.h"

#include "types/ink_list.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <string_view>
#include <functional>
#include <concepts>

template <typename T>
concept ConvertibleToVariant = std::convertible_to<T, ExpressionParserV2::Variant>;
template <typename T>
concept ConvertibleFromVariant = std::convertible_to<ExpressionParserV2::Variant, T>;

class InkStory {
private:
	InkStoryData* story_data;
	InkStoryState story_state;

	friend class InkCompiler;

private:
	void init_story();
	void bind_ink_functions();

	void try_remove_upper_knots(const GetContentResult& target);
	void apply_knot_args(const InkWeaveContent* target, InkStoryEvalResult& eval_result);
	void update_visit_count_variables(std::vector<ExpressionParserV2::ShuntedExpression*>&& expressions);

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
	const InkStoryState& get_story_state() const { return story_state; }
	void print_info() const;

	bool can_continue() const;
	std::string continue_story();
	std::string continue_story_maximally();

	std::vector<std::string> get_current_choices() const;
	const std::vector<std::string>& get_current_tags() const;

	void choose_choice_index(std::size_t index);

	std::optional<ExpressionParserV2::Variant> get_variable(const std::string& name) const;
	void set_variable(const std::string& name, ExpressionParserV2::Variant&& value);

	void observe_variable(const std::string& variable_name, ExpressionParserV2::VariableObserverFunc callback);
	void unobserve_variable(const std::string& variable_name);
	void unobserve_variable(ExpressionParserV2::VariableObserverFunc observer);
	void unobserve_variable(const std::string& variable_name, ExpressionParserV2::VariableObserverFunc observer);

	const std::unordered_map<Uuid, InkListDefinition>& get_list_definitions() const { return story_state.variable_info.defined_lists.defined_lists; }

	void bind_external_function_generic(const std::string& function_name, ExpressionParserV2::InkFunction function, bool lookahead_safe = false);

	//void bind_external_function(const std::string& function_name, std::function<void()> function, bool lookahead_safe = false);
	void bind_external_function(const std::string& function_name, void(*function)(), bool lookahead_safe = false);

	//template <ConvertibleToVariant A1>
	//void bind_external_function(const std::string& function_name, std::function<void(A1)> function, bool lookahead_safe = false);
	template <ConvertibleToVariant A1>
	void bind_external_function(const std::string& function_name, void(*function)(A1), bool lookahead_safe = false);

	//template <ConvertibleToVariant A1, ConvertibleToVariant A2>
	//void bind_external_function(const std::string& function_name, std::function<void(A1, A2)> function, bool lookahead_safe = false);
	template <ConvertibleToVariant A1, ConvertibleToVariant A2>
	void bind_external_function(const std::string& function_name, void(*function)(A1, A2), bool lookahead_safe = false);

	//template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3>
	//void bind_external_function(const std::string& function_name, std::function<void(A1, A2, A3)> function, bool lookahead_safe = false);
	template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3>
	void bind_external_function(const std::string& function_name, void(*function)(A1, A2, A3), bool lookahead_safe = false);

	//template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4>
	//void bind_external_function(const std::string& function_name, std::function<void(A1, A2, A3, A4)> function, bool lookahead_safe = false);
	template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4>
	void bind_external_function(const std::string& function_name, void(*function)(A1, A2, A3, A4), bool lookahead_safe = false);

	//template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5>
	//void bind_external_function(const std::string& function_name, std::function<void(A1, A2, A3, A4, A5)> function, bool lookahead_safe = false);
	template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5>
	void bind_external_function(const std::string& function_name, void(*function)(A1, A2, A3, A4, A5), bool lookahead_safe = false);

	//template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6>
	//void bind_external_function(const std::string& function_name, std::function<void(A1, A2, A3, A4, A5, A6)> function, bool lookahead_safe = false);
	template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6>
	void bind_external_function(const std::string& function_name, void(*function)(A1, A2, A3, A4, A5, A6), bool lookahead_safe = false);

	//template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6, ConvertibleToVariant A7>
	//void bind_external_function(const std::string& function_name, std::function<void(A1, A2, A3, A4, A5, A6, A7)> function, bool lookahead_safe = false);
	template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6, ConvertibleToVariant A7>
	void bind_external_function(const std::string& function_name, void(*function)(A1, A2, A3, A4, A5, A6, A7), bool lookahead_safe = false);

	//template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6, ConvertibleToVariant A7, ConvertibleToVariant A8>
	//void bind_external_function(const std::string& function_name, std::function<void(A1, A2, A3, A4, A5, A6, A7, A8)> function, bool lookahead_safe = false);
	template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6, ConvertibleToVariant A7, ConvertibleToVariant A8>
	void bind_external_function(const std::string& function_name, void(*function)(A1, A2, A3, A4, A5, A6, A7, A8), bool lookahead_safe = false);

	//template <ConvertibleFromVariant R>
	//void bind_external_function(const std::string& function_name, std::function<R()> function, bool lookahead_safe = false);
	template <ConvertibleFromVariant R>
	void bind_external_function(const std::string& function_name, R(*function)(), bool lookahead_safe = false);

	//template <ConvertibleFromVariant R, ConvertibleToVariant A1>
	//void bind_external_function(const std::string& function_name, std::function<R(A1)> function, bool lookahead_safe = false);
	template <ConvertibleFromVariant R, ConvertibleToVariant A1>
	void bind_external_function(const std::string& function_name, R(*function)(A1), bool lookahead_safe = false);

	//template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2>
	//void bind_external_function(const std::string& function_name, std::function<R(A1, A2)> function, bool lookahead_safe = false);
	template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2>
	void bind_external_function(const std::string& function_name, R(*function)(A1, A2), bool lookahead_safe = false);

	//template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3>
	//void bind_external_function(const std::string& function_name, std::function<R(A1, A2, A3)> function, bool lookahead_safe = false);
	template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3>
	void bind_external_function(const std::string& function_name, R(*function)(A1, A2, A3), bool lookahead_safe = false);

	//template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4>
	//void bind_external_function(const std::string& function_name, std::function<R(A1, A2, A3, A4)> function, bool lookahead_safe = false);
	template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4>
	void bind_external_function(const std::string& function_name, R(*function)(A1, A2, A3, A4), bool lookahead_safe = false);

	//template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5>
	//void bind_external_function(const std::string& function_name, std::function<R(A1, A2, A3, A4, A5)> function, bool lookahead_safe = false);
	template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5>
	void bind_external_function(const std::string& function_name, R(*function)(A1, A2, A3, A4, A5), bool lookahead_safe = false);

	//template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6>
	//void bind_external_function(const std::string& function_name, std::function<R(A1, A2, A3, A4, A5, A6)> function, bool lookahead_safe = false);
	template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6>
	void bind_external_function(const std::string& function_name, R(*function)(A1, A2, A3, A4, A5, A6), bool lookahead_safe = false);

	//template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6, ConvertibleToVariant A7>
	//void bind_external_function(const std::string& function_name, std::function<R(A1, A2, A3, A4, A5, A6, A7)> function, bool lookahead_safe = false);
	template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6, ConvertibleToVariant A7>
	void bind_external_function(const std::string& function_name, R(*function)(A1, A2, A3, A4, A5, A6, A7), bool lookahead_safe = false);

	//template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6, ConvertibleToVariant A7, ConvertibleToVariant A8>
	//void bind_external_function(const std::string& function_name, std::function<R(A1, A2, A3, A4, A5, A6, A7, A8)> function, bool lookahead_safe = false);
	template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6, ConvertibleToVariant A7, ConvertibleToVariant A8>
	void bind_external_function(const std::string& function_name, R(*function)(A1, A2, A3, A4, A5, A6, A7, A8), bool lookahead_safe = false);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*template <ConvertibleToVariant A1>
void InkStory::bind_external_function(const std::string& function_name, std::function<void(A1)> function, bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		function(static_cast<A1>(args[0]));
		return ExpressionParserV2::Variant();
	}, lookahead_safe);
}*/

template <ConvertibleToVariant A1>
void InkStory::bind_external_function(const std::string& function_name, void(*function)(A1), bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		function(static_cast<A1>(args[0]));
		return ExpressionParserV2::Variant();
	}, lookahead_safe);
}

/*template <ConvertibleToVariant A1, ConvertibleToVariant A2>
void InkStory::bind_external_function(const std::string& function_name, std::function<void(A1, A2)> function, bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1])
		);
		return ExpressionParserV2::Variant();
	}, lookahead_safe);
}*/

template <ConvertibleToVariant A1, ConvertibleToVariant A2>
void InkStory::bind_external_function(const std::string& function_name, void(*function)(A1, A2), bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1])
		);
		return ExpressionParserV2::Variant();
	}, lookahead_safe);
}

/*template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3>
void InkStory::bind_external_function(const std::string& function_name, std::function<void(A1, A2, A3)> function, bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2])
		);
		return ExpressionParserV2::Variant();
	}, lookahead_safe);
}*/

template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3>
void InkStory::bind_external_function(const std::string& function_name, void(*function)(A1, A2, A3), bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2])
		);
		return ExpressionParserV2::Variant();
	}, lookahead_safe);
}

/*template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4>
void InkStory::bind_external_function(const std::string& function_name, std::function<void(A1, A2, A3, A4)> function, bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2]),
			static_cast<A4>(args[3])
		);
		return ExpressionParserV2::Variant();
	}, lookahead_safe);
}*/

template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4>
void InkStory::bind_external_function(const std::string& function_name, void(*function)(A1, A2, A3, A4), bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2]),
			static_cast<A4>(args[3])
		);
		return ExpressionParserV2::Variant();
	}, lookahead_safe);
}

/*template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5>
void InkStory::bind_external_function(const std::string& function_name, std::function<void(A1, A2, A3, A4, A5)> function, bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2]),
			static_cast<A4>(args[3]),
			static_cast<A5>(args[4])
		);
		return ExpressionParserV2::Variant();
	}, lookahead_safe);
}*/

template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5>
void InkStory::bind_external_function(const std::string& function_name, void(*function)(A1, A2, A3, A4, A5), bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2]),
			static_cast<A4>(args[3]),
			static_cast<A5>(args[4])
		);
		return ExpressionParserV2::Variant();
	}, lookahead_safe);
}

/*template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6>
void InkStory::bind_external_function(const std::string& function_name, std::function<void(A1, A2, A3, A4, A5, A6)> function, bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2]),
			static_cast<A4>(args[3]),
			static_cast<A5>(args[4]),
			static_cast<A6>(args[5])
		);
		return ExpressionParserV2::Variant();
	}, lookahead_safe);
}*/

template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6>
void InkStory::bind_external_function(const std::string& function_name, void(*function)(A1, A2, A3, A4, A5, A6), bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2]),
			static_cast<A4>(args[3]),
			static_cast<A5>(args[4]),
			static_cast<A6>(args[5])
		);
		return ExpressionParserV2::Variant();
	}, lookahead_safe);
}

/*template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6, ConvertibleToVariant A7>
void InkStory::bind_external_function(const std::string& function_name, std::function<void(A1, A2, A3, A4, A5, A6, A7)> function, bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2]),
			static_cast<A4>(args[3]),
			static_cast<A5>(args[4]),
			static_cast<A6>(args[5]),
			static_cast<A7>(args[6])
		);
		return ExpressionParserV2::Variant();
	}, lookahead_safe);
}*/

template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6, ConvertibleToVariant A7>
void InkStory::bind_external_function(const std::string& function_name, void(*function)(A1, A2, A3, A4, A5, A6, A7), bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2]),
			static_cast<A4>(args[3]),
			static_cast<A5>(args[4]),
			static_cast<A6>(args[5]),
			static_cast<A7>(args[6])
		);
		return ExpressionParserV2::Variant();
	}, lookahead_safe);
}

/*template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6, ConvertibleToVariant A7, ConvertibleToVariant A8>
void InkStory::bind_external_function(const std::string& function_name, std::function<void(A1, A2, A3, A4, A5, A6, A7, A8)> function, bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2]),
			static_cast<A4>(args[3]),
			static_cast<A5>(args[4]),
			static_cast<A6>(args[5]),
			static_cast<A7>(args[6]),
			static_cast<A8>(args[7])
		);
		return ExpressionParserV2::Variant();
	}, lookahead_safe);
}*/

template <ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6, ConvertibleToVariant A7, ConvertibleToVariant A8>
void InkStory::bind_external_function(const std::string& function_name, void(*function)(A1, A2, A3, A4, A5, A6, A7, A8), bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2]),
			static_cast<A4>(args[3]),
			static_cast<A5>(args[4]),
			static_cast<A6>(args[5]),
			static_cast<A7>(args[6]),
			static_cast<A8>(args[7])
		);
		return ExpressionParserV2::Variant();
	}, lookahead_safe);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*template <ConvertibleFromVariant R, ConvertibleToVariant A1>
void InkStory::bind_external_function(const std::string& function_name, std::function<R(A1)> function, bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		return function(static_cast<A1>(args[0]));
	}, lookahead_safe);
}*/

template <ConvertibleFromVariant R, ConvertibleToVariant A1>
void InkStory::bind_external_function(const std::string& function_name, R(*function)(A1), bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		return function(static_cast<A1>(args[0]));
	}, lookahead_safe);
}

/*template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2>
void InkStory::bind_external_function(const std::string& function_name, std::function<R(A1, A2)> function, bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		return function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1])
		);
	}, lookahead_safe);
}*/

template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2>
void InkStory::bind_external_function(const std::string& function_name, R(*function)(A1, A2), bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		return function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1])
		);
	}, lookahead_safe);
}

/*template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3>
void InkStory::bind_external_function(const std::string& function_name, std::function<R(A1, A2, A3)> function, bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		return function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2])
		);
	}, lookahead_safe);
}*/

template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3>
void InkStory::bind_external_function(const std::string& function_name, R(*function)(A1, A2, A3), bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		return function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2])
		);
	}, lookahead_safe);
}

/*template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4>
void InkStory::bind_external_function(const std::string& function_name, std::function<R(A1, A2, A3, A4)> function, bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		return function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2]),
			static_cast<A4>(args[3])
		);
	}, lookahead_safe);
}*/

template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4>
void InkStory::bind_external_function(const std::string& function_name, R(*function)(A1, A2, A3, A4), bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		return function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2]),
			static_cast<A4>(args[3])
		);
	}, lookahead_safe);
}

/*template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5>
void InkStory::bind_external_function(const std::string& function_name, std::function<R(A1, A2, A3, A4, A5)> function, bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		return function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2]),
			static_cast<A4>(args[3]),
			static_cast<A5>(args[4])
		);
	}, lookahead_safe);
}*/

template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5>
void InkStory::bind_external_function(const std::string& function_name, R(*function)(A1, A2, A3, A4, A5), bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		return function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2]),
			static_cast<A4>(args[3]),
			static_cast<A5>(args[4])
		);
	}, lookahead_safe);
}

/*template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6>
void InkStory::bind_external_function(const std::string& function_name, std::function<R(A1, A2, A3, A4, A5, A6)> function, bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		return function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2]),
			static_cast<A4>(args[3]),
			static_cast<A5>(args[4]),
			static_cast<A6>(args[5])
		);
	}, lookahead_safe);
}*/

template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6>
void InkStory::bind_external_function(const std::string& function_name, R(*function)(A1, A2, A3, A4, A5, A6), bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		return function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2]),
			static_cast<A4>(args[3]),
			static_cast<A5>(args[4]),
			static_cast<A6>(args[5])
		);
	}, lookahead_safe);
}

/*template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6, ConvertibleToVariant A7>
void InkStory::bind_external_function(const std::string& function_name, std::function<R(A1, A2, A3, A4, A5, A6, A7)> function, bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		return function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2]),
			static_cast<A4>(args[3]),
			static_cast<A5>(args[4]),
			static_cast<A6>(args[5]),
			static_cast<A7>(args[6])
		);
	}, lookahead_safe);
}*/

template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6, ConvertibleToVariant A7>
void InkStory::bind_external_function(const std::string& function_name, R(*function)(A1, A2, A3, A4, A5, A6, A7), bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		return function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2]),
			static_cast<A4>(args[3]),
			static_cast<A5>(args[4]),
			static_cast<A6>(args[5]),
			static_cast<A7>(args[6])
		);
	}, lookahead_safe);
}

/*template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6, ConvertibleToVariant A7, ConvertibleToVariant A8>
void InkStory::bind_external_function(const std::string& function_name, std::function<R(A1, A2, A3, A4, A5, A6, A7, A8)> function, bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		return function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2]),
			static_cast<A4>(args[3]),
			static_cast<A5>(args[4]),
			static_cast<A6>(args[5]),
			static_cast<A7>(args[6]),
			static_cast<A8>(args[7])
		);
	}, lookahead_safe);
}*/

template <ConvertibleFromVariant R, ConvertibleToVariant A1, ConvertibleToVariant A2, ConvertibleToVariant A3, ConvertibleToVariant A4, ConvertibleToVariant A5, ConvertibleToVariant A6, ConvertibleToVariant A7, ConvertibleToVariant A8>
void InkStory::bind_external_function(const std::string& function_name, R(*function)(A1, A2, A3, A4, A5, A6, A7, A8), bool lookahead_safe) {
	bind_external_function_generic(function_name, [function](const std::vector<ExpressionParserV2::Variant>& args) {
		return function(
			static_cast<A1>(args[0]),
			static_cast<A2>(args[1]),
			static_cast<A3>(args[2]),
			static_cast<A4>(args[3]),
			static_cast<A5>(args[4]),
			static_cast<A6>(args[5]),
			static_cast<A7>(args[6]),
			static_cast<A8>(args[7])
		);
	}, lookahead_safe);
}
