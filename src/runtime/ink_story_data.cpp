#include "runtime/ink_story_data.h"

#include <iostream>

#ifndef INKB_VERSION
#define INKB_VERSION 255
#endif

InkStoryData::InkStoryData(const std::vector<Knot>& story_knots) {
	knots.reserve(story_knots.size());
	knot_order.reserve(story_knots.size());
	for (const Knot& knot : story_knots) {
		knots.insert({knot.name, knot});
		knot_order.push_back(knot.name);
	}
}

InkStoryData::~InkStoryData() {
	for (const auto& entry : knots) {
		for (InkObject* object : entry.second.objects) {
			delete object;
		}
	}
}

std::vector<std::uint8_t> InkStoryData::get_serialized_bytes() const {
	std::vector<std::uint8_t> result = {'I', 'N', 'K', 'B', INKB_VERSION};
	result.reserve(1000);

	for (const auto& entry : knots) {
		const Knot& knot = entry.second;
		Serializer<Knot> s;
		std::vector<std::uint8_t> bytes = s(knot);
		result.insert(result.end(), bytes.begin(), bytes.end());
	}

	Serializer<std::vector<std::string>> sorder;
	std::vector<std::uint8_t> bytes = sorder(knot_order);
	result.insert(result.end(), bytes.begin(), bytes.end());

	return result;
}

void InkStoryData::print_info() const {
	std::cout << "Story Knots" << std::endl;
	for (const auto& knot : knots) {
		std::cout << "=== " << knot.second.name << std::endl;
		for (InkObject* object : knot.second.objects) {
			std::cout << object->to_string() << std::endl;
			//delete object;
		}
	}

	std::cout << "Knot Order" << std::endl;
	for (const std::string& knot : knot_order) {
		std::cout << knot << std::endl;
	}
}
