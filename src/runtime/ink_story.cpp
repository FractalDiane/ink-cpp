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
#include <iostream>

InkStory::InkStory(const std::string& inkb_file) {
	std::ifstream infile{inkb_file, std::ios::binary};
	std::vector<std::uint8_t> bytes;

	//infile.seekg(0, std::ios::end);
	//bytes.reserve(infile.tellg());
	//infile.seekg(0);
	
	std::uint8_t inbyte;
	while (infile >> inbyte) {
		bytes.push_back(inbyte);
	}

	infile.close();

	for (std::uint8_t byte : bytes) {
		std::cout << static_cast<int>(byte) << " ";
	}

	std::cout << std::endl;

	std::vector<Knot> knots;
	std::vector<std::string> knot_order;

	// HACK: make this less bad and more fail safe
	std::size_t index = 4;
	/*constexpr const char* expected_header = "INKB";
	std::string header;
	for (std::size_t i = 0; i < 4; ++i) {
		header += bytes[index++];
	}

	if (header != expected_header) {
		throw std::runtime_error("Not a valid inkb file (incorrect header)");
	}*/

	std::uint8_t version = bytes[index++];

	Serializer<std::uint16_t> dssize;
	std::uint16_t knots_count = dssize(bytes, index);

	for (std::size_t i = 0; i < knots_count; ++i) {
		Serializer<std::string> dsname;
		std::string this_knot_name = dsname(bytes, index);

		std::uint16_t this_knot_size = dssize(bytes, index);

		std::vector<InkObject*> this_knot_contents;
		this_knot_contents.reserve(this_knot_size);

		for (std::size_t i = 0; i < this_knot_size; ++i) {
			InkObject* this_object = nullptr;
			ObjectId this_id = static_cast<ObjectId>(bytes[index++]);
			switch (this_id) {
				case ObjectId::Text: {
					this_object = (new InkObjectText())->populate_from_bytes(bytes, index);
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

	//Serializer<std::vector<std::string>> dsorder;
	//std::vector<std::string> knot_order = dsorder(bytes, index);

	story_data = new InkStoryData(knots);
}

void InkStory::print_info() const {
	story_data->print_info();
}
