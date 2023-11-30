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

#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <iostream>

InkStory::InkStory(const std::string& inkb_file) {
	std::ifstream infile{inkb_file, std::ios::binary};

	std::size_t infile_size = std::filesystem::file_size(inkb_file);
	std::vector<std::uint8_t> bytes(infile_size);

	infile.read(reinterpret_cast<char*>(bytes.data()), infile_size);
	infile.close();

	// HACK: make this less bad and more fail safe
	std::size_t index = 0;
	constexpr const char* expected_header = "INKB";
	std::string header;
	for (std::size_t i = 0; i < 4; ++i) {
		header += bytes[index++];
	}

	if (header != expected_header) {
		throw std::runtime_error("Not a valid inkb file (incorrect header)");
	}

	std::uint8_t version = bytes[index++];

	Serializer<std::uint16_t> dssize;
	std::uint16_t knots_count = dssize(bytes, index);

	std::vector<Knot> knots;
	for (std::size_t i = 0; i < knots_count; ++i) {
		Serializer<std::string> dsname;
		std::string this_knot_name = dsname(bytes, index);

		std::uint16_t this_knot_size = dssize(bytes, index);

		std::vector<InkObject*> this_knot_contents;
		this_knot_contents.reserve(this_knot_size);

		for (std::size_t i = 0; i < this_knot_size; ++i) {
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
	story_state.current_knot = &(story_data->knots[story_data->knot_order[0]]);
}

std::string InkStory::continue_story() {
	story_state.current_tags.clear();

	InkStoryEvalResult eval_result = {.should_continue = true};
	eval_result.result.reserve(512);
	while (eval_result.should_continue && story_state.index_in_knot < story_state.current_knot->objects.size()) {
		InkObject* current_object = story_state.current_knot->objects[story_state.index_in_knot];
		current_object->execute(story_state, eval_result);

		++story_state.index_in_knot;
	}

	return eval_result.result;
}

const std::vector<std::string>& InkStory::get_current_choices() const {
	return story_state.current_choices;
}

const std::vector<std::string>& InkStory::get_current_tags() const {
	return story_state.current_tags;
}
void InkStory::choose_choice_index(std::size_t index) {
	
}
