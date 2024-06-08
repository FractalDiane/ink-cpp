#include "runtime/ink_story.h"

#include "objects/ink_object.h"
#include "objects/ink_object_choice.h"
#include "objects/ink_object_divert.h"

#include "ink_utils.h"

#include <fstream>
#include <filesystem>
#include <algorithm>
#include <format>
#include <ranges>
#include <cmath>

#include <stdexcept>
#include <iostream>

#ifndef INKB_VERSION
#define INKB_VERSION 0
#endif

InkStory::InkStory(const std::string& inkb_file) {
	std::ifstream infile{inkb_file, std::ios::binary};

	std::size_t infile_size = static_cast<std::size_t>(std::filesystem::file_size(inkb_file));
	ByteVec bytes(infile_size);

	infile.read(reinterpret_cast<char*>(bytes.data()), infile_size);
	infile.close();

	constexpr const char* expected_header = "INKB";
	for (std::size_t i = 0; i < 4; ++i) {
		if (static_cast<signed char>(bytes[i]) != expected_header[i]) {
			throw std::runtime_error("Not a valid inkb file (incorrect header)");
		}
	}

	std::size_t index = 4;

	std::uint8_t version = bytes[index++];
	if (version != INKB_VERSION) {
		throw std::runtime_error(std::format("The version of this .inkb file ({}) does not match the version of your ink-cpp runtime ({}); please recompile your ink file", version, INKB_VERSION));
	}

	VectorDeserializer<Knot> dsknots;
	std::vector<Knot> knots = dsknots(bytes, index);

	story_data = new InkStoryData(knots);
	VectorDeserializer<std::string> dsorder;
	story_data->knot_order = dsorder(bytes, index);
	
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

	#define EXP_FUNC(name, body) story_state.functions.insert({name, [this](TokenStack& stack, VariableMap& variables, const VariableMap& constants, RedirectMap& variable_redirects) body});

	EXP_FUNC("CHOICE_COUNT", { return new TokenNumberInt(story_state.current_choices.size()); });
	EXP_FUNC("TURNS", { return new TokenNumberInt(story_state.total_choices_taken); });

	EXP_FUNC("TURNS_SINCE", {
		std::string knot = as_string(stack.top()->get_variant_value(variables, constants, variable_redirects).value());
		
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
		std::int64_t seed = as_int(stack.top()->get_variant_value(variables, constants, variable_redirects).value());
		stack.pop();

		story_state.rng.seed(static_cast<unsigned int>(seed));
		return nullptr;
	});

	EXP_FUNC("RANDOM", {
		std::int64_t to = as_int(stack.top()->get_variant_value(variables, constants, variable_redirects).value());
		stack.pop();
		std::int64_t from = as_int(stack.top()->get_variant_value(variables, constants, variable_redirects).value());
		stack.pop();

		std::int64_t result = randi_range(from, to, story_state.rng);
		return new TokenNumberInt(result);
	});

	#undef EXP_FUNC
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool InkStory::can_continue() {
	InkStoryState::KnotStatus& current_knot_status = story_state.current_knot();
	return !story_state.should_end_story
	&& (current_knot_status.index < current_knot_status.knot->objects.size() || story_state.current_knots_stack.size() > 1)
	&& (!story_state.at_choice || story_state.selected_choice.has_value());
}

std::string InkStory::continue_story() {
	story_state.current_tags.clear();
	story_state.current_knot().any_new_content = false;

	InkStoryEvalResult eval_result;
	eval_result.result.reserve(512);
	eval_result.target_knot.reserve(32);
	while (!story_state.should_end_story && (!story_state.at_choice || story_state.selected_choice.has_value()) && story_state.index_in_knot() < story_state.current_knot_size()) {
		Knot* knot_before_object = story_state.current_knot().knot;
		bool changed_knot = false;
		InkObject* current_object = story_state.current_knot().knot->objects[story_state.index_in_knot()];

		InkStoryState::KnotStatus& last_knot = story_state.previous_nonfunction_knot();
		bool last_knot_had_newline = last_knot.index > 0 && last_knot.knot->objects[last_knot.index - 1]->get_id() == ObjectId::LineBreak;
		if (eval_result.reached_newline
		&& eval_result.has_any_contents(true)
		&& (!story_state.current_nonchoice_knot().knot->is_function || story_state.current_nonchoice_knot().any_new_content || last_knot_had_newline)
		&& current_object->stop_before_this(story_state)) {
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

			bool function = false;
			if (target.found_any) {
				switch (eval_result.divert_type) {
					case DivertType::Thread: {
						story_state.in_thread = true;
						[[fallthrough]];
					}
					case DivertType::ToKnot:
					case DivertType::ToTunnel:
					case DivertType::FromTunnel: {
						switch (target.result_type) {
							case WeaveContentType::Knot: {
								if (eval_result.divert_type == DivertType::ToTunnel) {
									story_state.current_knots_stack.push_back({target.knot, 0});
								} else {
									while (story_state.current_knots_stack.size() > 1 && story_state.current_knot().knot != story_state.current_nonchoice_knot().knot) {
										story_state.current_knots_stack.pop_back();
									}

									story_state.current_knots_stack.back() = {target.knot, 0};
								}
								
								story_state.story_tracking.increment_visit_count(target.knot);

								for (std::size_t i = 0; i < target.knot->parameters.size(); ++i) {
									story_state.variables[target.knot->parameters[i].name] = story_state.arguments_stack.back()[i].second;
									if (target.knot->parameters[i].by_ref && target.knot->parameters[i].name != story_state.arguments_stack.back()[i].first) {
										story_state.variable_redirects[target.knot->uuid].insert({
											target.knot->parameters[i].name,
											story_state.arguments_stack.back()[i].first,
										});
									}
								}

								story_state.current_stitch = nullptr;
								changed_knot = true;
							} break;

							case WeaveContentType::Stitch: {
								story_state.current_stitch = target.stitch;

								if (eval_result.divert_type == DivertType::ToTunnel) {
									story_state.current_knots_stack.push_back({target.knot, target.stitch->index});
								} else {
									while (story_state.current_knots_stack.size() > 1 && story_state.current_knot().knot != story_state.current_nonchoice_knot().knot) {
										story_state.current_knots_stack.pop_back();
									}

									story_state.current_knots_stack.back() = {target.knot, target.stitch->index};
								}
								
								story_state.story_tracking.increment_visit_count(target.knot ? target.knot : story_state.current_nonchoice_knot().knot, story_state.current_stitch);
								if (story_state.current_nonchoice_knot().knot == story_state.current_knot().knot) {
									changed_knot = true;
								}

								for (std::size_t i = 0; i < target.stitch->parameters.size(); ++i) {
									story_state.variables[target.stitch->parameters[i].name] = story_state.arguments_stack.back()[i].second;
									if (target.knot->parameters[i].by_ref && target.knot->parameters[i].name != story_state.arguments_stack.back()[i].first) {
										story_state.variable_redirects[target.knot->uuid].insert({
											target.knot->parameters[i].name,
											story_state.arguments_stack.back()[i].first,
										});
									}
								}

								story_state.just_diverted_to_non_knot = true;
							} break;

							case WeaveContentType::GatherPoint:
							default: {
								if (eval_result.divert_type == DivertType::ToTunnel) {
									story_state.current_knots_stack.push_back({target.knot, target.gather_point->index});
								} else {
									while (story_state.current_knots_stack.size() > 1 && story_state.current_knot().knot != story_state.current_nonchoice_knot().knot) {
										story_state.current_knots_stack.pop_back();
									}

									/*if (target.gather_point->in_choice) {
										//story_state.current_knots_stack.back() = {target.}
									} else {
										story_state.current_knots_stack.back() = {target.knot, target.gather_point->index};
									}*/

									story_state.current_knots_stack.back() = {target.knot, target.gather_point->index};
									if (target.gather_point->in_choice) {
										//choose_choice_index(target.gather_point->choice_index);
										story_state.choice_divert_index = target.gather_point->choice_index;
									}
								}
								
								if (target.stitch) {
									story_state.current_stitch = target.stitch;
								}

								if (story_state.current_nonchoice_knot().knot == story_state.current_knot().knot) {
									changed_knot = true;
								}

								story_state.just_diverted_to_non_knot = true;
							} break;
						}
					} break;

					case DivertType::Function: {
						story_state.current_knot().returning_from_function = true;
						story_state.current_knots_stack.push_back({target.knot, 0});
						story_state.story_tracking.increment_visit_count(target.knot);

						for (std::size_t i = 0; i < target.knot->parameters.size(); ++i) {
							story_state.variables[target.knot->parameters[i].name] = story_state.arguments_stack.back()[i].second;
							if (target.knot->parameters[i].by_ref && target.knot->parameters[i].name != story_state.arguments_stack.back()[i].first) {
								story_state.variable_redirects[target.knot->uuid].insert({
									target.knot->parameters[i].name,
									story_state.arguments_stack.back()[i].first,
								});
							}
						}

						story_state.function_call_stack.push_back(target.knot);
						changed_knot = true;
						function = true;

						eval_result.imminent_function_prep = false;
					} break;

					/*case DivertType::Thread: {
						
					} break;*/

					default: {
						throw;
					} break;
				}
			}

			eval_result.target_knot.clear();

			if (!function && changed_knot) {
				story_state.arguments_stack.pop_back();
			}
			
			eval_result.divert_type = DivertType::ToKnot;
		} else if (eval_result.divert_type == DivertType::FromTunnel) {
			while (story_state.current_knot().knot != story_state.current_nonchoice_knot().knot) {
				story_state.current_knots_stack.pop_back();
			}
			
			story_state.current_knots_stack.pop_back();
			++story_state.current_knot().index;
			changed_knot = true;
			eval_result.divert_type = DivertType::ToKnot;
		}

		if (eval_result.reached_function_return) {
			while (story_state.current_knot().knot != story_state.function_call_stack.back()) {
				story_state.current_knots_stack.pop_back();
			}
			
			story_state.current_knots_stack.pop_back();
			story_state.function_call_stack.pop_back();
			story_state.arguments_stack.pop_back();

			eval_result.reached_function_return = false;
			eval_result.reached_newline = false;
			changed_knot = true;
		}

		std::vector<GatherPoint> no_current_stitch;
		std::vector<GatherPoint>& other_gathers = story_state.current_stitch ? story_state.current_stitch->gather_points : no_current_stitch;
		auto joint_gather_view = std::vector{
			std::views::all(story_state.current_knot().knot->gather_points),
			std::views::all(other_gathers),
		} | std::views::join;

		for (GatherPoint& gather_point : joint_gather_view) {
			if (!changed_knot && !gather_point.in_choice && gather_point.index == story_state.index_in_knot() && !gather_point.name.empty()) {
				story_state.story_tracking.increment_visit_count(story_state.current_nonchoice_knot().knot, story_state.current_stitch, &gather_point);
				break;
			}
		}

		if (!changed_knot) {
			++story_state.current_knot().index;
		}

		if (story_state.index_in_knot() >= story_state.current_knot_size()) {
			if (story_state.current_knot().knot != story_state.current_nonchoice_knot().knot) {
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
			} else if (story_state.current_knot().knot->is_function) {
				while (story_state.current_knot().knot != story_state.function_call_stack.back()) {
					story_state.current_knots_stack.pop_back();
				}
				
				story_state.current_knots_stack.pop_back();
				story_state.function_call_stack.pop_back();
				story_state.arguments_stack.pop_back();
				eval_result.reached_newline = false;
			}
		}
	}

	return remove_duplicate_spaces(strip_string_edges(eval_result.result, true, true, true));
}

std::string InkStory::continue_story_maximally() {
	std::string result;
	result.reserve(1024);
	while (can_continue()) {
		result += continue_story();
		result.push_back('\n');
	}

	result.pop_back();
	return result;
}

const std::vector<std::string>& InkStory::get_current_choices() const {
	return story_state.current_choices;
}

const std::vector<std::string>& InkStory::get_current_tags() const {
	return story_state.current_tags;
}

void InkStory::choose_choice_index(std::size_t index) {
	if (!story_state.selected_choice.has_value()) {
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

void InkStory::observe_variable(const std::string& variable_name, ExpressionParser::VariableObserverFunc callback) {
	story_state.variables.observe_variable(variable_name, callback);
}

void InkStory::unobserve_variable(const std::string& variable_name) {
	story_state.variables.unobserve_variable(variable_name);
}

void InkStory::unobserve_variable(ExpressionParser::VariableObserverFunc observer) {
	story_state.variables.unobserve_variable(observer);
}

void InkStory::unobserve_variable(const std::string& variable_name, ExpressionParser::VariableObserverFunc observer) {
	story_state.variables.unobserve_variable(variable_name, observer);
}
