#include "runtime/ink_story.h"

#include "objects/ink_object.h"
#include "objects/ink_object_choice.h"
#include "objects/ink_object_choicetextmix.h"
#include "objects/ink_object_conditional.h"
#include "objects/ink_object_divert.h"
#include "objects/ink_object_globalvariable.h"
#include "objects/ink_object_glue.h"
#include "objects/ink_object_interpolation.h"
#include "objects/ink_object_linebreak.h"
#include "objects/ink_object_sequence.h"
#include "objects/ink_object_tag.h"
#include "objects/ink_object_text.h"

#include "ink_utils.h"

#include <fstream>
#include <filesystem>
#include <algorithm>
#include <format>
#include <ranges>
#include <cmath>

#include <stdexcept>
#include <iostream>

InkStory::InkStory(const std::string& inkb_file) {
	std::ifstream infile{inkb_file, std::ios::binary};

	std::size_t infile_size = static_cast<std::size_t>(std::filesystem::file_size(inkb_file));
	std::vector<std::uint8_t> bytes(infile_size);

	infile.read(reinterpret_cast<char*>(bytes.data()), infile_size);
	infile.close();

	// HACK: make this less bad and more fail safe
	std::size_t index = 0;
	constexpr const char* expected_header = "INKB";
	std::string header;
	for (std::size_t i = 0; i < 4; ++i) {
		header += static_cast<signed char>(bytes[index++]);
	}

	if (header != expected_header) {
		throw std::runtime_error("Not a valid inkb file (incorrect header)");
	}

	//std::uint8_t version = bytes[index++];
	++index;

	Serializer<std::uint16_t> dssize;
	std::uint16_t knots_count = dssize(bytes, index);

	std::vector<Knot> knots;
	for (std::size_t i = 0; i < knots_count; ++i) {
		Serializer<std::string> dsname;
		std::string this_knot_name = dsname(bytes, index);

		std::uint16_t this_knot_size = dssize(bytes, index);

		std::vector<InkObject*> this_knot_contents;
		this_knot_contents.reserve(this_knot_size);

		for (std::size_t j = 0; j < this_knot_size; ++j) {
			InkObject* this_object = nullptr;
			ObjectId this_id = static_cast<ObjectId>(bytes[index++]);
			// TODO: find out if there's a better way to do this
			switch (this_id) {
				case ObjectId::Text: {
					this_object = (new InkObjectText())->populate_from_bytes(bytes, index);
				} break;

				case ObjectId::LineBreak: {
					this_object = new InkObjectLineBreak();
				} break;

				case ObjectId::Tag: {
					//this_object = (new InkObjectTag(""))->populate_from_bytes(bytes, index);
				} break;

				case ObjectId::Divert: {
					//this_object = (new InkObjectDivert(""))->populate_from_bytes(bytes, index);
				} break;

				default: break;
			}

			if (this_object) {
				this_knot_contents.push_back(this_object);
			}
		}

		std::uint16_t this_knot_stitches_size = dssize(bytes, index);
		std::vector<Stitch> this_knot_stitches;
		this_knot_stitches.reserve(this_knot_stitches_size);

		Serializer<Stitch> dsstitch;
		for (std::size_t j = 0; j < this_knot_stitches_size; ++j) {
			Stitch this_stitch = dsstitch(bytes, index);
			this_knot_stitches.push_back(this_stitch);
		}

		// TODO: figure out why this doesn't work
		//Serializer<Stitch> dsstitch;
		//dsstitch(bytes, index);
		//Serializer<std::vector<Stitch>> dsstitches;
		//dsstitches(bytes, index);
	
		Knot knot;
		knot.name = this_knot_name;
		knot.objects = this_knot_contents;
		knot.stitches = this_knot_stitches;
		knots.push_back(knot);
	}

	story_data = new InkStoryData(knots);
	init_story();
}

void InkStory::print_info() const {
	story_data->print_info();
}

void InkStory::init_story() {
	for (auto& knot : story_data->knots) {
		auto knot_stats = story_state.story_tracking.knot_stats.insert({knot.second.uuid, InkStoryTracking::KnotStats(knot.second.name)});
		for (Stitch& stitch : knot.second.stitches) {
			auto stitch_stats = story_state.story_tracking.stitch_stats.insert({stitch.uuid, InkStoryTracking::StitchStats(stitch.name)});
			knot_stats.first->second.stitches.push_back(stitch.uuid);

			for (GatherPoint& gather_point : stitch.gather_points) {
				story_state.story_tracking.gather_point_stats.insert({gather_point.uuid, InkStoryTracking::SubKnotStats(gather_point.name)});
				stitch_stats.first->second.gather_points.push_back(gather_point.uuid);
			}
		}

		for (GatherPoint& gather_point : knot.second.gather_points) {
			story_state.story_tracking.gather_point_stats.insert({gather_point.uuid, InkStoryTracking::SubKnotStats(gather_point.name)});
			knot_stats.first->second.gather_points.push_back(gather_point.uuid);
		}
	}

	bind_ink_functions();

	story_state.current_knots_stack = {{&(story_data->knots[story_data->knot_order[0]]), 0}};
}

void InkStory::bind_ink_functions() {
	using namespace ExpressionParser;

	#define EXP_FUNC(name, body) story_state.functions.insert({name, [this](TokenStack& stack, VariableMap& variables, const VariableMap& constants) body});

	EXP_FUNC("CHOICE_COUNT", { return new TokenNumberInt(story_state.current_choices.size()); });
	EXP_FUNC("TURNS", { return new TokenNumberInt(story_state.total_choices_taken); });

	EXP_FUNC("TURNS_SINCE", {
		std::string knot = as_string(stack.top()->get_variant_value(variables, constants).value());
		
		if (GetContentResult content = story_data->get_content(knot, story_state.current_knot().knot, story_state.current_stitch); content.found_any) {
			InkStoryTracking::SubKnotStats stats;
			if (story_state.story_tracking.get_content_stats(content.get_target(), stats)) {
				stack.pop();
				return new TokenNumberInt(stats.turns_since_visited);
			}
		}
		
		stack.pop();
		return new TokenNumberInt(-1);
	});

	EXP_FUNC("SEED_RANDOM", {
		std::int64_t seed = as_int(stack.top()->get_variant_value(variables, constants).value());
		stack.pop();

		story_state.rng.seed(static_cast<unsigned int>(seed));
		return nullptr;
	});

	EXP_FUNC("RANDOM", {
		std::int64_t to = as_int(stack.top()->get_variant_value(variables, constants).value());
		stack.pop();
		std::int64_t from = as_int(stack.top()->get_variant_value(variables, constants).value());
		stack.pop();

		std::int64_t result = randi_range(from, to, story_state.rng);
		return new TokenNumberInt(result);
	});

	#undef EXP_FUNC

	for (auto& knot : story_data->knots) {
		if (knot.second.is_function) {
			story_state.functions.insert({knot.first,
			[knot, this](TokenStack& stack, VariableMap& variables, const VariableMap& constants) -> ExpressionParser::Token* {
				for (auto parameter = knot.second.parameters.rbegin(); parameter != knot.second.parameters.rend(); ++parameter) {
					Token* param = stack.top();
					story_state.variables[*parameter] = param->get_variant_value(variables, constants).value();
					stack.pop();
				}

				std::optional<ExpressionParser::Variant> result = divert_to_function_knot(knot.first);
				if (result.has_value()) {
					return ExpressionParser::variant_to_token(*result);
				} else {
					return nullptr;
				}
			}});
		}
	}
}

std::optional<ExpressionParser::Variant> InkStory::divert_to_function_knot(const std::string& knot) {
	Knot* knot_ptr = &story_data->knots[knot];
	story_state.current_knots_stack.push_back({knot_ptr, 0});
	story_state.story_tracking.increment_visit_count(knot_ptr);

	InkStoryEvalResult eval_result;
	eval_result.result.reserve(512);
	while (!eval_result.reached_function_return && story_state.index_in_knot() < story_state.current_knot_size()) {
		InkObject* current_object = story_state.current_knot().knot->objects[story_state.index_in_knot()];
		current_object->execute(story_state, eval_result);

		++story_state.current_knot().index;
	}

	story_state.current_knots_stack.pop_back();
	return eval_result.return_value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool InkStory::can_continue() {
	InkStoryState::KnotStatus& current_knot_status = story_state.current_knot();
	return !story_state.should_end_story
	&& (current_knot_status.index < current_knot_status.knot->objects.size() || !story_state.current_knots_stack.empty())
	&& (!story_state.at_choice || story_state.selected_choice != SIZE_MAX);
}

std::string InkStory::continue_story() {
	story_state.current_tags.clear();

	InkStoryEvalResult eval_result;
	eval_result.result.reserve(512);
	eval_result.target_knot.reserve(32);
	while (!story_state.should_end_story && (!story_state.at_choice || story_state.selected_choice != SIZE_MAX) && story_state.index_in_knot() < story_state.current_knot_size()) {
		Knot* knot_before_object = story_state.current_knot().knot;
		bool changed_knot = false;
		InkObject* current_object = story_state.current_knot().knot->objects[story_state.index_in_knot()];

		if (eval_result.reached_newline && eval_result.has_any_contents(true) && current_object->stop_before_this()) {
			if (story_state.in_glue) {
				story_state.in_glue = false;
			} else {
				break;
			}
		}

		current_object->execute(story_state, eval_result);

		if (story_state.current_knot().knot != knot_before_object) {
			changed_knot = true;
		}

		story_state.just_diverted_to_non_knot = false;

		if (!eval_result.target_knot.empty()) {
			GetContentResult target = story_data->get_content(eval_result.target_knot, story_state.current_nonchoice_knot().knot, story_state.current_stitch);
			if (!target.found_any) {
				if (auto target_var = story_state.variables.find(eval_result.target_knot); target_var != story_state.variables.end()) {
					target = story_data->get_content(std::get<std::string>(target_var->second), story_state.current_nonchoice_knot().knot, story_state.current_stitch);
				}
			}

			if (target.found_any) {
				switch (target.result_type) {
					case WeaveContentType::Knot: {
						while (story_state.current_knots_stack.size() > 1 && story_state.current_knot().knot != story_state.current_nonchoice_knot().knot) {
							story_state.current_knots_stack.pop_back();
						}

						story_state.current_knots_stack.back() = {target.knot, 0};
						story_state.story_tracking.increment_visit_count(target.knot);

						for (std::size_t i = 0; i < target.knot->parameters.size(); ++i) {
							story_state.variables[target.knot->parameters[i]] = eval_result.arguments[i];
						}

						story_state.current_stitch = nullptr;
						changed_knot = true;
					} break;

					case WeaveContentType::Stitch: {
						story_state.current_stitch = target.stitch;

						while (story_state.current_knots_stack.size() > 1 && story_state.current_knot().knot != story_state.current_nonchoice_knot().knot) {
							story_state.current_knots_stack.pop_back();
						}

						story_state.current_knots_stack.back() = {target.knot, target.stitch->index};
						
						story_state.story_tracking.increment_visit_count(target.knot ? target.knot : story_state.current_nonchoice_knot().knot, story_state.current_stitch);
						if (story_state.current_nonchoice_knot().knot == story_state.current_knot().knot) {
							changed_knot = true;
						}

						for (std::size_t i = 0; i < target.stitch->parameters.size(); ++i) {
							story_state.variables[target.stitch->parameters[i]] = eval_result.arguments[i];
						}

						story_state.just_diverted_to_non_knot = true;
					} break;

					case WeaveContentType::GatherPoint:
					default: {
						while (story_state.current_knots_stack.size() > 1 && story_state.current_knot().knot != story_state.current_nonchoice_knot().knot) {
							story_state.current_knots_stack.pop_back();
						}

						story_state.current_knots_stack.back() = {target.knot, target.gather_point->index};
						if (target.stitch) {
							story_state.current_stitch = target.stitch;
						}

						if (story_state.current_nonchoice_knot().knot == story_state.current_knot().knot) {
							changed_knot = true;
						}

						story_state.just_diverted_to_non_knot = true;
					} break;
				}
			}

			eval_result.target_knot.clear();
			eval_result.arguments.clear();
		}

		std::vector<GatherPoint> no_current_stitch;
		auto joint_gather_view = std::vector{
			std::views::all(story_state.current_knot().knot->gather_points),
			std::views::all(story_state.current_stitch ? story_state.current_stitch->gather_points : no_current_stitch),
		} | std::views::join;

		for (GatherPoint& gather_point : joint_gather_view) {
			if (!changed_knot && !gather_point.in_choice && gather_point.index == story_state.index_in_knot() && !gather_point.name.empty()) {
				story_state.story_tracking.increment_visit_count(story_state.current_nonchoice_knot().knot, story_state.current_stitch, &gather_point);
				break;
			}
		}

		if (!changed_knot) {
			++story_state.current_knot().index;
		}/* else if (story_state.current_knot().knot == story_state.current_nonchoice_knot().knot) {
			for (const std::string& var : knot_before_object->local_variables) {
				story_state.variables.erase(var);
			}

			knot_before_object->local_variables.clear();
		}*/

		if (story_state.index_in_knot() >= story_state.current_knot_size() && story_state.current_knot().knot != story_state.current_nonchoice_knot().knot) {
			story_state.current_knots_stack.pop_back();

			bool found_gather = false;
			for (auto it = story_state.current_knots_stack.rbegin(); it != story_state.current_knots_stack.rend(); ++it) {
				for (GatherPoint& gather_point : it->knot->gather_points) {
					if (gather_point.level <= story_state.current_knots_stack.size() && gather_point.index > story_state.index_in_knot()) {
						story_state.current_knot().index = gather_point.index;
						story_state.story_tracking.increment_visit_count(story_state.current_nonchoice_knot().knot, story_state.current_stitch, &gather_point);
						found_gather = true;
						break;
					}
				}

				if (found_gather) {
					break;
				}
			}

			if (found_gather) {
				eval_result.reached_newline = true;
			} else {
				++story_state.current_knot().index;
			}
		}
	}

	return remove_duplicate_spaces(strip_string_edges(eval_result.result, true, true, true));
}

std::string InkStory::continue_story_maximally() {
	return std::string();
}

const std::vector<std::string>& InkStory::get_current_choices() const {
	return story_state.current_choices;
}

const std::vector<std::string>& InkStory::get_current_tags() const {
	return story_state.current_tags;
}

void InkStory::choose_choice_index(std::size_t index) {
	if (story_state.selected_choice == SIZE_MAX) {
		story_state.selected_choice = index;
		--story_state.current_knots_stack.back().index;
	}
}

std::optional<ExpressionParser::Variant> InkStory::get_variable(const std::string& name) const {
	if (auto variable = story_state.variables.find(name); variable != story_state.variables.end()) {
		return variable->second;
	}

	return {};
}

void InkStory::set_variable(const std::string& name, ExpressionParser::Variant&& value) {
	story_state.variables[name] = value;
}
