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

#include "shunting-yard.h"

#include <fstream>
#include <filesystem>
#include <algorithm>
#include <format>

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
					this_object = (new InkObjectTag(""))->populate_from_bytes(bytes, index);
				} break;

				case ObjectId::Divert: {
					this_object = (new InkObjectDivert(""))->populate_from_bytes(bytes, index);
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
	
		knots.push_back({this_knot_name, this_knot_contents, this_knot_stitches});
	}

	story_data = new InkStoryData(knots);
	init_story();
}

void InkStory::print_info() const {
	story_data->print_info();
}

void InkStory::init_story() {
	for (const auto& knot : story_data->knots) {
		story_state.variables[knot.first] = 0;
		for (const Stitch& stitch : knot.second.stitches) {
			std::string stitch_name = std::format("{}.{}", knot.first, stitch.name);
			story_state.variables[stitch_name] = 0;
		}
	}

	bind_ink_functions();

	story_state.current_knots_stack = {{&(story_data->knots[story_data->knot_order[0]]), 0}};
}

void InkStory::bind_ink_functions() {
	#define CP_FUNC(name, body, ...) {\
		auto func_##name = [this](cparse::TokenMap scope) -> cparse::packToken body;\
		story_state.variables[#name] = cparse::CppFunction(func_##name, {__VA_ARGS__}, "");\
	}

	CP_FUNC(CHOICE_COUNT, { return story_state.current_choices.size(); });
	CP_FUNC(TURNS, { return story_state.total_choices_taken; });
	CP_FUNC(TURNS_SINCE, {
		if (auto entry = story_state.turns_since_knots.find(scope["__knot"].asString()); entry != story_state.turns_since_knots.end()) {
			return static_cast<std::int64_t>(entry->second);
		} else {
			return -1;
		}
	}, "__knot");

	CP_FUNC(SEED_RANDOM, { 
		story_state.rng.seed(static_cast<unsigned int>(scope["__seed"].asInt()));
		return cparse::packToken::None();
	}, "__seed");

	CP_FUNC(RANDOM, {
		return randi_range(scope["__from"].asInt(), scope["__to"].asInt(), story_state.rng);
	}, "__from", "__to");

	CP_FUNC(INT, { return scope["__what"].asInt(); }, "__what");
	CP_FUNC(FLOOR, { return std::floor(scope["__what"].asDouble()); }, "__what");
	CP_FUNC(FLOAT, { return scope["__what"].asDouble(); }, "__what");

	#undef CP_FUNC
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
	while (eval_result.should_continue && !story_state.should_end_story && story_state.index_in_knot() < story_state.current_knot_size()) {
		Knot* knot_before_object = story_state.current_knot().knot;
		bool changed_knot = false;
		InkObject* current_object = story_state.current_knot().knot->objects[story_state.index_in_knot()];
		current_object->execute(story_state, eval_result);

		if (story_state.current_knot().knot != knot_before_object) {
			changed_knot = true;
		}

		// Special case: line break is ignored if a divert after it leads to glue
		if (story_state.check_for_glue_divert) { // HACK: is there a better way to do this?
			if (eval_result.target_knot == "END" || eval_result.target_knot == "DONE") {
				story_state.should_end_story = true;
			} else if (auto knot = story_data->knots.find(eval_result.target_knot); knot != story_data->knots.end() && !knot->second.objects.empty()) {
				eval_result.should_continue = knot->second.objects[0]->get_id() == ObjectId::Glue;
			}

			eval_result.target_knot.clear();
			story_state.check_for_glue_divert = false;
		} else if (!eval_result.target_knot.empty()) {
			// [knot].[stitch] divert
			if (std::size_t dot_index = eval_result.target_knot.find("."); dot_index != eval_result.target_knot.npos) {
				std::string knot_name = eval_result.target_knot.substr(0, dot_index);
				if (auto target_knot = story_data->knots.find(knot_name); target_knot != story_data->knots.end()) {
					const std::vector<Stitch>& knot_stitches = target_knot->second.stitches;

					std::string stitch_name = eval_result.target_knot.substr(dot_index + 1);
					std::size_t stitch_index = SIZE_MAX;
					for (const Stitch& stitch : knot_stitches) { // HACK: make this better than linear time
						if (stitch.name == stitch_name) {
							stitch_index = stitch.index;
							break;
						}
					}

					if (stitch_index != SIZE_MAX) {
						story_state.current_knots_stack.back() = {&(target_knot->second), stitch_index};
						story_state.increment_visit_count(std::format("{}.{}", knot_name, stitch_name));
						changed_knot = true;
					} else {
						throw std::runtime_error("Stitch not found");
					}
				} else {
					throw std::runtime_error("Knot for stitch not found");
				}
			// [knot] divert
			} else if (auto target_knot = story_data->knots.find(eval_result.target_knot); target_knot != story_data->knots.end()) {
				while (story_state.current_knots_stack.size() > 1 && story_state.current_knot().knot != story_state.current_nonchoice_knot().knot) {
					story_state.current_knots_stack.pop_back();
				}

				story_state.current_knots_stack.back() = {&(target_knot->second), 0};
				story_state.increment_visit_count(target_knot->first);
				changed_knot = true;
			// [stitch] divert
			} else {
				const std::vector<Stitch>& current_stitches = story_state.current_nonchoice_knot().knot->stitches;
				std::size_t stitch_index = SIZE_MAX;
				for (const Stitch& stitch : current_stitches) { // HACK: make this better than linear time
					if (stitch.name == eval_result.target_knot) {
						stitch_index = stitch.index;
						break;
					}
				}

				if (stitch_index != SIZE_MAX) {
					story_state.current_nonchoice_knot().index = stitch_index;
					std::string full_name = std::format("{}.{}", story_state.current_nonchoice_knot().knot->name, eval_result.target_knot);
					story_state.increment_visit_count(full_name);
					if (story_state.current_nonchoice_knot().knot == story_state.current_knot().knot) {
						changed_knot = true;
					}
				} else {
					throw std::runtime_error("Stitch not found");
				}
			}

			eval_result.target_knot.clear();
		}

		if (!changed_knot) {
			++story_state.current_knot().index;
		}

		if (story_state.index_in_knot() >= story_state.current_knot_size() && story_state.current_knot().knot != story_state.current_nonchoice_knot().knot) {
			bool was_empty = story_state.current_knots_stack.back().knot->objects.empty();
			story_state.current_knots_stack.pop_back();
			if (was_empty || story_state.in_glue) {
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

void InkStory::set_variable(const std::string& name, const cparse::packToken& value) {
	story_state.variables[name] = value;
}
