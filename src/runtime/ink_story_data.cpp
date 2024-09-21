#include "runtime/ink_story_data.h"

#include "runtime/ink_story_state.h"

#include <format>

#include <iostream>
#include <stdexcept>

#ifndef INKB_VERSION
#define INKB_VERSION 0
#endif

InkStoryData::InkStoryData(const std::vector<Knot>& story_knots, ExpressionParserV2::StoryVariableInfo&& variable_info) : variable_info(variable_info) {
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
	ByteVec result = {'I', 'N', 'K', 'B', INKB_VERSION};
	result.reserve(2048);

	Serializer<std::uint16_t> ssize;
	std::vector<std::uint8_t> size_bytes = ssize(static_cast<std::uint16_t>(knots.size()));
	result.insert(result.end(), size_bytes.begin(), size_bytes.end());

	for (const auto& entry : knots) {
		const Knot& knot = entry.second;
		Serializer<Knot> s;
		ByteVec bytes = s(knot);
		result.insert(result.end(), bytes.begin(), bytes.end());
	}

	VectorSerializer<std::string> sorder;
	ByteVec knot_order_bytes = sorder(knot_order);
	result.insert(result.end(), knot_order_bytes.begin(), knot_order_bytes.end());

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

#include "objects/ink_object_choice.h"

GetContentResult find_gather_point_recursive(const std::string& path, std::size_t dots, Knot* topmost_knot, std::vector<KnotStatus>& knots_stack, Knot* new_knot, Stitch* enclosing_stitch, Stitch* current_story_stitch, bool use_stitch, bool update_stack, bool top) {
	KnotStatus* this_knot_status = nullptr;
	if (update_stack && !top) {
		knots_stack.push_back({new_knot, 0});
		this_knot_status = &knots_stack.back();
	} else {
		bool found_knot = false;
		for (KnotStatus& knot : knots_stack) {
			if (knot.knot == topmost_knot) {
				this_knot_status = &knot;
				found_knot = true;
				break;
			}
		}

		if (!found_knot) {
			knots_stack.clear();
			knots_stack.push_back({topmost_knot, 0});
			this_knot_status = &knots_stack.front();
		}
	}

	GetContentResult non_stitch_result;

	std::vector<GatherPoint>& gather_points = use_stitch ? current_story_stitch->gather_points : new_knot->gather_points;
	for (GatherPoint& gather_point : gather_points) {
		if (!gather_point.in_choice && !gather_point.name.empty() && gather_point.name == path) {
			GetContentResult result;
			result.knot = new_knot;
			if (use_stitch) {
				result.stitch = current_story_stitch;
			}

			result.gather_point = &gather_point;
			result.result_type = WeaveContentType::GatherPoint;
			result.found_any = true;

			if (dots > 0 || enclosing_stitch == current_story_stitch) {
				return result;
			} else if (!enclosing_stitch) {
				non_stitch_result = result;
			}
		}
	}
	
	std::size_t i = use_stitch ? current_story_stitch->index : 0;
	while (i < new_knot->objects.size()) {
		InkObject* object = new_knot->objects[i];
		if (object->get_id() == ObjectId::Choice) {
			InkObjectChoice* choice_object = static_cast<InkObjectChoice*>(object);
			std::vector<GatherPoint*> choice_labels = choice_object->get_choice_labels();
			for (GatherPoint* gather_point : choice_labels) {
				if (gather_point->name == path) {
					GetContentResult result;
					result.knot = new_knot;
					result.gather_point = gather_point;
					result.result_type = WeaveContentType::GatherPoint;
					result.is_choice_label = true;
					result.found_any = true;

					if (dots > 0 || enclosing_stitch == current_story_stitch) {
						return result;
					} else if (!enclosing_stitch) {
						non_stitch_result = result;
					}
				}
			}

			std::vector<Knot*> choice_results = choice_object->get_choice_result_knots();
			for (Knot* knot : choice_results) {
				GetContentResult result = find_gather_point_recursive(path, dots, topmost_knot, knots_stack, knot, enclosing_stitch, current_story_stitch, false, update_stack, false);
				if (result.found_any) {
					if (update_stack) {
						this_knot_status->index = i;
					}

					return result;
				}
			}
		}

		++i;

		for (auto stitch = new_knot->stitches.rbegin(); stitch != new_knot->stitches.rend(); ++stitch) {
			if (stitch->index <= i) {
				enclosing_stitch = &*stitch;
				break;
			}
		}
	}

	if (update_stack && !top) {
		knots_stack.pop_back();
	}

	return non_stitch_result;
}

GetContentResult find_gather_point(const std::string& path, std::size_t dots, Knot* topmost_knot, std::vector<KnotStatus>& knots_stack, Stitch* current_story_stitch, bool use_stitch, bool update_stack) {
	return find_gather_point_recursive(path, dots, topmost_knot, knots_stack,
	topmost_knot, !topmost_knot->stitches.empty() && topmost_knot->stitches[0].index == 0 ? &topmost_knot->stitches[0] : nullptr,
	current_story_stitch, use_stitch, update_stack, true);
}

GetContentResult InkStoryData::get_content(const std::string& path, Knot* topmost_knot, std::vector<KnotStatus>& knots_stack, Stitch* current_stitch, bool update_stack) {
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

	GetContentResult result;
	switch (dots) {
		case 0: {
			if (auto knot = knots.find(first); knot != knots.end()) {
				result.knot = &knot->second;
				result.result_type = WeaveContentType::Knot;
				result.found_any = true;
				return result;
			}
			
			for (Stitch& stitch : topmost_knot->stitches) {
				if (stitch.name == first) {
					result.knot = topmost_knot;
					result.stitch = &stitch;
					result.result_type = WeaveContentType::Stitch;
					result.found_any = true;
					return result;
				}
			}

			return find_gather_point(first, 0, topmost_knot, knots_stack, current_stitch, false, update_stack);
		} break;

		case 1: {
			if (auto knot = knots.find(first); knot != knots.end()) {
				for (Stitch& stitch : knot->second.stitches) {
					if (stitch.name == second) {
						result.knot = &knot->second;
						result.stitch = &stitch;
						result.result_type = WeaveContentType::Stitch;
						result.found_any = true;
						return result;
					}
				}

				return find_gather_point(second, 1, &knot->second, knots_stack, current_stitch, false, update_stack);
			} else {
				for (Stitch& stitch : topmost_knot->stitches) {
					if (stitch.name == first) {
						return find_gather_point(second, 1, topmost_knot, knots_stack, &stitch, true, update_stack);
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
								result.knot = &knot->second;
								result.stitch = &stitch;
								result.gather_point = &gather_point;
								result.result_type = WeaveContentType::GatherPoint;
								result.found_any = true;
								return result;
							}
						}

						return find_gather_point(third, 2, &knot->second, knots_stack, &stitch, true, update_stack);
					}
				}
			}
		} break;

		default: {
			throw std::runtime_error("Invalid content address");
		}
	}

	return result;
}
