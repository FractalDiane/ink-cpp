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

	// TODO: read variable info from file
	story_data = new InkStoryData(knots, {});
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

	story_state.variable_info = std::move(story_data->variable_info);
	bind_ink_functions();

	story_state.current_knots_stack = {{&(story_data->knots[story_data->knot_order[0]]), 0}};
	story_state.variable_info.current_weave_uuid = story_state.current_knot().knot->uuid;
}

void InkStory::bind_ink_functions() {
	using namespace ExpressionParserV2;

	#define EXP_FUNC(name, argc, body) story_state.variable_info.builtin_functions[name] = {[this](const std::vector<ExpressionParserV2::Variant>& arguments) -> ExpressionParserV2::Variant body, (std::uint8_t)argc};

	EXP_FUNC("CHOICE_COUNT", 0, { return story_state.current_choices.size(); });
	EXP_FUNC("TURNS", 0, { return story_state.total_choices_taken; });

	EXP_FUNC("TURNS_SINCE", 1, {
		const Variant& knot = arguments[0];

		if (GetContentResult content = story_data->get_content(static_cast<std::string>(knot), story_state.current_nonchoice_knot().knot, story_state.current_stitch); content.found_any) {
			InkStoryTracking::SubKnotStats stats;
			if (story_state.story_tracking.get_content_stats(content.get_target(), stats)) {
				return stats.turns_since_visited;
			}
		}
		
		return -1;
	});

	EXP_FUNC("SEED_RANDOM", 1, {
		std::int64_t seed = arguments[0];

		story_state.rng.seed(static_cast<unsigned int>(seed));
		return Variant();
	});

	EXP_FUNC("RANDOM", 2, {
		std::int64_t from = arguments[0];
		std::int64_t to = arguments[1];

		std::int64_t result = randi_range(from, to, story_state.rng);
		return result;
	});

	EXP_FUNC("LIST_VALUE", 1, {
		const InkList& list = arguments[0];
		return list.value();
	});

	EXP_FUNC("LIST_COUNT", 1, {
		const InkList& list = arguments[0];
		return list.count();
	});

	EXP_FUNC("LIST_MIN", 1, {
		const InkList& list = arguments[0];
		return list.min();
	});

	EXP_FUNC("LIST_MAX", 1, {
		const InkList& list = arguments[0];
		return list.max();
	});

	EXP_FUNC("LIST_RANDOM", 1, {
		const InkList& list = arguments[0];
		std::int64_t index = randi_range(0, list.count() - 1, story_state.rng);
		return list.at(static_cast<std::size_t>(index));
	});

	EXP_FUNC("LIST_ALL", 1, {
		const InkList& list = arguments[0];
		return list.all_possible_items();
	});

	EXP_FUNC("LIST_INVERT", 1, {
		const InkList& list = arguments[0];
		return list.inverted();
	});

	EXP_FUNC("LIST_RANGE", 3, {
		const InkList& list = arguments[0];
		const Variant& minimum = arguments[1];
		const Variant& maximum = arguments[2];

		if (minimum.index() == Variant_Int && maximum.index() == Variant_Int) {
			return list.range(static_cast<std::int64_t>(minimum), static_cast<std::int64_t>(maximum));
		} else if (minimum.index() == Variant_String && maximum.index() == Variant_String) {
			return list.range(static_cast<std::string>(minimum), static_cast<std::string>(maximum));
		} else {
			return list;
		}
	});

	#undef EXP_FUNC
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool InkStory::can_continue() {
	return !story_state.should_end_story
	&& (!story_state.at_choice || story_state.selected_choice.has_value())
	&& (!story_state.next_stitch || story_state.current_knot().index != story_state.next_stitch->index)
	&& !story_state.current_knots_stack.empty()
	&& story_state.index_in_knot() < story_state.current_knot_size();
}

std::string InkStory::continue_story() {
	story_state.current_tags.clear();
	if (story_state.current_knots_stack.empty()) {
		return std::string();
	}

	story_state.current_knot().any_new_content = false;

	InkStoryEvalResult eval_result;
	eval_result.result.reserve(512);
	eval_result.target_knot.reserve(32);
	while (can_continue()) {
		Knot* knot_before_object = story_state.current_knot().knot;
		bool changed_knot = false;
		InkObject* current_object = story_state.current_knot().knot->objects[story_state.index_in_knot()];

		InkStoryState::KnotStatus& last_knot = story_state.previous_nonfunction_knot();
		bool last_knot_had_newline = last_knot.index > 0 && last_knot.knot->objects[last_knot.index - 1]->get_id() == ObjectId::LineBreak;

		// are we stopping here?
		if (eval_result.reached_newline
		&& eval_result.has_any_contents(true)
		&& (!story_state.current_nonchoice_knot().knot->is_function || story_state.current_knot().any_new_content || last_knot_had_newline)
		&& current_object->stop_before_this(story_state)) {
		//&& (story_state.function_call_stack.empty() || !story_state.function_call_stack.back()->function_prep_interpolation)) {
			/*if (const InkStoryState::KnotStatus& knot = story_state.previous_nonfunction_knot(); knot.knot->objects[knot.index]->get_id() == ObjectId::Interpolation) {
				if (story_state.in_glue) {
					story_state.in_glue = false;
					eval_result.reached_newline = false;
				} else if (const InkStoryState::KnotStatus& knot_above = story_state.previous_nonfunction_knot(true); !knot_above.any_new_content || !knot_above.reached_newline) {

				} else {
					break;
				}
			} else {*/
				if (story_state.in_glue) {
					story_state.in_glue = false;
					eval_result.reached_newline = false;
				} else {
					break;
				}
			//}
		}

		//std::string previous_content = eval_result.result;
		current_object->execute(story_state, eval_result);
		//if (eval_result.reached_newline && !story_state.current_knot().any_new_content) {
		//	eval_result.reached_newline = false;
		//}
		
		// after collecting the options from a choice, a thread returns to its origin
		if (story_state.should_wrap_up_thread && story_state.current_thread_depth > 0) {
			story_state.current_knots_stack.pop_back();
			story_state.thread_arguments_stack.pop_back();
			++story_state.current_knot().index;
			--story_state.current_thread_depth;
			story_state.should_wrap_up_thread = false;
		}

		if (story_state.current_knot().knot != knot_before_object) {
			changed_knot = true;
		}

		story_state.just_diverted_to_non_knot = false;

		// diverts
		if (!eval_result.target_knot.empty()) {
			GetContentResult target = story_data->get_content(eval_result.target_knot, story_state.current_nonchoice_knot().knot, story_state.current_stitch);
			if (!target.found_any) {
				if (std::optional<ExpressionParserV2::Variant> target_var = story_state.variable_info.get_variable_value(eval_result.target_knot); target_var.has_value()) {
					target = story_data->get_content(*target_var, story_state.current_nonchoice_knot().knot, story_state.current_stitch);
				}
			}

			bool function = false;
			if (target.found_any) {
				switch (eval_result.divert_type) {
					case DivertType::Thread: {
						++story_state.current_thread_depth;

						// still need to put something on the stack if a thread goes to the same knot, so it can return
						if (target.knot == story_state.current_nonchoice_knot().knot) {
							switch (target.result_type) {
								case WeaveContentType::Stitch: {
									story_state.current_knots_stack.push_back({target.knot, target.stitch->index});
								} break;

								case WeaveContentType::GatherPoint: {
									story_state.current_knots_stack.push_back({target.knot, target.gather_point->index});
								} break;

								default: break;
							}
						}

						std::vector<std::pair<std::string, ExpressionParserV2::Variant>> arguments = story_state.arguments_stack.back();
						for (auto& arg : arguments) {
							if (arg.second.index() == ExpressionParserV2::Variant_String) {
								arg.second = story_state.current_knot().knot->divert_target_to_global(arg.second);
							}
						}

						story_state.thread_arguments_stack.push_back(arguments);
						[[fallthrough]];
					}
					case DivertType::ToKnot:
					case DivertType::ToTunnel:
					case DivertType::FromTunnel: {
						switch (target.result_type) {
							case WeaveContentType::Knot: {
								if (eval_result.divert_type == DivertType::ToTunnel || eval_result.divert_type == DivertType::Thread) {
									story_state.current_knots_stack.push_back({target.knot, 0});
								} else {
									while (story_state.current_knots_stack.size() > 1 && story_state.current_knot().knot != story_state.current_nonchoice_knot().knot) {
										story_state.current_knots_stack.pop_back();
									}

									story_state.current_knots_stack.back() = {target.knot, 0};
								}
								
								story_state.story_tracking.increment_visit_count(target.knot);
								if (!target.knot->stitches.empty() && target.knot->stitches[0].index == 0) {
									story_state.story_tracking.increment_visit_count(target.knot, &target.knot->stitches[0]);
								}

								story_state.variable_info.current_weave_uuid = target.knot->uuid;
								for (std::size_t i = 0; i < target.knot->parameters.size(); ++i) {
									story_state.variable_info.set_variable_value(target.knot->parameters[i].name, story_state.arguments_stack.back()[i].second);
									if (target.knot->parameters[i].by_ref && target.knot->parameters[i].name != story_state.arguments_stack.back()[i].first) {
										story_state.variable_info.redirects[target.knot->uuid].insert({
											target.knot->parameters[i].name,
											story_state.arguments_stack.back()[i].first,
										});
									}
								}

								// auto divert to the first stitch if there's no content outside of stitches
								if (!target.knot->stitches.empty() && target.knot->stitches[0].index == 0) {
									story_state.current_stitch = &target.knot->stitches[0];
								} else {
									story_state.current_stitch = nullptr;
								}
								
								story_state.setup_next_stitch();
								changed_knot = true;
							} break;

							case WeaveContentType::Stitch: {
								if (eval_result.divert_type == DivertType::ToTunnel) {
									story_state.current_knots_stack.push_back({target.knot, target.stitch->index});
								} else {
									while (story_state.current_knots_stack.size() > 1 && story_state.current_knot().knot != story_state.current_nonchoice_knot().knot) {
										story_state.current_knots_stack.pop_back();
									}

									story_state.current_knots_stack.back() = {target.knot, target.stitch->index};
								}
								
								story_state.story_tracking.increment_visit_count(target.knot ? target.knot : story_state.current_nonchoice_knot().knot, target.stitch);
								if (story_state.current_nonchoice_knot().knot == story_state.current_knot().knot) {
									changed_knot = true;
								}

								story_state.current_stitch = target.stitch;
								story_state.setup_next_stitch();

								story_state.variable_info.current_weave_uuid = target.stitch->uuid;
								for (std::size_t i = 0; i < target.stitch->parameters.size(); ++i) {
									story_state.variable_info.set_variable_value(target.stitch->parameters[i].name, story_state.arguments_stack.back()[i].second);
									if (target.stitch->parameters[i].by_ref && target.stitch->parameters[i].name != story_state.arguments_stack.back()[i].first) {
										story_state.variable_info.redirects[target.stitch->uuid].insert({
											target.stitch->parameters[i].name,
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

									story_state.current_knots_stack.back() = {target.knot, target.gather_point->index};
									if (target.gather_point->in_choice) {
										story_state.choice_divert_index = target.gather_point->choice_index;
									}
								}
								
								if (target.stitch) {
									story_state.current_stitch = target.stitch;
									story_state.setup_next_stitch();
								}

								if (story_state.current_nonchoice_knot().knot == story_state.current_knot().knot) {
									changed_knot = true;
								}

								story_state.just_diverted_to_non_knot = true;
							} break;
						}
					} break;

					case DivertType::Function: {
						bool from_interpolate = story_state.current_knot().knot->objects[story_state.current_knot().index]->get_id() == ObjectId::Interpolation;
						story_state.current_knot().returning_from_function = true;
						story_state.current_knots_stack.push_back({target.knot, 0});
						story_state.story_tracking.increment_visit_count(target.knot);

						story_state.variable_info.current_weave_uuid = target.knot->uuid;
						for (std::size_t i = 0; i < target.knot->parameters.size(); ++i) {
							story_state.variable_info.set_variable_value(target.knot->parameters[i].name, story_state.arguments_stack.back()[i].second);
							if (target.knot->parameters[i].by_ref && target.knot->parameters[i].name != story_state.arguments_stack.back()[i].first) {
								story_state.variable_info.redirects[target.knot->uuid].insert({
									target.knot->parameters[i].name,
									story_state.arguments_stack.back()[i].first,
								});
							}
						}

						story_state.function_call_stack.push_back(target.knot);
						changed_knot = true;
						function = true;

						if (eval_result.imminent_function_prep) {
							story_state.current_knot().knot->is_function_prep = true;
							story_state.current_knot().knot->function_prep_interpolation = from_interpolate;
						}

						eval_result.imminent_function_prep = false;
					} break;

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

		// if we're returning from a function, we need to get it off the stack
		bool function_has_return_value = false;
		if (eval_result.reached_function_return) {
			while (story_state.current_knot().knot != story_state.function_call_stack.back()) {
				story_state.current_knots_stack.pop_back();
			}
			
			story_state.current_knots_stack.pop_back();
			story_state.function_call_stack.pop_back();
			story_state.arguments_stack.pop_back();

			eval_result.reached_function_return = false;
			function_has_return_value = true;
			eval_result.reached_newline = false;
			changed_knot = true;

			story_state.variable_info.current_weave_uuid = story_state.current_stitch ? story_state.current_stitch->uuid : story_state.current_nonchoice_knot().knot->uuid;
		}

		// any gather points hit need to have their visit counts incremented
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
		} else {
			story_state.variable_info.current_weave_uuid = story_state.current_stitch ? story_state.current_stitch->uuid : story_state.current_nonchoice_knot().knot->uuid;
		}

		// if we've run out of content in this knot, the story continues to the next gather point
		while (!story_state.current_knots_stack.empty() && (!story_state.at_choice || story_state.current_knot().knot->is_function_prep) && story_state.index_in_knot() >= story_state.current_knot_size()) {
			if (story_state.should_wrap_up_thread) {
				break;
			} else if (story_state.current_knot().knot->is_choice_result) {
				story_state.current_knots_stack.pop_back();

				if (!story_state.current_knots_stack.empty()) {
					GatherPoint* found_gather = nullptr;
					while (true) {
						InkStoryState::KnotStatus& this_knot = story_state.current_knots_stack.back();
						for (GatherPoint* gather_point : this_knot.knot->get_all_gather_points()) {
							if (gather_point->level <= story_state.current_knots_stack.size() && gather_point->index > this_knot.index) {
								story_state.current_knot().index = gather_point->index;
								story_state.story_tracking.increment_visit_count(story_state.current_nonchoice_knot().knot, story_state.current_stitch, gather_point);
								found_gather = gather_point;
								break;
							}
						}

						if (found_gather || story_state.current_knots_stack.size() <= 1) {
							break;
						}

						story_state.current_knots_stack.pop_back();
					}

					story_state.variable_info.current_weave_uuid = story_state.current_stitch ? story_state.current_stitch->uuid : story_state.current_nonchoice_knot().knot->uuid;

					if (found_gather) {
						eval_result.reached_newline = story_state.current_knot().reached_newline = true;
					} else {
						++story_state.current_knot().index;
					}
				}
			} else if (story_state.current_knot().knot->is_function) {
				while (story_state.current_knot().knot != story_state.function_call_stack.back()) {
					story_state.current_knots_stack.pop_back();
				}
				
				story_state.current_knots_stack.pop_back();
				story_state.function_call_stack.pop_back();
				story_state.arguments_stack.pop_back();
				eval_result.reached_newline = false;
				if (!function_has_return_value) {
					eval_result.return_value = std::nullopt;
				}

				story_state.variable_info.current_weave_uuid = story_state.current_stitch ? story_state.current_stitch->uuid : story_state.current_nonchoice_knot().knot->uuid;
			// ending a thread needs to keep the last knot on the stack to apply the choices to
			} else if (story_state.current_thread_entries.empty() || story_state.current_knots_stack.size() > 1) {
				story_state.current_knots_stack.pop_back();
				if (!story_state.current_knots_stack.empty()) {
					++story_state.current_knot().index;
				}
			} else {
				break;
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

std::vector<std::string> InkStory::get_current_choices() const {
	std::vector<std::string> result;
	result.reserve(story_state.current_choices.size());
	for (const InkStoryState::StoryChoice& choice : story_state.current_choices) {
		result.push_back(choice.text);
	}

	return result;
}

const std::vector<std::string>& InkStory::get_current_tags() const {
	return story_state.current_tags;
}

void InkStory::choose_choice_index(std::size_t index) {
	if (index < story_state.current_choices.size()) {
		story_state.selected_choice = index;
		if (story_state.current_choices[index].from_thread) {
			const InkStoryState::ThreadEntry& thread_entry = story_state.current_thread_entries[index];
			story_state.current_knots_stack.back() = {thread_entry.containing_knot, thread_entry.index_in_knot};
			story_state.setup_next_stitch();

			InkWeaveContent* thread_target = thread_entry.containing_stitch
											? static_cast<InkWeaveContent*>(thread_entry.containing_stitch)
											: static_cast<InkWeaveContent*>(thread_entry.containing_knot);

			story_state.variable_info.current_weave_uuid = thread_target->uuid;
			for (std::size_t i = 0; i < thread_target->parameters.size(); ++i) {
				story_state.variable_info.set_variable_value(thread_target->parameters[i].name, thread_entry.arguments[i].second);
				if (thread_target->parameters[i].by_ref && thread_target->parameters[i].name != thread_entry.arguments[i].first) {
					story_state.variable_info.redirects[thread_target->uuid].insert({
						thread_target->parameters[i].name,
						thread_entry.arguments[i].first,
					});
				}
			}

			story_state.should_wrap_up_thread = false;
		} else {
			--story_state.current_knots_stack.back().index;
		}
	}
}

std::optional<ExpressionParserV2::Variant> InkStory::get_variable(const std::string& name) const {
	return story_state.variable_info.get_variable_value(name);
}

void InkStory::set_variable(const std::string& name, ExpressionParserV2::Variant&& value) {
	story_state.variable_info.set_variable_value(name, value);
}

void InkStory::observe_variable(const std::string& variable_name, ExpressionParserV2::VariableObserverFunc callback) {
	story_state.variable_info.observe_variable(variable_name, callback);
}

void InkStory::unobserve_variable(const std::string& variable_name) {
	story_state.variable_info.unobserve_variable(variable_name);
}

void InkStory::unobserve_variable(ExpressionParserV2::VariableObserverFunc observer) {
	story_state.variable_info.unobserve_variable(observer);
}

void InkStory::unobserve_variable(const std::string& variable_name, ExpressionParserV2::VariableObserverFunc observer) {
	story_state.variable_info.unobserve_variable(variable_name, observer);
}
