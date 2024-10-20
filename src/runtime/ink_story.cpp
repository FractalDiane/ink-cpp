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
	story_state.setup_next_stitch();
}

void InkStory::bind_ink_functions() {
	using namespace ExpressionParserV2;

	#define EXP_FUNC(name, argc, body) story_state.variable_info.builtin_functions[name] = {[this](const std::vector<ExpressionParserV2::Variant>& arguments) -> ExpressionParserV2::Variant body, (std::uint8_t)argc};

	EXP_FUNC("CHOICE_COUNT", 0, { return story_state.current_choices.size() + story_state.current_thread_entries.size(); });
	EXP_FUNC("TURNS", 0, { return story_state.total_choices_taken; });

	EXP_FUNC("TURNS_SINCE", 1, {
		const Variant& knot = arguments[0];

		if (GetContentResult content = story_data->get_content(static_cast<std::string>(knot), story_state.current_nonchoice_knot().knot, story_state.current_knots_stack, story_state.current_stitch(), false); content.found_any) {
			InkStoryTracking::SubKnotStats stats;
			if (story_state.story_tracking.get_content_stats(content.get_target(), stats)) {
				return stats.turns_since_visited;
			}
		}
		
		return -1;
	});

	EXP_FUNC("READ_COUNT", 1, {
		const Variant& knot = arguments[0];

		if (GetContentResult content = story_data->get_content(static_cast<std::string>(knot), story_state.current_nonchoice_knot().knot, story_state.current_knots_stack, story_state.current_stitch(), false); content.found_any) {
			InkStoryTracking::SubKnotStats stats;
			if (story_state.story_tracking.get_content_stats(content.get_target(), stats)) {
				return stats.times_visited;
			}
		}

		return 0;
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
		} else if (minimum.index() == Variant_List && maximum.index() == Variant_List) {
			return list.range(static_cast<InkList>(minimum), static_cast<InkList>(maximum));
		} else {
			return list;
		}
	});

	#undef EXP_FUNC
}

void InkStory::try_remove_upper_knots(const GetContentResult& target) {
	for (int i = story_state.current_knots_stack.size() - 1; i >= 0; --i) {
		const Knot* this_knot = story_state.current_knots_stack[i].knot;
		if (this_knot == target.knot) {
			while (story_state.current_knots_stack.back().knot != this_knot) {
				story_state.current_knots_stack.pop_back();
			}

			break;
		}
	}
}

void InkStory::apply_knot_args(const InkWeaveContent* target, InkStoryEvalResult& eval_result) {
	story_state.variable_info.current_weave_uuid = target->uuid;
	story_state.variable_info.function_arguments_stack.push_back({});
	story_state.variable_info.redirects_stack.push_back({});
	std::unordered_map<std::string, ExpressionParserV2::Variant>& args = story_state.variable_info.function_arguments_stack.back();
	std::unordered_map<std::string, std::string>& redirects = story_state.variable_info.redirects_stack.back();
	for (std::size_t i = 0; i < target->parameters.size(); ++i) {
		args[target->parameters[i].name] = eval_result.divert_args[i].second;
		if (target->parameters[i].by_ref) {
			const std::string& lhs = target->parameters[i].name;
			const std::string& rhs = eval_result.divert_args[i].first;
			if (lhs != rhs) {
				redirects[lhs] = rhs;
			}
		}
	}

	eval_result.divert_args.clear();
}

void InkStory::update_visit_count_variables(std::vector<ExpressionParserV2::ShuntedExpression*>&& expressions) {
	for (ExpressionParserV2::ShuntedExpression* expression : expressions) {
		for (ExpressionParserV2::Token& token : expression->tokens) {
			if (token.type == ExpressionParserV2::TokenType::Variable) {
				GetContentResult content = story_data->get_content(token.variable_name, story_state.current_nonchoice_knot().knot, story_state.current_knots_stack, story_state.current_stitch(), false);
				if (content.found_any) {
					InkStoryTracking::SubKnotStats target_count;
					story_state.story_tracking.get_content_stats(content.get_target(), target_count);
					story_state.variable_info.constants[token.variable_name] = target_count.times_visited;
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool InkStory::can_continue() {
	return !story_state.should_end_story
	&& (!story_state.at_choice || story_state.selected_choice.has_value())
	&& !story_state.current_knots_stack.empty()
	&& (!story_state.current_knot().next_stitch || story_state.current_knot().index != story_state.current_knot().next_stitch->index)
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
		bool advance_knot_index = true;
		InkObject* current_object = story_state.current_knot().knot->objects[story_state.index_in_knot()];

		KnotStatus& last_knot = story_state.previous_nonfunction_knot();
		bool last_knot_had_newline = last_knot.index > 0 && last_knot.knot->objects[last_knot.index - 1]->get_id() == ObjectId::LineBreak;

		// any gather points hit need to have their visit counts incremented
		if (!changed_knot && !story_state.current_knot().returning_from_function) {
			for (GatherPoint& gather_point : story_state.current_knot().knot->gather_points) {
				if (!gather_point.in_choice && !gather_point.name.empty() && gather_point.index == story_state.index_in_knot()) {
					story_state.story_tracking.increment_visit_count(story_state.current_knot().knot, story_state.current_stitch(), &gather_point);
					break;
				}
			}
		}
		
		// are we stopping here?
		if (eval_result.reached_newline
		&& eval_result.has_any_contents(true)
		&& (!story_state.current_nonchoice_knot().knot->is_function || story_state.current_knot().any_new_content || last_knot_had_newline)
		&& current_object->stop_before_this(story_state)) {
			if (story_state.in_glue) {
				story_state.in_glue = false;
				eval_result.reached_newline = false;
			} else {
				break;
			}
		}

		update_visit_count_variables(current_object->get_all_expressions());
		current_object->execute(story_state, eval_result);
		
		// after collecting the options from a choice, a thread returns to its origin
		if (story_state.should_wrap_up_thread && story_state.current_thread_depth() > 0) {
			while (story_state.current_knot().knot != story_state.threads_stack.back()) {
				story_state.current_knots_stack.pop_back();
			}
			
			story_state.current_knots_stack.pop_back();

			story_state.thread_arguments_stack.pop_back();
			++story_state.current_knot().index;
			//--story_state.current_thread_depth;
			story_state.threads_stack.pop_back();
			
			story_state.should_wrap_up_thread = false;
		}

		if (story_state.current_knot().knot != knot_before_object) {
			changed_knot = true;
			advance_knot_index = false;
		}

		story_state.just_diverted_to_non_knot = false;

		// diverts
		if (!eval_result.target_knot.empty()) {
			GetContentResult target = story_data->get_content(eval_result.target_knot, story_state.current_nonchoice_knot().knot, story_state.current_knots_stack, story_state.current_stitch(), true);
			if (!target.found_any) {
				if (std::optional<ExpressionParserV2::Variant> target_var = story_state.variable_info.get_variable_value(eval_result.target_knot); target_var.has_value()) {
					target = story_data->get_content(*target_var, story_state.current_nonchoice_knot().knot, story_state.current_knots_stack, story_state.current_stitch(), true);
				}
			}

			bool function = false;
			bool any_parameters = false;
			if (target.found_any) {
				if (target.is_choice_label) {
					story_state.choice_divert_index = target.gather_point->choice_index;
					story_state.current_knots_stack.back() = {target.knot, target.gather_point->index};
					advance_knot_index = false;
				} else {
					switch (eval_result.divert_type) {
						case DivertType::Thread: {
							//++story_state.current_thread_depth;
							story_state.threads_stack.push_back(target.knot);

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

							std::vector<std::pair<std::string, ExpressionParserV2::Variant>> thread_args;
							if (const auto& arguments = eval_result.divert_args; !arguments.empty()) {
								for (const auto& arg : arguments) {
									std::pair<std::string, ExpressionParserV2::Variant> thread_arg = arg;
									if (arg.second.index() == ExpressionParserV2::Variant_String) {
										thread_arg.second = story_state.current_knot().knot->divert_target_to_global(arg.second);
									}

									thread_args.push_back(thread_arg);
								}
							}

							story_state.thread_arguments_stack.push_back(thread_args);
							[[fallthrough]];
						}

						case DivertType::ToKnot:
						case DivertType::ToTunnel:
						case DivertType::FromTunnel: {
							switch (target.result_type) {
								case WeaveContentType::Knot: {
									if (eval_result.divert_type == DivertType::ToTunnel || eval_result.divert_type == DivertType::Thread) {
										if (eval_result.divert_type == DivertType::ToTunnel) {
											story_state.thread_tunnels_stack.push_back(story_state.current_knot());
											story_state.thread_tunnels_stack.push_back({target.knot, 0});
										}

										story_state.current_knots_stack.push_back({target.knot, 0});
									} else {
										if (eval_result.divert_type == DivertType::FromTunnel) {
											story_state.thread_tunnels_stack.pop_back();
										}

										try_remove_upper_knots(target);
										story_state.current_knots_stack.back() = {target.knot, 0};
									}
									
									story_state.story_tracking.increment_visit_count(target.knot);
									if (!target.knot->stitches.empty() && target.knot->stitches[0].index == 0) {
										story_state.story_tracking.increment_visit_count(target.knot, &target.knot->stitches[0]);
									}

									apply_knot_args(target.knot, eval_result);
									any_parameters = !target.knot->parameters.empty();

									// auto divert to the first stitch if there's no content outside of stitches
									if (!target.knot->stitches.empty() && target.knot->stitches[0].index == 0) {
										story_state.current_nonchoice_knot().current_stitch = &target.knot->stitches[0];
									} else {
										story_state.current_nonchoice_knot().current_stitch = nullptr;
									}
									
									story_state.setup_next_stitch();
									advance_knot_index = false;
									changed_knot = true;
								} break;

								case WeaveContentType::Stitch: {
									if (eval_result.divert_type == DivertType::ToTunnel) {
										story_state.thread_tunnels_stack.push_back(story_state.current_knot());
										story_state.thread_tunnels_stack.push_back({target.knot, target.stitch->index});
										story_state.current_knots_stack.push_back({target.knot, target.stitch->index});
									} else {
										if (eval_result.divert_type == DivertType::FromTunnel) {
											story_state.thread_tunnels_stack.pop_back();
										}

										try_remove_upper_knots(target);
										story_state.current_knots_stack.back() = {target.knot, target.stitch->index};
									}
									
									story_state.story_tracking.increment_visit_count(target.knot ? target.knot : story_state.current_nonchoice_knot().knot, target.stitch);
									if (story_state.current_nonchoice_knot().knot == story_state.current_knot().knot) {
										advance_knot_index = false;
									}

									story_state.current_nonchoice_knot().current_stitch = target.stitch;
									story_state.setup_next_stitch();

									apply_knot_args(target.stitch, eval_result);
									any_parameters = !target.stitch->parameters.empty();

									story_state.just_diverted_to_non_knot = true;
								} break;

								case WeaveContentType::GatherPoint:
								default: {
									Knot* previous_knot = story_state.current_knots_stack.back().knot;
									if (eval_result.divert_type == DivertType::ToTunnel) {
										story_state.thread_tunnels_stack.push_back(story_state.current_knot());
										story_state.thread_tunnels_stack.push_back({target.knot, target.gather_point->index});
										story_state.current_knots_stack.push_back({target.knot, target.gather_point->index});
									} else {
										if (eval_result.divert_type == DivertType::FromTunnel) {
											story_state.thread_tunnels_stack.pop_back();
										}
										
										try_remove_upper_knots(target);
										story_state.current_knots_stack.back() = {target.knot, target.gather_point->index};
										if (target.gather_point->in_choice) {
											story_state.choice_divert_index = target.gather_point->choice_index;
										}
									}
									
									if (target.stitch) {
										story_state.current_nonchoice_knot().current_stitch = target.stitch;
										story_state.setup_next_stitch();
									}

									if (story_state.current_nonchoice_knot().knot == story_state.current_knot().knot || story_state.current_knots_stack.back().knot == previous_knot) {
										advance_knot_index = false;
									}

									story_state.just_diverted_to_non_knot = true;
								} break;
							}
						} break;

						case DivertType::Function: {
							story_state.current_knot().returning_from_function = true;
							story_state.current_knots_stack.push_back({target.knot, 0});
							story_state.story_tracking.increment_visit_count(target.knot);

							apply_knot_args(target.knot, eval_result);

							story_state.function_call_stack.push_back(target.knot);
							changed_knot = true;
							advance_knot_index = false;
							function = true;

							story_state.current_knot().knot->function_prep_type = eval_result.imminent_function_prep;
							eval_result.imminent_function_prep = FunctionPrepType::None;
						} break;

						default: {
							throw;
						} break;
					}
				}
			}

			eval_result.target_knot.clear();

			if (!function && !any_parameters && changed_knot) {
				story_state.variable_info.function_arguments_stack.pop_back();
				story_state.variable_info.redirects_stack.pop_back();
			}
			
			eval_result.divert_type = DivertType::ToKnot;
		} else if (eval_result.divert_type == DivertType::FromTunnel) {
			while (story_state.current_knot().knot != story_state.current_nonchoice_knot().knot) {
				story_state.current_knots_stack.pop_back();
			}
			
			story_state.current_knots_stack.pop_back();
			++story_state.current_knot().index;
			changed_knot = true;
			advance_knot_index = false;
			eval_result.divert_type = DivertType::ToKnot;
		}

		// if we're returning from a function, we need to get it off the stack
		bool function_has_return_value = eval_result.return_value.has_value();
		if (eval_result.reached_function_return) {
			while (story_state.current_knot().knot != story_state.function_call_stack.back()) {
				story_state.current_knots_stack.pop_back();
			}
			
			story_state.current_knots_stack.pop_back();
			story_state.function_call_stack.pop_back();
			story_state.variable_info.function_arguments_stack.pop_back();
			story_state.variable_info.redirects_stack.pop_back();

			eval_result.reached_function_return = false;
			function_has_return_value = true;

			// THIS IS HORRIBLE
			const KnotStatus& new_back = story_state.current_knots_stack.back();
			ObjectId back_type = new_back.knot->objects[new_back.index]->get_id();
			if (back_type == ObjectId::Interpolation || back_type == ObjectId::Choice) {
				eval_result.reached_newline = false;
			}

			changed_knot = true;
			advance_knot_index = false;

			story_state.update_weave_uuid();
		}

		if (advance_knot_index) {
			++story_state.current_knot().index;
		}

		// if we've run out of content in this knot, the story continues to the next gather point
 		while (!story_state.current_knots_stack.empty() && (!story_state.at_choice || story_state.current_knot().knot->function_prep_type != FunctionPrepType::None) && story_state.index_in_knot() >= story_state.current_knot_size()) {
			if (story_state.should_wrap_up_thread) {
				break;
			} else if (story_state.current_knot().knot->is_choice_result) {
				story_state.current_knots_stack.pop_back();

				if (!story_state.current_knots_stack.empty()) {
					GatherPoint* found_gather = nullptr;
					while (true) {
						KnotStatus& this_knot = story_state.current_knots_stack.back();
						for (GatherPoint* gather_point : this_knot.knot->get_all_gather_points()) {
							if (!gather_point->in_choice && gather_point->level <= story_state.current_knots_stack.size() && gather_point->index > this_knot.index) {
								story_state.current_knot().index = gather_point->index;
								story_state.story_tracking.increment_visit_count(story_state.current_nonchoice_knot().knot, story_state.current_stitch(), gather_point);
								found_gather = gather_point;
								break;
							}
						}

						if (found_gather || story_state.current_knots_stack.size() <= 1) {
							break;
						}

						story_state.current_knots_stack.pop_back();
					}

					story_state.update_weave_uuid();

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
				story_state.variable_info.function_arguments_stack.pop_back();
				story_state.variable_info.redirects_stack.pop_back();

				if (story_state.current_knot().knot->objects[story_state.current_knot().index]->get_id() != ObjectId::Logic) {
					eval_result.reached_newline = false;
				}

				if (!function_has_return_value) {
					eval_result.return_value = std::nullopt;
				}

				story_state.update_weave_uuid();
			// ending a thread needs to keep the last knot on the stack to apply the choices to
			} else if (story_state.current_thread_entries.empty() || story_state.current_knots_stack.size() > 1) {
				story_state.current_knots_stack.pop_back();
				if (!story_state.current_knots_stack.empty()) {
					++story_state.current_knot().index;
				}
			} else {
				story_state.apply_thread_choices();
				break;
			}
		}
	}

	if (!story_state.thread_entries_applied) {
		story_state.apply_thread_choices();
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
	if (!story_state.choice_divert_index.has_value() && index < story_state.current_choices.size()) {
		story_state.selected_choice = index;
		if (story_state.current_choices[index].from_thread) {
			const InkStoryState::ThreadChoiceEntry& thread_entry = story_state.current_thread_entries[index];

			while (story_state.current_nonchoice_knot().knot != story_state.current_knot().knot) {
				story_state.current_knots_stack.pop_back();
			}
			

			for (const KnotStatus& knot : thread_entry.tunnels_stack) {
				if (knot.knot == thread_entry.containing_knot) {
					break;
				}

				story_state.current_knots_stack.push_back(knot);
			}

			story_state.current_knots_stack.push_back({thread_entry.containing_knot, thread_entry.index_in_knot});

			if (!thread_entry.containing_knot->name.empty()) {
				for (auto stitch = story_state.current_nonchoice_knot().knot->stitches.rbegin(); stitch != story_state.current_nonchoice_knot().knot->stitches.rend(); ++stitch) {
					if (stitch->index <= thread_entry.index_in_knot) {
						story_state.current_knots_stack.back().current_stitch = &*stitch;
						break;
					}
				}
			}
			
			story_state.setup_next_stitch();

			InkWeaveContent* thread_target = thread_entry.containing_stitch
											? static_cast<InkWeaveContent*>(thread_entry.containing_stitch)
											: static_cast<InkWeaveContent*>(thread_entry.containing_knot);

			story_state.variable_info.current_weave_uuid = thread_target->uuid;

			story_state.variable_info.redirects_stack.push_back({});
			std::unordered_map<std::string, std::string>& redirects = story_state.variable_info.redirects_stack.back();
			for (std::size_t i = 0; i < thread_target->parameters.size(); ++i) {
				story_state.variable_info.set_variable_value(thread_target->parameters[i].name, thread_entry.arguments[i].second, true);
				if (thread_target->parameters[i].by_ref) {
					const std::string& lhs = thread_target->parameters[i].name;
					const std::string& rhs = thread_entry.arguments[i].first;
					if (lhs != rhs) {
						redirects[lhs] = rhs;
					}
				}
			}

			story_state.should_wrap_up_thread = false;
			story_state.thread_entries_applied = false;
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
