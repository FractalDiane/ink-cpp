#include "runtime/ink_story_data.h"

#include <format>

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
	result.reserve(2048);

	Serializer<std::uint16_t> ssize;
	std::vector<std::uint8_t> size_bytes = ssize(static_cast<std::uint16_t>(knots.size()));
	result.insert(result.end(), size_bytes.begin(), size_bytes.end());

	for (const auto& entry : knots) {
		const Knot& knot = entry.second;
		Serializer<Knot> s;
		std::vector<std::uint8_t> bytes = s(knot);
		result.insert(result.end(), bytes.begin(), bytes.end());
	}

	/*Serializer<std::vector<std::string>> sorder;
	std::vector<std::uint8_t> bytes = sorder(knot_order);
	result.insert(result.end(), bytes.begin(), bytes.end());*/

	return result;
}

void InkStoryData::print_info() const {
	std::cout << "Story Knots" << std::endl;
	for (const auto& knot : knots) {
		std::cout << "=== " << knot.second.name << std::endl;
		for (InkObject* object : knot.second.objects) {
			std::cout << object->to_string() << std::endl;
		}
	}

	std::cout << "Knot Order" << std::endl;
	for (const std::string& knot : knot_order) {
		std::cout << knot << std::endl;
	}
}

InkWeaveContent* InkStoryData::get_content(const std::string& path, Knot* current_knot, Stitch* current_stitch) {
	std::string first;
	first.reserve(10);
	std::string second;
	second.reserve(10);
	std::string third;
	third.reserve(10);

	std::size_t dots = 0;
	for (char chr : path) {
		std::string& str = dots == 0 ? first : dots == 1 ? second : third;
		if (chr == '.') {
			++dots;
		} else {
			str += chr;
		}
	}

	switch (dots) {
		case 0: {
			if (auto knot = knots.find(first); knot != knots.end()) {
				return &knot->second;
			}
		} break;

		case 1: {
			if (auto knot = knots.find(first); knot != knots.end()) {
				for (Stitch& stitch : knot->second.stitches) {
					if (stitch.name == second) {
						return &stitch;
					}
				}

				for (GatherPoint& gather_point : knot->second.gather_points) {
					if (gather_point.name == second) {
						return &gather_point;
					}
				}
			}
		} break;

		case 2: {
			if (auto knot = knots.find(first); knot != knots.end()) {
				for (Stitch& stitch : knot->second.stitches) {
					if (stitch.name == second) {
						for (GatherPoint& gather_point : stitch.gather_points) {
							if (gather_point.name == third) {
								return &gather_point;
							}
						}
					}
				}
			}
		} break;

		default: {
			throw;
		}
	}

	return nullptr;
}
